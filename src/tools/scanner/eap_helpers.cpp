#include "eap_helpers.hpp"
#include "io_helpers.hpp" // For safeReadQuadlet
#include "scanner.hpp"		// For FireWireDevice, DiceDefines.hpp constants

#include <iostream>
#include <iomanip> // For std::hex

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER
{

	// --- EAP Helper Function Implementations ---

	bool readDiceEAPCapabilities(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
															 FireWireDevice &device, UInt32 generation)
	{
		std::cerr << "Debug [DICE EAP]: Attempting to read DICE EAP capabilities..." << std::endl;

		// Determine base address (use hardcoded standard base for EAP offset read)
		// This assumes the EAP offset register itself is at the standard DICE base.
		uint64_t diceBaseForEAPOffsetRead = DICE_REGISTER_BASE;

		// 1. Read the EAP Capability Space Offset
		uint64_t eapCapOffsetAddr = diceBaseForEAPOffsetRead + DICE_EAP_CAPABILITY_SPACE_OFF;
		UInt32 eapCapOffsetQuadlets = 0;
		IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, eapCapOffsetAddr, eapCapOffsetQuadlets, generation);

		if (status != kIOReturnSuccess)
		{
			std::cerr << "Warning [DICE EAP]: Failed to read EAP capability space offset (status: " << status << "). EAP may not be supported." << std::endl;
			return false;
		}
		// The actual EAP space base address is relative to the *device's* DICE base (which might differ)
		// We need the discovered base here. For now, assume standard base.
		// TODO: Pass discoveredDiceBase into this function if it can differ.
		uint64_t eapCapBaseAddr = diceBaseForEAPOffsetRead + (eapCapOffsetQuadlets * 4); // Convert quadlets to bytes
		std::cerr << "Debug [DICE EAP]: EAP Capability Space Base Address: 0x" << std::hex << eapCapBaseAddr << std::dec << std::endl;

		// 2. Read the General Capability Register from the EAP space
		uint64_t generalCapAddr = eapCapBaseAddr + DICE_EAP_CAPABILITY_GENERAL;
		UInt32 generalCapValue = 0;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, generalCapAddr, generalCapValue, generation);

		if (status != kIOReturnSuccess)
		{
			std::cerr << "Warning [DICE EAP]: Failed to read EAP General Capability register (status: " << status << ")." << std::endl;
			return false; // If this fails, EAP is likely unusable
		}

		// Store the raw capability value
		device.diceRegisters[generalCapAddr] = generalCapValue;

		// 3. Extract Chip Type
		int chipTypeValue = (generalCapValue >> DICE_EAP_CAP_GENERAL_CHIP) & 0xFF;
		device.diceChipType = static_cast<FWA::DICE::DiceChipType>(chipTypeValue);

		std::cerr << "Debug [DICE EAP]: Read EAP General Capability: 0x" << std::hex << generalCapValue << std::dec << std::endl;
		// Chip type logging happens in the calling function (readDiceRegisters)

		// 4. Read Router Capability Register
		uint64_t routerCapAddr = eapCapBaseAddr + DICE_EAP_CAPABILITY_ROUTER;
		UInt32 routerCapValue = 0;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, routerCapAddr, routerCapValue, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[routerCapAddr] = routerCapValue;
			std::cerr << "Debug [DICE EAP]: Read EAP Router Capability: 0x" << std::hex << routerCapValue << std::dec << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE EAP]: Failed to read EAP Router Capability register (status: " << status << ")." << std::endl;
		}

		// 5. Read Mixer Capability Register
		uint64_t mixerCapAddr = eapCapBaseAddr + DICE_EAP_CAPABILITY_MIXER;
		UInt32 mixerCapValue = 0;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, mixerCapAddr, mixerCapValue, generation);
		if (status == kIOReturnSuccess)
		{
			device.diceRegisters[mixerCapAddr] = mixerCapValue;
			std::cerr << "Debug [DICE EAP]: Read EAP Mixer Capability: 0x" << std::hex << mixerCapValue << std::dec << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE EAP]: Failed to read EAP Mixer Capability register (status: " << status << ")." << std::endl;
		}

		return true; // Indicate EAP capabilities were at least partially read
	}

	bool readDiceEAPCurrentConfig(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
																FireWireDevice &device, UInt32 generation)
	{
		std::cerr << "Debug [DICE EAP]: Attempting to read EAP current configuration..." << std::endl;

		// Assume standard base for reading the offset register itself
		uint64_t diceBaseForEAPOffsetRead = DICE_REGISTER_BASE;

		// 1. Read the EAP Current Config Space Offset
		uint64_t eapCurrCfgOffsetAddr = diceBaseForEAPOffsetRead + DICE_EAP_CURR_CFG_SPACE_OFF;
		UInt32 eapCurrCfgOffsetQuadlets = 0;
		IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, eapCurrCfgOffsetAddr, eapCurrCfgOffsetQuadlets, generation);

		if (status != kIOReturnSuccess)
		{
			std::cerr << "Warning [DICE EAP]: Failed to read EAP Current Config space offset (status: " << status << "). Cannot read config." << std::endl;
			return false;
		}
		// TODO: Pass discoveredDiceBase into this function if it can differ.
		uint64_t eapCurrCfgBaseAddr = diceBaseForEAPOffsetRead + (eapCurrCfgOffsetQuadlets * 4); // Convert quadlets to bytes
		std::cerr << "Debug [DICE EAP]: EAP Current Config Space Base Address: 0x" << std::hex << eapCurrCfgBaseAddr << std::dec << std::endl;

		// Determine which config block to read based on current sample rate (if known)
		// For now, just try reading the LOW config block as an example
		// TODO: Read sample rate first and select appropriate block (LOW/MID/HIGH)
		uint64_t configBlockBase = eapCurrCfgBaseAddr + DICE_EAP_CURRCFG_LOW_STREAM; // Example: Low rate stream config
		device.currentConfig = FWA::DICE::DiceConfig::Low;													 // Assume Low for now - Note: DiceConfig is an enum, not a macro, so namespace is correct here.

		// Read number of TX streams from the config block
		uint64_t txCountAddr = configBlockBase + 0; // Offset 0 within stream config block for TX count
		UInt32 txCountValue = 0;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, txCountAddr, txCountValue, generation);
		if (status == kIOReturnSuccess)
		{
			// Apply sanity check/limit
			if (txCountValue > 64)
			{
				std::cerr << "Warning [DICE EAP]: TX stream count " << txCountValue << " from EAP exceeds maximum reasonable value. Using previous/default." << std::endl;
				// Don't overwrite potentially valid default/register value
			}
			else
			{
				device.txStreamCount = txCountValue;
			}
			device.diceRegisters[txCountAddr] = txCountValue; // Store raw value read
			std::cerr << "Debug [DICE EAP]: Read TX Stream Count from EAP Config: " << device.txStreamCount << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE EAP]: Failed to read TX Stream Count from EAP Config (status: " << status << ")." << std::endl;
			return false; // If we can't read this, likely can't read others
		}

		// Read number of RX streams from the config block
		uint64_t rxCountAddr = configBlockBase + 4; // Offset 4 within stream config block for RX count
		UInt32 rxCountValue = 0;
		status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, rxCountAddr, rxCountValue, generation);
		if (status == kIOReturnSuccess)
		{
			if (rxCountValue > 64)
			{
				std::cerr << "Warning [DICE EAP]: RX stream count " << rxCountValue << " from EAP exceeds maximum reasonable value. Using previous/default." << std::endl;
			}
			else
			{
				device.rxStreamCount = rxCountValue;
			}
			device.diceRegisters[rxCountAddr] = rxCountValue; // Store raw value read
			std::cerr << "Debug [DICE EAP]: Read RX Stream Count from EAP Config: " << device.rxStreamCount << std::endl;
		}
		else
		{
			std::cerr << "Warning [DICE EAP]: Failed to read RX Stream Count from EAP Config (status: " << status << ")." << std::endl;
			// Continue even if this fails, maybe TX count was useful
		}

		// TODO: Read detailed stream configurations (channels, MIDI, etc.) if needed

		return true;
	}

} // namespace FWA::SCANNER