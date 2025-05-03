#include "utils_explore_channels.hpp"
#include "io_helpers.hpp" // For safeReadQuadlet, interpretAsASCII
#include "utils_string.hpp" // For StringMatch, extractStringsFromMemory, validateChannelNumbers, discoverChannelNamesAddress
#include "scanner.hpp"		// For FireWireDevice, DiceDefines.hpp constants

#include <iostream>
#include <iomanip> // For std::setw, std::setfill, std::left, std::hex, std::dec
#include <map>
#include <string>
#include <algorithm> // For std::sort
#include <regex>		 // For std::regex, std::smatch
#include <set>			 // For std::set

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost, CFSwapInt32BigToHost

namespace FWA::SCANNER
{

	void exploreChannelNamesArea(IOFireWireDeviceInterface **deviceInterface,
															 io_service_t service,
															 FireWireDevice &device,
															 UInt32 generation)
	{
		std::cout << "\n=== DETAILED CHANNEL NAMES EXPLORATION ===\n"
							<< std::endl;

		// Dynamically discover the channel names address
		uint64_t channelNamesBaseAddr = discoverChannelNamesAddress(deviceInterface, service, generation);

		// Scan a much larger range to find all channel names
		// We'll scan 1024 bytes (256 quadlets) before and 4096 bytes (1024 quadlets) after
		const int PRE_RANGE = 256;	 // Quadlets before
		const int POST_RANGE = 1024; // Quadlets after
		const int STEP = 1;					 // Scan every quadlet

		std::cout << "Scanning " << PRE_RANGE * 4 << " bytes before and " << POST_RANGE * 4
							<< " bytes after 0x" << std::hex << channelNamesBaseAddr << std::dec
							<< " for channel names..." << std::endl;

		// First, collect all readable registers in the range
		std::map<uint64_t, uint32_t> channelRegisters;

		for (int offset = -PRE_RANGE; offset <= POST_RANGE; offset += STEP)
		{
			uint64_t addr = channelNamesBaseAddr + (offset * 4); // 4 bytes per quadlet
			UInt32 value = 0;

			IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, addr, value, generation);
			if (status == kIOReturnSuccess)
			{
				channelRegisters[addr] = value;
			}
		}

		// Now analyze the collected registers for strings
		if (channelRegisters.empty())
		{
			std::cout << "No registers could be read in the channel names area." << std::endl;
			return;
		}

		std::cout << "Found " << channelRegisters.size() << " readable registers in the channel names area." << std::endl;

		// Use the common string extraction utility
		std::vector<StringMatch> strings = extractStringsFromMemory(channelRegisters);

		// Report all strings found
		std::cout << "\n--- Channel Names and Related Strings ---" << std::endl;
		for (const auto &match : strings)
		{
			std::cout << (match.isByteLevel ? "Byte-level" : "Quadlet-level") << " string at 0x"
								<< std::hex << match.address << std::dec << ": \"" << match.text << "\"" << std::endl;
		}

		// Look for patterns in the channel names
		std::cout << "\n--- Channel Name Patterns ---" << std::endl;

		// Check if we have a pattern of "OUTPUT CH1", "OUTPUT CH2", etc. or "INPUT CH1", "INPUT CH2", etc.
		std::set<std::string> outputChannelNames; // Using set instead of vector to automatically deduplicate
		std::set<std::string> inputChannelNames;	// Using set instead of vector to automatically deduplicate
		std::set<int> outputChannelNumbers;
		std::set<int> inputChannelNumbers;

		// Regex patterns for channel names
		std::regex outputChannelPattern(R"(OUTPUT\s+CH(\d+))");
		std::regex inputChannelPattern(R"(INPUT\s+CH(\d+))");

		// Combine all string texts for pattern matching, but avoid duplicates
		std::string allText;
		std::set<std::string> uniqueStrings;

		for (const auto &match : strings)
		{
			// Only add each unique string once to avoid duplicates
			if (uniqueStrings.insert(match.text).second)
			{
				allText += match.text + " "; // Add space to separate strings
			}
		}

		// Now search for patterns in the combined text
		std::smatch match;
		std::string::const_iterator searchStart(allText.cbegin());

		// Find all output channels
		while (std::regex_search(searchStart, allText.cend(), match, outputChannelPattern))
		{
			outputChannelNames.insert(match[0]); // Using insert instead of push_back
			// Extract the channel number
			int channelNum = std::stoi(match[1]);
			outputChannelNumbers.insert(channelNum);
			searchStart = match.suffix().first;
		}

		// Find all input channels
		searchStart = allText.cbegin();
		while (std::regex_search(searchStart, allText.cend(), match, inputChannelPattern))
		{
			inputChannelNames.insert(match[0]); // Using insert instead of push_back
			// Extract the channel number
			int channelNum = std::stoi(match[1]);
			inputChannelNumbers.insert(channelNum);
			searchStart = match.suffix().first;
		}

		// Report output channels
		if (!outputChannelNames.empty())
		{
			std::cout << "Found " << outputChannelNames.size() << " output channel names matching pattern 'OUTPUT CH#':" << std::endl;
			for (const auto &name : outputChannelNames)
			{
				std::cout << "  " << name << std::endl;
			}
		}
		else
		{
			std::cout << "No output channel names matching pattern 'OUTPUT CH#' found." << std::endl;
		}

