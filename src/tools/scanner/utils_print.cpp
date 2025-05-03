#include "utils_print.hpp"
#include "io_helpers.hpp" // For interpretAsASCII
#include "scanner.hpp"		// For FireWireDevice, DiceDefines.hpp constants

#include <iostream>
#include <iomanip> // For std::setw, std::setfill, std::left, std::hex, std::dec
#include <map>
#include <string>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost

namespace FWA::SCANNER
{

	// --- Utility Function Implementations ---

	void printDeviceInfo(const FireWireDevice &device)
	{
		std::cout << "===========================================" << std::endl;
		std::cout << "Device: " << device.name << std::endl;
		std::cout << "Vendor: " << device.vendor << std::endl;
		std::cout << "GUID: 0x" << std::hex << std::setw(16) << std::setfill('0') << device.guid << std::dec << std::endl;

		// Check if DICE registers were read (or attempted)
		if (device.diceChipType != FWA::DICE::DiceChipType::Unknown || !device.diceRegisters.empty())
		{
			std::cout << "DICE Device: Yes (or attempted)" << std::endl;

			// Print Chip Type
			std::cout << "DICE Chip Type: ";
			switch (device.diceChipType)
			{
			case FWA::DICE::DiceChipType::DiceII:
				std::cout << "DICE II";
				break;
			case FWA::DICE::DiceChipType::DiceMini:
				std::cout << "DICE Mini";
				break;
			case FWA::DICE::DiceChipType::DiceJr:
				std::cout << "DICE Jr";
				break;
			default:
				std::cout << "Unknown";
				break;
			}
			std::cout << std::endl;

			// Print Active Configuration (if determined)
			if (device.currentConfig != FWA::DICE::DiceConfig::Unknown)
			{
				std::cout << "Active Configuration: ";
				switch (device.currentConfig)
				{
				case FWA::DICE::DiceConfig::Low:
					std::cout << "Low (32k-48k)";
					break;
				case FWA::DICE::DiceConfig::Mid:
					std::cout << "Mid (88.2k-96k)";
					break;
				case FWA::DICE::DiceConfig::High:
					std::cout << "High (176.4k-192k)";
					break;
				default:
					std::cout << "Unknown";
					break; // Should not happen if check above passed
				}
				std::cout << std::endl;
			}
			else
			{
				std::cout << "Active Configuration: Unknown" << std::endl;
			}

			// Print Stream Counts
			std::cout << "TX Streams: " << device.txStreamCount << std::endl;
			std::cout << "RX Streams: " << device.rxStreamCount << std::endl;

			// Print DICE Registers if any were read
			if (!device.diceRegisters.empty())
			{
				std::cout << "DICE Registers Read:" << std::endl;
				std::cout << "-------------------------------------------" << std::endl;
				std::cout << std::left << std::setw(22) << "Address"
									<< "| " << std::setw(10) << "Value (LE)"
									<< "| ASCII" << std::endl;
				std::cout << "-------------------------------------------" << std::endl;

				// Iterate through the stored registers (which are stored with absolute address keys)
				for (const auto &regPair : device.diceRegisters)
				{
					uint64_t addr = regPair.first;
					uint32_t rawValue = regPair.second;											// Raw Big Endian value
					uint32_t hostValue = CFSwapInt32LittleToHost(rawValue); // Convert to Host (Little Endian for DICE)
					std::string ascii = FWA::SCANNER::interpretAsASCII(hostValue);

					std::cout << "0x" << std::hex << std::setw(16) << std::setfill('0') << addr << std::dec
										<< " | 0x" << std::hex << std::setw(8) << std::setfill('0') << hostValue << std::dec
										<< " | " << (ascii.empty() ? "-" : ascii) << std::endl;
				}
				std::cout << "-------------------------------------------" << std::endl;
			}
			else
			{
				std::cout << "DICE Registers Read: None" << std::endl;
			}
		}
		else
		{
			std::cout << "DICE Device: No (or registers inaccessible)" << std::endl;
		}
	}

} // namespace FWA::SCANNER