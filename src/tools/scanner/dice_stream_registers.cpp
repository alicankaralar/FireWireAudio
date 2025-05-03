#include "dice_stream_registers.hpp"
#include "io_helpers.hpp"		// For safeReadQuadlet, interpretAsASCII
#include "scanner.hpp"			// For FireWireDevice, DiceDefines.hpp constants
#include "dice_helpers.hpp" // For DICE_REGISTER constants

#include <iostream>
#include <iomanip> // For std::hex, std::setw, std::setfill
#include <map>
#include <string>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER
{

	// Helper to read TX stream parameters using progressive discovery
	void readDiceTxStreamRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
																 FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation,
																 uint32_t txStreamSizeQuadlets)
	{
		// Skip if invalid size
		if (txStreamSizeQuadlets == 0)
		{
			std::cerr << "Debug [DICE]: Skipping TX stream register read (invalid size=" << txStreamSizeQuadlets << ")." << std::endl;
			return;
		}

		// Store the original reported count for reference
		uint32_t reportedStreamCount = device.txStreamCount;

		// Set a reasonable upper bound for exploration
		// This is not a hardcoded limit, just a safety cap for exploration
		const uint32_t MAX_REASONABLE_STREAMS = 16;
		uint32_t maxStreamsToExplore = (reportedStreamCount == 0) ? MAX_REASONABLE_STREAMS : std::min(reportedStreamCount, MAX_REASONABLE_STREAMS);

		if (reportedStreamCount > MAX_REASONABLE_STREAMS)
		{
			std::cerr << "Warning [DICE]: Reported TX stream count (" << reportedStreamCount
								<< ") exceeds reasonable limit. Will explore up to "
								<< MAX_REASONABLE_STREAMS << " streams." << std::endl;
		}
		else if (reportedStreamCount == 0)
		{
			std::cerr << "Warning [DICE]: No TX streams reported. Will explore up to "
								<< MAX_REASONABLE_STREAMS << " streams to discover actual count." << std::endl;
		}

		std::cerr << "Info [DICE]: Starting progressive discovery of TX streams..." << std::endl;

		// Find TX Parameter Space Offset (needs to be read first)
		uint64_t txParamSpaceOffsetAddr = discoveredDiceBase + DICE_REGISTER_TX_PAR_SPACE_OFF;
		uint32_t txParamSpaceOffsetQuadlets = 0;
		if (device.diceRegisters.count(txParamSpaceOffsetAddr))
		{
			txParamSpaceOffsetQuadlets = CFSwapInt32LittleToHost(device.diceRegisters[txParamSpaceOffsetAddr]);
		}
		else
		{
			std::cerr << "Warning [DICE]: TX Parameter Space Offset not previously read. Cannot read TX stream details." << std::endl;
			return;
		}
		uint64_t txParamSpaceBase = discoveredDiceBase + (txParamSpaceOffsetQuadlets * 4);
		std::cerr << "Debug [DICE]: TX Parameter Space Base Address: 0x" << std::hex << txParamSpaceBase << std::dec << std::endl;

		// Define key registers to check for each stream
		// We'll use a smaller set of critical registers as "probe" registers
		std::vector<std::pair<uint64_t, std::string>> probeRegisters = {
				{DICE_REGISTER_TX_ISOC_BASE, "ISOC"},
				{DICE_REGISTER_TX_NB_AUDIO_BASE, "Audio Channels"}};

		// Define all registers to read for valid streams
		std::map<uint64_t, std::string> allTxStreamRegs = {
				{DICE_REGISTER_TX_ISOC_BASE, "ISOC"},
				{DICE_REGISTER_TX_NB_AUDIO_BASE, "Audio Channels"},
				{DICE_REGISTER_TX_MIDI_BASE, "MIDI"},
				{DICE_REGISTER_TX_SPEED_BASE, "Speed"},
				{DICE_REGISTER_TX_NAMES_BASE, "Names Base"},
				{DICE_REGISTER_TX_AC3_CAPABILITIES_BASE, "AC3 Capabilities"},
				{DICE_REGISTER_TX_AC3_ENABLE_BASE, "AC3 Enable"}};

		// Track valid streams and failures
		uint32_t validStreamCount = 0;
		uint32_t consecutiveFailures = 0;
		const uint32_t MAX_CONSECUTIVE_FAILURES = 3;

		// Progressive discovery loop
		for (uint32_t i = 0; i < maxStreamsToExplore; ++i)
		{
			uint64_t streamInstanceOffsetBytes = i * txStreamSizeQuadlets * 4;
			bool streamValid = false;

			// Reduced logging - only log at higher level
			if (logger_)
				logger_->debug("Probing TX stream [{}]...", i);

			// Try to read the probe registers for this stream
			for (const auto &regPair : probeRegisters)
			{
				uint64_t regRelativeOffset = regPair.first;
				const std::string &regName = regPair.second;
				uint64_t fullAddr = txParamSpaceBase + streamInstanceOffsetBytes + regRelativeOffset;
				UInt32 value = 0;

				IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, fullAddr, value, generation);
				if (status == kIOReturnSuccess)
				{
					// Successfully read this register
					device.diceRegisters[fullAddr] = value; // Store raw BE value
					UInt32 swappedValue = CFSwapInt32LittleToHost(value);
					std::string ascii = FWA::SCANNER::interpretAsASCII(swappedValue);

					// Reduce log verbosity - only log non-empty ASCII values or at debug level
					if (!ascii.empty())
					{
						std::cerr << "Info [DICE]: TX[" << i << "] " << regName << ": '" << ascii << "'" << std::endl;
					}
					// Detailed register info only at debug level
					if (logger_)
						logger_->debug("TX[{}] {} (0x{:x}): 0x{:x}", i, regName, fullAddr, swappedValue);

					streamValid = true; // Mark this stream as valid if at least one register reads successfully
				}
				else
				{
					std::cerr << "Warning [DICE]: Read failed for TX[" << i << "] " << regName
										<< " (0x" << std::hex << fullAddr << ") (status: " << status << ")" << std::dec << std::endl;

					// Don't increment consecutive failures here, only if the entire stream fails
				}
			}

			if (streamValid)
			{
				// This stream is valid, increment counter
				validStreamCount++;
				consecutiveFailures = 0; // Reset failure counter on success

				// Now read the rest of the registers for this stream
				for (const auto &regPair : allTxStreamRegs)
				{
					// Skip registers we already read during probing
					bool alreadyRead = false;
					for (const auto &probeReg : probeRegisters)
					{
						if (probeReg.first == regPair.first)
						{
							alreadyRead = true;
							break;
						}
					}
					if (alreadyRead)
						continue;

					uint64_t regRelativeOffset = regPair.first;
					const std::string &regName = regPair.second;
					uint64_t fullAddr = txParamSpaceBase + streamInstanceOffsetBytes + regRelativeOffset;
					UInt32 value = 0;

					IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, fullAddr, value, generation);
					if (status == kIOReturnSuccess)
					{
						device.diceRegisters[fullAddr] = value; // Store raw BE value
						UInt32 swappedValue = CFSwapInt32LittleToHost(value);
						std::string ascii = FWA::SCANNER::interpretAsASCII(swappedValue);

						// Reduce log verbosity - only log non-empty ASCII values or at debug level
						if (!ascii.empty())
						{
							std::cerr << "Info [DICE]: TX[" << i << "] " << regName << ": '" << ascii << "'" << std::endl;
						}
						// Detailed register info only at debug level
						if (logger_)
							logger_->debug("TX[{}] {} (0x{:x}): 0x{:x}", i, regName, fullAddr, swappedValue);
					}
					else
					{
						std::cerr << "Warning [DICE]: Read failed for TX[" << i << "] " << regName
											<< " (0x" << std::hex << fullAddr << ") (status: " << status << ")" << std::dec << std::endl;
						// Non-critical register, continue anyway
					}
				}
			}
			else
			{
				// This entire stream failed, increment consecutive failures
				consecutiveFailures++;

				if (consecutiveFailures >= MAX_CONSECUTIVE_FAILURES)
				{
					std::cerr << "Info [DICE]: Stopping TX stream discovery after " << consecutiveFailures
										<< " consecutive stream failures." << std::endl;
					break;
				}
			}
		}

		// Update the device's stream count based on our discovery
		if (validStreamCount > 0)
		{
			if (validStreamCount != reportedStreamCount)
			{
				std::cerr << "Info [DICE]: Adjusting TX stream count from reported " << reportedStreamCount
									<< " to discovered " << validStreamCount << " based on successful reads." << std::endl;
				device.txStreamCount = validStreamCount;
			}
			else
			{
				std::cerr << "Info [DICE]: Confirmed TX stream count of " << validStreamCount
									<< " matches reported count." << std::endl;
			}
		}
		else if (reportedStreamCount > 0)
		{
			std::cerr << "Warning [DICE]: No valid TX streams discovered. Keeping reported count of "
								<< reportedStreamCount << " for reference." << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE]: No valid TX streams discovered and none reported." << std::endl;
			device.txStreamCount = 0;
		}
	}

	// Helper to read RX stream parameters using progressive discovery
	void readDiceRxStreamRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
																 FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation,
																 uint32_t rxStreamSizeQuadlets)
	{
		// Skip if invalid size
		if (rxStreamSizeQuadlets == 0)
		{
			std::cerr << "Debug [DICE]: Skipping RX stream register read (invalid size=" << rxStreamSizeQuadlets << ")." << std::endl;
			return;
		}

		// Store the original reported count for reference
		uint32_t reportedStreamCount = device.rxStreamCount;

		// Set a reasonable upper bound for exploration
		// This is not a hardcoded limit, just a safety cap for exploration
		const uint32_t MAX_REASONABLE_STREAMS = 16;
		uint32_t maxStreamsToExplore = (reportedStreamCount == 0) ? MAX_REASONABLE_STREAMS : std::min(reportedStreamCount, MAX_REASONABLE_STREAMS);

		if (reportedStreamCount > MAX_REASONABLE_STREAMS)
		{
			std::cerr << "Warning [DICE]: Reported RX stream count (" << reportedStreamCount
								<< ") exceeds reasonable limit. Will explore up to "
								<< MAX_REASONABLE_STREAMS << " streams." << std::endl;
		}
		else if (reportedStreamCount == 0)
		{
			std::cerr << "Warning [DICE]: No RX streams reported. Will explore up to "
								<< MAX_REASONABLE_STREAMS << " streams to discover actual count." << std::endl;
		}

		std::cerr << "Info [DICE]: Starting progressive discovery of RX streams..." << std::endl;

		// Find RX Parameter Space Offset (needs to be read first)
		uint64_t rxParamSpaceOffsetAddr = discoveredDiceBase + DICE_REGISTER_RX_PAR_SPACE_OFF;
		uint32_t rxParamSpaceOffsetQuadlets = 0;
		if (device.diceRegisters.count(rxParamSpaceOffsetAddr))
		{
			rxParamSpaceOffsetQuadlets = CFSwapInt32LittleToHost(device.diceRegisters[rxParamSpaceOffsetAddr]);
		}
		else
		{
			std::cerr << "Warning [DICE]: RX Parameter Space Offset not previously read. Cannot read RX stream details." << std::endl;
			return;
		}
		uint64_t rxParamSpaceBase = discoveredDiceBase + (rxParamSpaceOffsetQuadlets * 4);
		std::cerr << "Debug [DICE]: RX Parameter Space Base Address: 0x" << std::hex << rxParamSpaceBase << std::dec << std::endl;

		// Define key registers to check for each stream
		// We'll use a smaller set of critical registers as "probe" registers
		std::vector<std::pair<uint64_t, std::string>> probeRegisters = {
				{DICE_REGISTER_RX_ISOC_BASE, "ISOC"},
				{DICE_REGISTER_RX_NB_AUDIO_BASE, "Audio Channels"}};

		// Define all registers to read for valid streams
		std::map<uint64_t, std::string> allRxStreamRegs = {
				{DICE_REGISTER_RX_ISOC_BASE, "ISOC"},
				{DICE_REGISTER_RX_SEQ_START_BASE, "Sequence Start"},
				{DICE_REGISTER_RX_NB_AUDIO_BASE, "Audio Channels"},
				{DICE_REGISTER_RX_MIDI_BASE, "MIDI"},
				{DICE_REGISTER_RX_NAMES_BASE, "Names Base"},
				{DICE_REGISTER_RX_AC3_CAPABILITIES_BASE, "AC3 Capabilities"},
				{DICE_REGISTER_RX_AC3_ENABLE_BASE, "AC3 Enable"}};

		// Track valid streams and failures
		uint32_t validStreamCount = 0;
		uint32_t consecutiveFailures = 0;
		const uint32_t MAX_CONSECUTIVE_FAILURES = 3;

		// Progressive discovery loop
		for (uint32_t i = 0; i < maxStreamsToExplore; ++i)
		{
			uint64_t streamInstanceOffsetBytes = i * rxStreamSizeQuadlets * 4;
			bool streamValid = false;

			// Reduced logging - only log at higher level
			if (logger_)
				logger_->debug("Probing RX stream [{}]...", i);

			// Try to read the probe registers for this stream
			for (const auto &regPair : probeRegisters)
			{
				uint64_t regRelativeOffset = regPair.first;
				const std::string &regName = regPair.second;
				uint64_t fullAddr = rxParamSpaceBase + streamInstanceOffsetBytes + regRelativeOffset;
				UInt32 value = 0;

				IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, fullAddr, value, generation);
				if (status == kIOReturnSuccess)
				{
					// Successfully read this register
					device.diceRegisters[fullAddr] = value; // Store raw BE value
					UInt32 swappedValue = CFSwapInt32LittleToHost(value);
					std::string ascii = FWA::SCANNER::interpretAsASCII(swappedValue);

					// Reduce log verbosity - only log non-empty ASCII values or at debug level
					if (!ascii.empty())
					{
						std::cerr << "Info [DICE]: RX[" << i << "] " << regName << ": '" << ascii << "'" << std::endl;
					}
					// Detailed register info only at debug level
					if (logger_)
						logger_->debug("RX[{}] {} (0x{:x}): 0x{:x}", i, regName, fullAddr, swappedValue);

					streamValid = true; // Mark this stream as valid if at least one register reads successfully
				}
				else
				{
					std::cerr << "Warning [DICE]: Read failed for RX[" << i << "] " << regName
										<< " (0x" << std::hex << fullAddr << ") (status: " << status << ")" << std::dec << std::endl;

					// Don't increment consecutive failures here, only if the entire stream fails
				}
			}

			if (streamValid)
			{
				// This stream is valid, increment counter
				validStreamCount++;
				consecutiveFailures = 0; // Reset failure counter on success

				// Now read the rest of the registers for this stream
				for (const auto &regPair : allRxStreamRegs)
				{
					// Skip registers we already read during probing
					bool alreadyRead = false;
					for (const auto &probeReg : probeRegisters)
					{
						if (probeReg.first == regPair.first)
						{
							alreadyRead = true;
							break;
						}
					}
					if (alreadyRead)
						continue;

					uint64_t regRelativeOffset = regPair.first;
					const std::string &regName = regPair.second;
					uint64_t fullAddr = rxParamSpaceBase + streamInstanceOffsetBytes + regRelativeOffset;
					UInt32 value = 0;

					IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, fullAddr, value, generation);
					if (status == kIOReturnSuccess)
					{
						device.diceRegisters[fullAddr] = value; // Store raw BE value
						UInt32 swappedValue = CFSwapInt32LittleToHost(value);
						std::string ascii = FWA::SCANNER::interpretAsASCII(swappedValue);

						// Reduce log verbosity - only log non-empty ASCII values or at debug level
						if (!ascii.empty())
						{
							std::cerr << "Info [DICE]: RX[" << i << "] " << regName << ": '" << ascii << "'" << std::endl;
						}
						// Detailed register info only at debug level
						if (logger_)
							logger_->debug("RX[{}] {} (0x{:x}): 0x{:x}", i, regName, fullAddr, swappedValue);
					}
					else
					{
						std::cerr << "Warning [DICE]: Read failed for RX[" << i << "] " << regName
											<< " (0x" << std::hex << fullAddr << ") (status: " << status << ")" << std::dec << std::endl;
						// Non-critical register, continue anyway
					}
				}
			}
			else
			{
				// This entire stream failed, increment consecutive failures
				consecutiveFailures++;

				if (consecutiveFailures >= MAX_CONSECUTIVE_FAILURES)
				{
					std::cerr << "Info [DICE]: Stopping RX stream discovery after " << consecutiveFailures
										<< " consecutive stream failures." << std::endl;
					break;
				}
			}
		}

		// Update the device's stream count based on our discovery
		if (validStreamCount > 0)
		{
			if (validStreamCount != reportedStreamCount)
			{
				std::cerr << "Info [DICE]: Adjusting RX stream count from reported " << reportedStreamCount
									<< " to discovered " << validStreamCount << " based on successful reads." << std::endl;
				device.rxStreamCount = validStreamCount;
			}
			else
			{
				std::cerr << "Info [DICE]: Confirmed RX stream count of " << validStreamCount
									<< " matches reported count." << std::endl;
			}
		}
		else if (reportedStreamCount > 0)
		{
			std::cerr << "Warning [DICE]: No valid RX streams discovered. Keeping reported count of "
								<< reportedStreamCount << " for reference." << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE]: No valid RX streams discovered and none reported." << std::endl;
			device.rxStreamCount = 0;
		}
	}

} // namespace FWA::SCANNER