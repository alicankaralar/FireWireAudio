#include "dice_helpers.hpp"
#include "eap_helpers.hpp"						// Include EAP functions
#include "io_helpers.hpp"							// For safeReadQuadlet, interpretAsASCII
#include "config_rom.hpp"							// For parseConfigRom
#include "scanner.hpp"								// For FireWireDevice, DiceDefines.hpp constants
#include "utils_explore_general.hpp"	// For exploreDiceMemoryLayout, extractCoherentRegisterStrings
#include "utils_explore_channels.hpp" // For exploreChannelNamesArea
#include "utils_string.hpp"						// For extractStringsFromMemory, discoverChannelNamesAddress, validateChannelNumbers
#include "dice_stream_registers.hpp"	// For readDiceTxStreamRegisters, readDiceRxStreamRegisters

#include <iostream>
#include <iomanip> // For std::hex, std::setw, std::setfill
#include <map>
#include <string>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost
#include <IOKit/firewire/IOFireWireLib.h>
#include <IOKit/firewire/IOFireWireFamilyCommon.h> // For kConfigDirectoryKey_Unit_Dependent (FWStandardHeaders.h is deprecated/moved)

namespace FWA::SCANNER
{

	// --- Static Helper Functions (Internal to this file) ---

	namespace
	{ // Anonymous namespace for internal linkage