		// Report input channels
		if (!inputChannelNames.empty())
		{
			std::cout << "Found " << inputChannelNames.size() << " input channel names matching pattern 'INPUT CH#':" << std::endl;
			for (const auto &name : inputChannelNames)
			{
				std::cout << "  " << name << std::endl;
			}
		}
		else
		{
			std::cout << "No input channel names matching pattern 'INPUT CH#' found." << std::endl;
		}

		// Also look for stereo channel patterns
		std::set<std::string> stereoOutputChannels; // Using set instead of vector to automatically deduplicate
		std::set<std::string> stereoInputChannels;	// Using set instead of vector to automatically deduplicate
		std::set<int> stereoOutputChannelNumbers;
		std::set<int> stereoInputChannelNumbers;

		// Regex patterns for stereo channels
		std::regex stereoOutputPattern(R"(OUTPUT\s+ST\s+CH(\d+)([LR]))");
		std::regex stereoInputPattern(R"(INPUT\s+ST\s+CH(\d+)([LR]))");

		// Find all stereo output channels
		searchStart = allText.cbegin();
		while (std::regex_search(searchStart, allText.cend(), match, stereoOutputPattern))
		{
			stereoOutputChannels.insert(match[0]); // Using insert instead of push_back
			// Extract the channel number
			int channelNum = std::stoi(match[1]);
			stereoOutputChannelNumbers.insert(channelNum);
			searchStart = match.suffix().first;
		}

		// Find all stereo input channels
		searchStart = allText.cbegin();
		while (std::regex_search(searchStart, allText.cend(), match, stereoInputPattern))
		{
			stereoInputChannels.insert(match[0]); // Using insert instead of push_back
			// Extract the channel number
			int channelNum = std::stoi(match[1]);
			stereoInputChannelNumbers.insert(channelNum);
			searchStart = match.suffix().first;
		}

		// Report stereo output channels
		if (!stereoOutputChannels.empty())
		{
			std::cout << "Found " << stereoOutputChannels.size() << " stereo output channel names:" << std::endl;
			for (const auto &name : stereoOutputChannels)
			{
				std::cout << "  " << name << std::endl;
			}
		}

		// Report stereo input channels
		if (!stereoInputChannels.empty())
		{
			std::cout << "Found " << stereoInputChannels.size() << " stereo input channel names:" << std::endl;
			for (const auto &name : stereoInputChannels)
			{
				std::cout << "  " << name << std::endl;
			}
		}

		// Validate channel numbers
		std::cout << "\n--- Channel Number Validation ---" << std::endl;
		validateChannelNumbers(outputChannelNumbers, "Output");
		validateChannelNumbers(inputChannelNumbers, "Input");
		validateChannelNumbers(stereoOutputChannelNumbers, "Stereo Output");
		validateChannelNumbers(stereoInputChannelNumbers, "Stereo Input");

		// Add a summary section
		std::cout << "\n--- Channel Configuration Summary ---" << std::endl;

		// Count total channels
		int totalOutputChannels = outputChannelNames.size();
		int totalInputChannels = inputChannelNames.size();
		int totalStereoOutputChannels = stereoOutputChannels.size();
		int totalStereoInputChannels = stereoInputChannels.size();

		// Calculate total mono and stereo channels
		int totalMonoOutputs = totalOutputChannels;
		int totalMonoInputs = totalInputChannels;
		// For stereo channels, we count pairs (L/R) as one stereo channel
		// The number of stereo pairs is the number of unique channel numbers
		int totalStereoOutputs = stereoOutputChannelNumbers.size();
		int totalStereoInputs = stereoInputChannelNumbers.size();

		// Calculate total physical channels
		int totalPhysicalOutputs = totalMonoOutputs + (totalStereoOutputs * 2);
		int totalPhysicalInputs = totalMonoInputs + (totalStereoInputs * 2);

		std::cout << "Output Channels:" << std::endl;
		std::cout << "  Mono: " << totalMonoOutputs << std::endl;
		std::cout << "  Stereo Pairs: " << totalStereoOutputs << " (" << totalStereoOutputs * 2 << " channels)" << std::endl;
		std::cout << "  Total Physical Outputs: " << totalPhysicalOutputs << std::endl;

		std::cout << "Input Channels:" << std::endl;
		std::cout << "  Mono: " << totalMonoInputs << std::endl;
		std::cout << "  Stereo Pairs: " << totalStereoInputs << " (" << totalStereoInputs * 2 << " channels)" << std::endl;
		std::cout << "  Total Physical Inputs: " << totalPhysicalInputs << std::endl;

		std::cout << "Total I/O Channels: " << (totalPhysicalOutputs + totalPhysicalInputs) << std::endl;

		// Validate total channel count
		const int MAX_REASONABLE_TOTAL_CHANNELS = 128; // Adjust based on device capabilities
		if (totalPhysicalOutputs + totalPhysicalInputs > MAX_REASONABLE_TOTAL_CHANNELS)
		{
			std::cout << "Warning: Total channel count (" << (totalPhysicalOutputs + totalPhysicalInputs)
								<< ") exceeds reasonable limit of " << MAX_REASONABLE_TOTAL_CHANNELS << std::endl;
		}

		std::cout << "\n=== END CHANNEL NAMES EXPLORATION ===\n"
							<< std::endl;
	}

} // namespace FWA::SCANNER