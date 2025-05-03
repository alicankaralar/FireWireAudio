#include "utils_string.hpp"
#include "io_helpers.hpp" // For interpretAsASCII, safeReadQuadlet
#include "scanner.hpp"		// For FireWireDevice, DiceDefines.hpp constants

#include <iostream>
#include <iomanip> // For std::hex, std::dec
#include <map>
#include <string>
#include <algorithm> // For std::sort
#include <regex>		 // For std::regex, std::smatch
#include <set>			 // For std::set

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost, CFSwapInt32BigToHost

namespace FWA::SCANNER
{

	// --- String Extraction Utility Implementation ---

	std::vector<StringMatch> extractStringsFromMemory(const std::map<uint64_t, uint32_t> &registers)
	{
		std::vector<StringMatch> results;

		// Sort registers by address
		std::vector<std::pair<uint64_t, uint32_t>> sortedRegisters;
		for (const auto &regPair : registers)
		{
			sortedRegisters.push_back(regPair);
		}
		std::sort(sortedRegisters.begin(), sortedRegisters.end());

		// Extract quadlet-level strings
		std::string currentString;
		uint64_t stringStartAddr = 0;
		bool inString = false;

		for (const auto &regPair : sortedRegisters)
		{
			uint64_t addr = regPair.first;
			uint32_t rawValue = regPair.second;
			uint32_t hostValue = CFSwapInt32LittleToHost(rawValue);
			std::string ascii = interpretAsASCII(hostValue);

			// Also try byte-swapped interpretation for little-endian strings
			std::string swappedAscii = interpretAsASCII(CFSwapInt32BigToHost(rawValue));
			if (swappedAscii.length() > ascii.length() ||
					(swappedAscii.length() == ascii.length() && swappedAscii.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") > ascii.find_first_not_of("abcdefghijklmnopqrstQRSTUVWXYZ0123456789")))
			{
				// Use the swapped version if it's longer or has more alphanumeric characters
				ascii = swappedAscii;
			}

			if (!ascii.empty())
			{
				if (!inString)
				{
					inString = true;
					stringStartAddr = addr;
				}
				currentString += ascii;
			}
			else
			{
				if (inString)
				{
					// End of string
					if (currentString.length() >= 3) // Minimum meaningful length
					{
						StringMatch match;
						match.text = currentString;
						match.address = stringStartAddr;
						match.isByteLevel = false;
						results.push_back(match);
					}
					currentString.clear();
					inString = false;
				}
			}
		}

		// Check if we were in a string at the end
		if (inString && currentString.length() >= 3)
		{
			StringMatch match;
			match.text = currentString;
			match.address = stringStartAddr;
			match.isByteLevel = false;
			results.push_back(match);
		}

		// Extract byte-level strings
		std::string byteString;
		uint64_t byteStringStartAddr = 0;
		bool inByteString = false;

		for (const auto &regPair : sortedRegisters)
		{
			uint64_t addr = regPair.first;
			uint32_t rawValue = regPair.second;
			uint32_t hostValue = CFSwapInt32LittleToHost(rawValue);
			uint32_t swappedValue = CFSwapInt32BigToHost(rawValue);

			// Try both byte orders for better string detection
			std::string normalBytes;
			std::string swappedBytes;
			bool foundNormalBytes = false;
			bool foundSwappedBytes = false;

			// Check normal byte order
			for (int bytePos = 3; bytePos >= 0; bytePos--)
			{
				char c = static_cast<char>((hostValue >> (8 * bytePos)) & 0xFF);
				if (c >= 32 && c <= 126)
				{
					normalBytes += c;
					foundNormalBytes = true;
				}
				else if (c == 0 && normalBytes.length() > 0)
				{
					// Null terminator - end of string
					break;
				}
				else if (normalBytes.length() > 0)
				{
					// Non-printable, non-null character breaks the string
					break;
				}
			}

			// Check swapped byte order
			for (int bytePos = 3; bytePos >= 0; bytePos--)
			{
				char c = static_cast<char>((swappedValue >> (8 * bytePos)) & 0xFF);
				if (c >= 32 && c <= 126)
				{
					swappedBytes += c;
					foundSwappedBytes = true;
				}
				else if (c == 0 && swappedBytes.length() > 0)
				{
					// Null terminator - end of string
					break;
				}
				else if (swappedBytes.length() > 0)
				{
					// Non-printable, non-null character breaks the string
					break;
				}
			}

			// Use the better string (more printable characters)
			std::string betterBytes;
			if (foundSwappedBytes && (swappedBytes.length() > normalBytes.length()))
			{
				betterBytes = swappedBytes;
			}
			else if (foundNormalBytes)
			{
				betterBytes = normalBytes;
			}

			// Process the better string
			if (!betterBytes.empty())
			{
				if (!inByteString)
				{
					inByteString = true;
					byteStringStartAddr = addr;
				}
				byteString += betterBytes;
			}
			else if (inByteString)
			{
				// No ASCII found in this register, end any current string
				if (byteString.length() >= 3)
				{
					StringMatch match;
					match.text = byteString;
					match.address = byteStringStartAddr;
					match.isByteLevel = true;
					results.push_back(match);
				}
				byteString.clear();
				inByteString = false;
			}
		}

		// Check for any remaining byte string
		if (inByteString && byteString.length() >= 3)
		{
			StringMatch match;
			match.text = byteString;
			match.address = byteStringStartAddr;
			match.isByteLevel = true;
			results.push_back(match);
		}

		return results;
	}

	uint64_t discoverChannelNamesAddress(IOFireWireDeviceInterface **deviceInterface,
																			 io_service_t service,
																			 UInt32 generation)
	{
		std::cerr << "Debug [Utils]: Attempting to discover channel names address..." << std::endl;

		// Known potential addresses for channel names
		std::vector<uint64_t> potentialAddresses = {
				0xffffe00001a8, // Known address from previous scans
				0xffffe0000090, // Channel Configuration area
				0xffffe0000100, // Another potential area
				0xffffe0000200	// Another potential area
		};

		// Try each address and look for channel name patterns
		for (uint64_t addr : potentialAddresses)
		{
			std::cerr << "Debug [Utils]: Checking address 0x" << std::hex << addr << std::dec << " for channel names..." << std::endl;

			// Read a block of memory at this address
			const int BLOCK_SIZE = 256; // 256 quadlets = 1024 bytes
			std::map<uint64_t, uint32_t> memoryBlock;
			bool validBlock = true;

			for (int i = 0; i < BLOCK_SIZE; i++)
			{
				UInt32 value = 0;
				IOReturn status = safeReadQuadlet(deviceInterface, service, addr + (i * 4), value, generation);
				if (status != kIOReturnSuccess)
				{
					std::cerr << "Debug [Utils]: Failed to read quadlet at 0x" << std::hex << addr + (i * 4) << std::dec << std::endl;
					if (i == 0)
					{
						// If we can't read the first quadlet, this address is invalid
						validBlock = false;
						break;
					}
					// Otherwise, we've read some quadlets, so continue with what we have
					break;
				}
				memoryBlock[addr + (i * 4)] = value;
			}

			if (!validBlock || memoryBlock.empty())
			{
				std::cerr << "Debug [Utils]: Address 0x" << std::hex << addr << std::dec << " is not readable" << std::endl;
				continue;
			}

			// Extract strings from the memory block
			std::vector<StringMatch> strings = extractStringsFromMemory(memoryBlock);

			// Check if any of the strings match channel name patterns
			std::regex channelPattern(R"((?:OUTPUT|INPUT)(?:\s+ST)?\s+CH\d+)");
			bool foundChannelNames = false;

			for (const auto &match : strings)
			{
				std::smatch regexMatch;
				if (std::regex_search(match.text, regexMatch, channelPattern))
				{
					std::cerr << "Debug [Utils]: Found channel name pattern '" << regexMatch.str()
										<< "' at address 0x" << std::hex << match.address << std::dec << std::endl;
					foundChannelNames = true;
					break;
				}
			}

			if (foundChannelNames)
			{
				std::cerr << "Debug [Utils]: Discovered channel names at address 0x" << std::hex << addr << std::dec << std::endl;
				return addr;
			}
		}

		// If no address was found, return the known address as a fallback
		std::cerr << "Debug [Utils]: No channel names address discovered, using fallback address 0xffffe00001a8" << std::endl;
		return 0xffffe00001a8;
	}

	void validateChannelNumbers(const std::set<int> &channelNumbers, const std::string &channelType)
	{
		if (channelNumbers.empty())
		{
			std::cout << channelType << " channel numbers: None found" << std::endl;
			return;
		}

		// Check if the numbers are sequential
		int expectedNext = *channelNumbers.begin();
		bool sequential = true;
		std::vector<int> gaps;

		for (int num : channelNumbers)
		{
			if (num != expectedNext)
			{
				sequential = false;
				// Record the gap
				while (expectedNext < num)
				{
					gaps.push_back(expectedNext);
					expectedNext++;
				}
			}
			expectedNext = num + 1;
		}

		// Report findings
		std::cout << channelType << " channel numbers: ";
		if (sequential)
		{
			std::cout << "Sequential from " << *channelNumbers.begin() << " to " << *channelNumbers.rbegin()
								<< " (" << channelNumbers.size() << " channels)" << std::endl;
		}
		else
		{
			std::cout << "Non-sequential with gaps: ";
			for (int gap : gaps)
				std::cout << gap << " ";
			std::cout << std::endl;
			std::cout << "Found " << channelNumbers.size() << " channels, from "
								<< *channelNumbers.begin() << " to " << *channelNumbers.rbegin() << std::endl;
		}

		// Check if the numbers are within reasonable limits
		const int MAX_REASONABLE_CHANNELS = 64; // Adjust based on device capabilities
		if (*channelNumbers.rbegin() > MAX_REASONABLE_CHANNELS)
		{
			std::cout << "Warning: " << channelType << " channel numbers exceed reasonable limit of "
								<< MAX_REASONABLE_CHANNELS << std::endl;
		}
	}

} // namespace FWA::SCANNER