		// Helper to read common global DICE registers
		static void readDiceGlobalRegistersInternal(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
																								FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation)
		{
			std::map<uint64_t, std::string> registersToRead = {
					// Base Parameter Space Offsets/Sizes (read these first)
					{DICE_REGISTER_GLOBAL_PAR_SPACE_OFF, "Global Parameter Space Offset (Quadlets)"},
					{DICE_REGISTER_GLOBAL_PAR_SPACE_SZ, "Global Parameter Space Size (Quadlets)"},
					{DICE_REGISTER_TX_PAR_SPACE_OFF, "TX Parameter Space Offset (Quadlets)"},
					{DICE_REGISTER_TX_PAR_SPACE_SZ, "TX Parameter Space Size (Quadlets)"},
					{DICE_REGISTER_RX_PAR_SPACE_OFF, "RX Parameter Space Offset (Quadlets)"},
					{DICE_REGISTER_RX_PAR_SPACE_SZ, "RX Parameter Space Size (Quadlets)"},
					// Global Registers (offsets relative to DISCOVERED base)
					{DICE_REGISTER_GLOBAL_OWNER, "Owner"},
					{DICE_REGISTER_GLOBAL_NOTIFICATION, "Notification"},
					{DICE_REGISTER_GLOBAL_NICK_NAME, "Nick Name Base"}, // Note: This is base, actual name needs block read
					{DICE_REGISTER_GLOBAL_CLOCK_SELECT, "Clock Select"},
					{DICE_REGISTER_GLOBAL_ENABLE, "Enable"},
					{DICE_REGISTER_GLOBAL_STATUS, "Status"},
					{DICE_REGISTER_GLOBAL_EXTENDED_STATUS, "Extended Status"},
					{DICE_REGISTER_GLOBAL_SAMPLE_RATE, "Sample Rate"},
					{DICE_REGISTER_GLOBAL_VERSION, "Version"}, // Already read, but read again for consistency here? Or skip? Skip for now.
					{DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES, "Clock Capabilities"},
					{DICE_REGISTER_GLOBAL_CLOCKSOURCENAMES, "Clock Source Names Base"}, // Note: Base address
			};

			std::cerr << "Debug [DICE]: Reading global registers relative to base 0x" << std::hex << discoveredDiceBase << std::dec << std::endl;

			for (const auto &pair : registersToRead)
			{
				uint64_t regOffset = pair.first;
				const std::string &regName = pair.second;
				uint64_t fullAddr = discoveredDiceBase + regOffset;
				UInt32 value = 0;

				IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, fullAddr, value, generation);
				if (status == kIOReturnSuccess)
				{
					device.diceRegisters[fullAddr] = value; // Store raw BE value
					std::cerr << "Debug [DICE]: Read success for Global " << regName << " (0x" << std::hex << fullAddr << ") -> 0x" << CFSwapInt32LittleToHost(value) << std::dec << std::endl;
				}
				else
				{
					std::cerr << "Warning [DICE]: Read failed for Global " << regName << " (0x" << std::hex << fullAddr << ") (status: " << status << ")" << std::dec << std::endl;
				}
			}
		}

	} // Anonymous namespace

	// --- Targeted Register Reading Functions ---

	static void readGpcsrRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
																 FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation)
	{
		std::cerr << "Debug [DICE]: Reading GPCSR registers..." << std::endl;
		uint64_t gpcsrBase = DICE_REGISTER_GPCSR_BASE;

		// Read GPCSR_CHIP_ID
		uint64_t chipIdAddr = gpcsrBase + DICE_REGISTER_GPCSR_CHIP_ID;
		UInt32 chipIdValue = 0;
		IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, chipIdAddr, chipIdValue, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[chipIdAddr] = chipIdValue; // Store raw BE value
			uint32_t hostValue = CFSwapInt32LittleToHost(chipIdValue);
			uint32_t chipType = (hostValue & DICE_GPCSR_CHIP_ID_CHIP_TYPE_MASK) >> DICE_GPCSR_CHIP_ID_CHIP_TYPE_SHIFT;
			// uint32_t chipId = (hostValue & DICE_GPCSR_CHIP_ID_CHIP_ID_MASK) >> DICE_GPCSR_CHIP_ID_CHIP_ID_SHIFT; // Not currently stored

			switch (chipType)
			{
			case static_cast<uint32_t>(FWA::DICE::DiceChipType::DiceII):
				device.diceChipType = FWA::DICE::DiceChipType::DiceII;
				std::cerr << "Debug [DICE]: Detected Chip Type: DICE II" << std::endl;
				break;
			case static_cast<uint32_t>(FWA::DICE::DiceChipType::DiceMini):
				device.diceChipType = FWA::DICE::DiceChipType::DiceMini;
				std::cerr << "Debug [DICE]: Detected Chip Type: DICE Mini" << std::endl;
				break;
			case static_cast<uint32_t>(FWA::DICE::DiceChipType::DiceJr):
				device.diceChipType = FWA::DICE::DiceChipType::DiceJr;
				std::cerr << "Debug [DICE]: Detected Chip Type: DICE Jr" << std::endl;
				break;
			default:
				device.diceChipType = FWA::DICE::DiceChipType::Unknown;
				std::cerr << "Warning [DICE]: Unknown Chip Type: " << chipType << std::endl;
				break;
			}
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read GPCSR_CHIP_ID (0x" << std::hex << chipIdAddr << ") (status: " << status << ")" << std::dec << std::endl;
		}

		// Read GPCSR_AUDIO_SELECT
		uint64_t audioSelectAddr = gpcsrBase + DICE_REGISTER_GPCSR_AUDIO_SELECT;
		UInt32 audioSelectValue = 0;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, audioSelectAddr, audioSelectValue, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[audioSelectAddr] = audioSelectValue; // Store raw BE value
			uint32_t hostValue = CFSwapInt32LittleToHost(audioSelectValue);
			std::cerr << "Debug [DICE]: Read GPCSR_AUDIO_SELECT (0x" << std::hex << audioSelectAddr << "): 0x" << hostValue << std::dec << std::endl;
			// TODO: Interpret bitfields and store relevant info in DiscoveredDevice (e.g., flags for enabled interfaces)
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read GPCSR_AUDIO_SELECT (0x" << std::hex << audioSelectAddr << ") (status: " << status << ")" << std::dec << std::endl;
		}
	}

	static void readClockControllerRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
																					 FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation)
	{
		std::cerr << "Debug [DICE]: Reading Clock Controller registers..." << std::endl;
		uint64_t clockControllerBase = DICE_REGISTER_CLOCK_CONTROLLER_BASE;

		// Read SYNC_CTRL
		uint64_t syncCtrlAddr = clockControllerBase + DICE_REGISTER_CLOCK_CONTROLLER_SYNC_CTRL;
		UInt32 syncCtrlValue = 0;
		IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, syncCtrlAddr, syncCtrlValue, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[syncCtrlAddr] = syncCtrlValue; // Store raw BE value
			uint32_t hostValue = CFSwapInt32LittleToHost(syncCtrlValue);
			uint32_t syncSrc = (hostValue & DICE_CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_MASK) >> DICE_CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_SHIFT;

			switch (syncSrc)
			{
			case 0:
				device.syncSource = FWA::DICE::ClockSource::AES0;
				break;
			case 1:
				device.syncSource = FWA::DICE::ClockSource::AES1;
				break;
			case 2:
				device.syncSource = FWA::DICE::ClockSource::AES2;
				break;
			case 3:
				device.syncSource = FWA::DICE::ClockSource::AES3;
				break;
			case 4:
				device.syncSource = FWA::DICE::ClockSource::SlaveInputs;
				break;
			case 5:
				device.syncSource = FWA::DICE::ClockSource::HPLL;
				break;
			case 6:
				device.syncSource = FWA::DICE::ClockSource::Internal;
				break;
			default:
				device.syncSource = FWA::DICE::ClockSource::Unknown;
				break;
			}
			std::cerr << "Debug [DICE]: Read SYNC_CTRL (0x" << std::hex << syncCtrlAddr << "): Sync Source = " << static_cast<int>(device.syncSource) << std::dec << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read CLOCK_CONTROLLER_SYNC_CTRL (0x" << std::hex << syncCtrlAddr << ") (status: " << status << ")" << std::dec << std::endl;
		}

		// Read DOMAIN_CTRL
		uint64_t domainCtrlAddr = clockControllerBase + DICE_REGISTER_CLOCK_CONTROLLER_DOMAIN_CTRL;
		UInt32 domainCtrlValue = 0;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, domainCtrlAddr, domainCtrlValue, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[domainCtrlAddr] = domainCtrlValue; // Store raw BE value
			uint32_t hostValue = CFSwapInt32LittleToHost(domainCtrlValue);
			uint32_t rtrFs = (hostValue & DICE_CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_MASK) >> DICE_CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_SHIFT;

			switch (rtrFs)
			{
			case 0:
				device.routerFsMode = FWA::DICE::RouterFrameSyncMode::BaseRate;
				break;
			case 1:
				device.routerFsMode = FWA::DICE::RouterFrameSyncMode::DoubleRate;
				break;
			case 2:
				device.routerFsMode = FWA::DICE::RouterFrameSyncMode::QuadRate;
				break;
			default:
				device.routerFsMode = FWA::DICE::RouterFrameSyncMode::Unknown;
				break;
			}
			std::cerr << "Debug [DICE]: Read DOMAIN_CTRL (0x" << std::hex << domainCtrlAddr << "): Router FS Mode = " << static_cast<int>(device.routerFsMode) << std::dec << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read CLOCK_CONTROLLER_DOMAIN_CTRL (0x" << std::hex << domainCtrlAddr << ") (status: " << status << ")" << std::dec << std::endl;
		}
	}

	static void readAesReceiverRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
																			 FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation)
	{
		std::cerr << "Debug [DICE]: Reading AES Receiver registers..." << std::endl;
		uint64_t aesReceiverBase = DICE_REGISTER_AES_RECEIVER_BASE;

		// Read STAT_ALL
		uint64_t statAllAddr = aesReceiverBase + DICE_REGISTER_AES_RECEIVER_STAT_ALL;
		UInt32 statAllValue = 0;
		IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, statAllAddr, statAllValue, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[statAllAddr] = statAllValue; // Store raw BE value
			uint32_t hostValue = CFSwapInt32LittleToHost(statAllValue);
			device.aesLocked = (hostValue & DICE_AES_RECEIVER_STAT_ALL_LOCK_BIT) != 0;
			std::cerr << "Debug [DICE]: Read AES_RECEIVER_STAT_ALL (0x" << std::hex << statAllAddr << "): Locked = " << (device.aesLocked ? "true" : "false") << std::dec << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read AES_RECEIVER_STAT_ALL (0x" << std::hex << statAllAddr << ") (status: " << status << ")" << std::dec << std::endl;
		}
	}

	static void readAudioMixerRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
																			FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation)
	{
		std::cerr << "Debug [DICE]: Reading Audio Mixer registers..." << std::endl;
		uint64_t audioMixerBase = DICE_REGISTER_AUDIO_MIXER_BASE;

		// Read MIXER_NUMOFCH
		uint64_t numOfChAddr = audioMixerBase + DICE_REGISTER_AUDIO_MIXER_NUMOFCH;
		UInt32 numOfChValue = 0;
		IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, numOfChAddr, numOfChValue, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[numOfChAddr] = numOfChValue; // Store raw BE value
			uint32_t hostValue = CFSwapInt32LittleToHost(numOfChValue);
			device.mixerRxChannels = static_cast<uint8_t>(hostValue & 0xFF); // Assuming lower 8 bits for channel count
			std::cerr << "Debug [DICE]: Read AUDIO_MIXER_NUMOFCH (0x" << std::hex << numOfChAddr << "): RX Channels = " << static_cast<int>(device.mixerRxChannels) << std::dec << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read AUDIO_MIXER_NUMOFCH (0x" << std::hex << numOfChAddr << ") (status: " << status << ")" << std::dec << std::endl;
		}
	}

	static void readAvsRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
															 FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation)
	{
		std::cerr << "Debug [DICE]: Reading AVS registers..." << std::endl;
		uint64_t avsBase = DICE_REGISTER_AVS_SUB_SYSTEM_BASE;

		// Read AVS Audio Receiver Config (for receiver 0)
		uint64_t arxCfg0Addr = avsBase + DICE_REGISTER_AVS_AUDIO_RECEIVER_CFG0;
		UInt32 arxCfg0Value = 0;
		IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, arxCfg0Addr, arxCfg0Value, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[arxCfg0Addr] = arxCfg0Value; // Store raw BE value
			uint32_t hostValue = CFSwapInt32LittleToHost(arxCfg0Value);
			device.avsRxChannelId = static_cast<uint8_t>((hostValue & DICE_AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_MASK) >> DICE_AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_SHIFT);
			std::cerr << "Debug [DICE]: Read ARX0_CFG0 (0x" << std::hex << arxCfg0Addr << "): Channel ID = " << static_cast<int>(device.avsRxChannelId) << std::dec << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read ARX0_CFG0 (0x" << std::hex << arxCfg0Addr << ") (status: " << status << ")" << std::dec << std::endl;
		}

		uint64_t arxCfg1Addr = avsBase + DICE_REGISTER_AVS_AUDIO_RECEIVER_CFG1;
		UInt32 arxCfg1Value = 0;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, arxCfg1Addr, arxCfg1Value, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[arxCfg1Addr] = arxCfg1Value; // Store raw BE value
			uint32_t hostValue = CFSwapInt32LittleToHost(arxCfg1Value);
			device.avsRxDataBlockSize = static_cast<uint8_t>((hostValue & DICE_AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_MASK) >> DICE_AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_SHIFT);
			std::cerr << "Debug [DICE]: Read ARX0_CFG1 (0x" << std::hex << arxCfg1Addr << "): Specified DBS = " << static_cast<int>(device.avsRxDataBlockSize) << std::dec << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read ARX0_CFG1 (0x" << std::hex << arxCfg1Addr << ") (status: " << status << ")" << std::dec << std::endl;
		}

		// Read AVS Audio Transmitter Config (for transmitter 0)
		uint64_t atxCfgAddr = avsBase + DICE_REGISTER_AVS_AUDIO_TRANSMITTER_CFG;
		UInt32 atxCfgValue = 0;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, atxCfgAddr, atxCfgValue, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[atxCfgAddr] = atxCfgValue; // Store raw BE value
			uint32_t hostValue = CFSwapInt32LittleToHost(atxCfgValue);
			device.avsTxDataBlockSize = static_cast<uint8_t>((hostValue & DICE_AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_MASK) >> DICE_AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_SHIFT);
			uint32_t sysMode = (hostValue & DICE_AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_MASK) >> DICE_AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_SHIFT;

			switch (sysMode)
			{
			case 0:
				device.avsTxSystemMode = FWA::DICE::AvsSystemMode::Low;
				break;
			case 1:
				device.avsTxSystemMode = FWA::DICE::AvsSystemMode::Mid;
				break;
			case 2:
				device.avsTxSystemMode = FWA::DICE::AvsSystemMode::High;
				break;
			default:
				device.avsTxSystemMode = FWA::DICE::AvsSystemMode::Unknown;
				break;
			}
			std::cerr << "Debug [DICE]: Read ATX0_CFG (0x" << std::hex << atxCfgAddr << "): Data Block Size = " << static_cast<int>(device.avsTxDataBlockSize)
								<< ", System Mode = " << static_cast<int>(device.avsTxSystemMode) << std::dec << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read AVS_AUDIO_TRANSMITTER_CFG (0x" << std::hex << atxCfgAddr << ") (status: " << status << ")" << std::dec << std::endl;
		}
	}

	// --- Public DICE Helper Function Implementations ---
	// EAP functions moved to eap_helpers.cpp
	void setDefaultDiceConfig(FireWireDevice &device)
	{
		std::cerr << "Debug [DICE]: Setting default configuration for ";
		switch (device.diceChipType)
		{
		case FWA::DICE::DiceChipType::DiceII:
			std::cerr << "DICE II";
			break;
		case FWA::DICE::DiceChipType::DiceMini:
			std::cerr << "DICE Mini";
			break;
		case FWA::DICE::DiceChipType::DiceJr:
			std::cerr << "DICE Jr";
			break;
		default:
			std::cerr << "Unknown";
			break;
		}
		std::cerr << " chip type." << std::endl;

		// Default values based on chip type
		switch (device.diceChipType)
		{
		case FWA::DICE::DiceChipType::DiceII:
			device.txStreamCount = 4;
			device.rxStreamCount = 4;
			break;
		case FWA::DICE::DiceChipType::DiceMini:
			device.txStreamCount = 2;
			device.rxStreamCount = 2;
			break;
		case FWA::DICE::DiceChipType::DiceJr:
			device.txStreamCount = 1;
			device.rxStreamCount = 1;
			break;
		default:
			// For unknown types, assume a conservative 2 streams each if not already set
			if (device.txStreamCount == 0)
				device.txStreamCount = 2;
			if (device.rxStreamCount == 0)
				device.rxStreamCount = 2;
			break;
		}

		std::cerr << "Debug [DICE]: Using default TX streams: " << device.txStreamCount << std::endl;
		std::cerr << "Debug [DICE]: Using default RX streams: " << device.rxStreamCount << std::endl;
	}

	void readDiceRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service, FireWireDevice &device)
	{
		if (!deviceInterface || !*deviceInterface)
		{
			std::cerr << "Error [DICE]: Invalid device interface passed to readDiceRegisters." << std::endl;
			return;
		}

		// Get the current generation
		UInt32 generation = 0;
		std::cerr << "Debug [DICE]: Calling GetBusGeneration..." << std::endl;
		IOReturn status = (*deviceInterface)->GetBusGeneration(deviceInterface, &generation);
		std::cerr << "Debug [DICE]: GetBusGeneration result: " << status << ", generation: " << generation << std::endl;
		if (status != kIOReturnSuccess)
		{
			std::cerr << "Error [DICE]: Failed to get bus generation (status: " << status << "). Aborting register read." << std::endl;
			return;
		}

		// --- Discover Base Address via Config ROM ---
		std::cerr << "Debug [DICE]: Preparing to parse Config ROM to find DICE base..." << std::endl;
		uint64_t discoveredDiceBase = DICE_INVALID_OFFSET;
		uint32_t targetKeyUsed = 0;
		const uint32_t DICE_UNIT_DEPENDENT_KEY_D1 = 0xD1;

		// Try standard Unit Dependent key (0xC1)
		discoveredDiceBase = FWA::SCANNER::parseConfigRom(deviceInterface, service, kConfigDirectoryKey_Unit_Dependent, generation);
		if (discoveredDiceBase != DICE_INVALID_OFFSET)
		{
			targetKeyUsed = kConfigDirectoryKey_Unit_Dependent;
		}
		else
		{
			// Try the observed 0xD1 key
			discoveredDiceBase = FWA::SCANNER::parseConfigRom(deviceInterface, service, DICE_UNIT_DEPENDENT_KEY_D1, generation);
			if (discoveredDiceBase != DICE_INVALID_OFFSET)
			{
				targetKeyUsed = DICE_UNIT_DEPENDENT_KEY_D1;
			}
		}

		if (discoveredDiceBase == DICE_INVALID_OFFSET)
		{
			std::cerr << "Warning [DICE]: Could not find standard DICE Unit Dependent Directory keys (0xC1, 0xD1) via Config ROM. Falling back to hardcoded base 0x"
								<< std::hex << DICE_REGISTER_BASE << std::dec << ". Register reads likely to fail." << std::endl;
			discoveredDiceBase = DICE_REGISTER_BASE; // Final fallback
			targetKeyUsed = 0;
		}
		else
		{
			std::cerr << "Info [DICE]: Discovered DICE Base Address via key 0x" << std::hex << targetKeyUsed << ": 0x" << discoveredDiceBase << std::dec << std::endl;
		}
		// --- End Config ROM Parse ---

		// --- Try EAP First ---
		// TODO: Update EAP functions to use discoveredDiceBase if necessary
		bool eapCapabilitiesRead = readDiceEAPCapabilities(deviceInterface, service, device, generation);
		bool eapConfigRead = false;
		if (eapCapabilitiesRead)
		{
			std::cerr << "Debug [DICE]: Detected DICE chipset via EAP: ";
			switch (device.diceChipType)
			{
			case FWA::DICE::DiceChipType::DiceII:
				std::cerr << "DICE II";
				break;
			case FWA::DICE::DiceChipType::DiceMini:
				std::cerr << "DICE Mini";
				break;
			case FWA::DICE::DiceChipType::DiceJr:
				std::cerr << "DICE Jr";
				break;
			default:
				std::cerr << "Unknown (" << static_cast<int>(device.diceChipType) << ")";
				break;
			}
			std::cerr << std::endl;
			eapConfigRead = readDiceEAPCurrentConfig(deviceInterface, service, device, generation);
		}
		if (!eapCapabilitiesRead || !eapConfigRead)
		{
			// If EAP failed or didn't provide config, set defaults based on chip type (if known) or generic defaults
			setDefaultDiceConfig(device);
		}
		// --- End EAP ---

		// --- Test Reads ---
		const uint64_t CSR_STATE_CLEAR_ADDR = 0xFFFFF000000CULL;
		UInt32 csrValue = 0;
		std::cerr << "Debug [DICE]: Attempting test read of CSR STATE_CLEAR (0x" << std::hex << CSR_STATE_CLEAR_ADDR << ")..." << std::dec << std::endl;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, CSR_STATE_CLEAR_ADDR, csrValue, generation);
		if (status != kIOReturnSuccess)
		{
			std::cerr << "Error [DICE]: Failed test read of CSR STATE_CLEAR (status: " << status << "). Basic communication might be failing." << std::endl;
		}
		else
		{
			std::cerr << "Debug [DICE]: Successfully read CSR STATE_CLEAR: 0x" << std::hex << csrValue << std::dec << std::endl;
			// device.diceRegisters[CSR_STATE_CLEAR_ADDR] = csrValue; // Optionally store CSR value
		}

		UInt32 versionValue = 0;
		uint64_t versionRegAddr = discoveredDiceBase + DICE_REGISTER_GLOBAL_VERSION;
		std::cerr << "Debug [DICE]: Attempting test read of DICE VERSION register (at 0x" << std::hex << versionRegAddr << ")..." << std::dec << std::endl;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, versionRegAddr, versionValue, generation);
		if (status == kIOReturnSuccess)
		{
			std::cerr << "Debug [DICE]: Successfully read VERSION register: 0x" << std::hex << versionValue << std::dec << std::endl;
			device.diceRegisters[versionRegAddr] = versionValue; // Store raw BE value
		}
		else
		{
			std::cerr << "Error [DICE]: Failed test read of VERSION register (status: " << status << ")." << std::endl;
		}
		// --- End Test Reads ---

		// --- Read Standard DICE Registers using Helpers ---
		readDiceGlobalRegistersInternal(deviceInterface, service, device, discoveredDiceBase, generation);

		// --- Read Targeted Registers ---
		readGpcsrRegisters(deviceInterface, service, device, discoveredDiceBase, generation);
		readClockControllerRegisters(deviceInterface, service, device, discoveredDiceBase, generation);
		readAesReceiverRegisters(deviceInterface, service, device, discoveredDiceBase, generation);
		readAudioMixerRegisters(deviceInterface, service, device, discoveredDiceBase, generation);
		readAvsRegisters(deviceInterface, service, device, discoveredDiceBase, generation);

		// Determine stream sizes (needed for stream register helpers)
		uint32_t txStreamSizeQuadlets = 0;
		uint64_t txSizeAddr = discoveredDiceBase + DICE_REGISTER_TX_SZ_TX;
		if (device.diceRegisters.count(txSizeAddr))
		{
			txStreamSizeQuadlets = CFSwapInt32LittleToHost(device.diceRegisters[txSizeAddr]);
			if (txStreamSizeQuadlets == 0 || txStreamSizeQuadlets > 1024)
			{ // Sanity check
				std::cerr << "Warning [DICE]: Invalid TX stream size read (" << txStreamSizeQuadlets << "). Assuming 256." << std::endl;
				txStreamSizeQuadlets = 256;
			}
		}
		else
		{
			std::cerr << "Warning [DICE]: Could not read TX stream size register. Assuming 256." << std::endl;
			txStreamSizeQuadlets = 256; // Default if read fails
		}

		uint32_t rxStreamSizeQuadlets = 0;
		uint64_t rxSizeAddr = discoveredDiceBase + DICE_REGISTER_RX_SZ_RX;
		if (device.diceRegisters.count(rxSizeAddr))
		{
			rxStreamSizeQuadlets = CFSwapInt32LittleToHost(device.diceRegisters[rxSizeAddr]);
			if (rxStreamSizeQuadlets == 0 || rxStreamSizeQuadlets > 1024)
			{ // Sanity check
				std::cerr << "Warning [DICE]: Invalid RX stream size read (" << rxStreamSizeQuadlets << "). Assuming 256." << std::endl;
				rxStreamSizeQuadlets = 256;
			}
		}
		else
		{
			std::cerr << "Warning [DICE]: Could not read RX stream size register. Assuming 256." << std::endl;
			rxStreamSizeQuadlets = 256; // Default if read fails
		}

		// Read stream counts from registers
		uint64_t txCountAddr = discoveredDiceBase + DICE_REGISTER_TX_NB_TX;
		uint32_t rawTxCount = 0;
		IOReturn txCountStatus = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, txCountAddr, rawTxCount, generation);
		if (txCountStatus == kIOReturnSuccess)
		{
			device.diceRegisters[txCountAddr] = rawTxCount; // Store raw BE value
			device.txStreamCount = CFSwapInt32LittleToHost(rawTxCount);
			std::cerr << "Debug [DICE]: Read TX stream count from register (0x" << std::hex << txCountAddr << "): " << device.txStreamCount << std::dec << std::endl;

			// Validate TX stream count
			if (device.txStreamCount > 64) // Assuming more than 64 streams is unreasonable for typical devices
			{
				std::cerr << "Warning [DICE]: Unreasonable TX stream count detected (" << device.txStreamCount << "). This might indicate an incorrect register read or an unusual device configuration. Proceeding with caution, but results may be unreliable." << std::endl;
				// Optionally, you could set a maximum reasonable value here, e.g.:
				// device.txStreamCount = 64;
			}
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read TX stream count register (0x" << std::hex << txCountAddr << ") (status: " << txCountStatus << "). Using default TX stream count: " << device.txStreamCount << std::dec << std::endl;
			// Keep the default count set by setDefaultDiceConfig if read fails
		}

		uint64_t rxCountAddr = discoveredDiceBase + DICE_REGISTER_RX_NB_RX;
		uint32_t rawRxCount = 0;
		IOReturn rxCountStatus = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, rxCountAddr, rawRxCount, generation);
		if (rxCountStatus == kIOReturnSuccess)
		{
			device.diceRegisters[rxCountAddr] = rawRxCount; // Store raw BE value
			device.rxStreamCount = CFSwapInt32LittleToHost(rawRxCount);
			std::cerr << "Debug [DICE]: Read RX stream count from register (0x" << std::hex << rxCountAddr << "): " << device.rxStreamCount << std::dec << std::endl;

			// Validate RX stream count
			if (device.rxStreamCount > 64) // Assuming more than 64 streams is unreasonable
			{
				std::cerr << "Warning [DICE]: Unreasonable RX stream count detected (" << device.rxStreamCount << "). This might indicate an incorrect register read or an unusual device configuration. Proceeding with caution, but results may be unreliable." << std::endl;
				// Optionally, you could set a maximum reasonable value here, e.g.:
				// device.rxStreamCount = 64;
			}
		}
		else
		{
			std::cerr << "Warning [DICE]: Failed to read RX stream count register (0x" << std::hex << rxCountAddr << ") (status: " << rxCountStatus << "). Using default RX stream count: " << device.rxStreamCount << std::dec << std::endl;
			// Keep the default count set by setDefaultDiceConfig if read fails
		}

		std::cerr << "Debug [DICE]: Final stream counts: TX=" << device.txStreamCount << ", RX=" << device.rxStreamCount << std::endl;

		readDiceTxStreamRegisters(deviceInterface, service, device, discoveredDiceBase, generation, txStreamSizeQuadlets);
		readDiceRxStreamRegisters(deviceInterface, service, device, discoveredDiceBase, generation, rxStreamSizeQuadlets);
		// --- End Standard DICE Register Reads ---

		// --- Memory exploration (call helper from utils.cpp) ---
		// Note: This is kept separate as it's purely for debugging/discovery
		std::cerr << "\n\n========== MEMORY EXPLORATION ==========\n"
							<< std::endl;

		// Explore the standard DICE register area
		FWA::SCANNER::exploreDiceMemoryLayout(deviceInterface, service, device, generation, DICE_REGISTER_BASE); // Needs definition in utils.cpp

		// Also explore the alternate address range where we found strings in previous scans
		std::cerr << "\n--- Exploring Alternate Address Range ---\n"
							<< std::endl;
		FWA::SCANNER::exploreDiceMemoryLayout(deviceInterface, service, device, generation, 0xffffe0000000ULL);

		std::cerr << "\n========== END MEMORY EXPLORATION ==========\n\n"
							<< std::endl;

		// Perform a detailed exploration of the channel names area
		FWA::SCANNER::exploreChannelNamesArea(deviceInterface, service, device, generation);
	}

} // namespace FWA::SCANNER