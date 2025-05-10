#include "register_name_analyzer.hpp"
#include "io_helpers.hpp" // For interpretAsASCII
#include "scanner.hpp"    // For FireWireDevice, DiceDefines.hpp constants

#include <iomanip> // For std::setw, std::setfill, std::left, std::hex, std::dec
#include <iostream>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost

namespace FWA::SCANNER {

void analyzeChannelAndDeviceNames(const FireWireDevice &device) {
  std::cout << "=== CHANNEL AND DEVICE NAMES ===\n" << std::endl;

  // Skip if no registers were read
  if (device.diceRegisters.empty()) {
    std::cout << "No DICE registers available for analysis." << std::endl;
    return;
  }

  // Look for TX/RX name base registers
  for (const auto &regPair : device.diceRegisters) {
    UInt64 addr = regPair.first;
    UInt32 rawValue = regPair.second;
    UInt32 hostValue = CFSwapInt32LittleToHost(rawValue);

    // Check if this is a name base register
    bool isTxNameBase = false;
    bool isRxNameBase = false;
    bool isNickNameBase = false;

    // Extract the register offset from the absolute address
    UInt64 baseAddr = DICE_REGISTER_BASE; // Use discovered base if available
    int64_t offset = addr - baseAddr;

    // Check against known offsets
    if (offset == DICE_REGISTER_GLOBAL_NICK_NAME_OFFSET) {
      isNickNameBase = true;
    } else if ((offset - TX_NAMES_BASE_ADDR) % (256 * 4) == 0) {
      isTxNameBase = true;
    } else if ((offset - RX_NAMES_BASE_ADDR) % (256 * 4) == 0) {
      isRxNameBase = true;
    }

    if (isTxNameBase || isRxNameBase || isNickNameBase) {
      std::string nameType =
          isNickNameBase
              ? "Device Nick Name"
              : (isTxNameBase ? "TX Channel Name" : "RX Channel Name");

      // The value is likely a pointer to the actual name data
      UInt64 nameDataAddr =
          baseAddr + (hostValue * 4); // Convert quadlet offset to byte address

      std::cout << nameType << " Base at 0x" << std::hex << addr
                << " points to 0x" << nameDataAddr << std::dec << std::endl;

      // Try to read the name data if it's in our register map
      std::string nameData;
      bool foundData = false;

      // Look for registers in the vicinity of the name data address
      for (const auto &reg : device.diceRegisters) {
        if (reg.first >= nameDataAddr && reg.first < nameDataAddr + 64) {
          UInt32 nameValue = CFSwapInt32LittleToHost(reg.second);
          std::string ascii = interpretAsASCII(nameValue);
          if (!ascii.empty()) {
            nameData += ascii;
            foundData = true;
          }
        }
      }

      if (foundData) {
        std::cout << "  Name data: \"" << nameData << "\"" << std::endl;
      } else {
        std::cout << "  Name data not found in register map" << std::endl;
      }
    }
  }
}

void analyzeClockSourceNames(const FireWireDevice &device) {
  std::cout << "\n=== CLOCK SOURCE NAMES ===\n" << std::endl;

  // Skip if no registers were read
  if (device.diceRegisters.empty()) {
    std::cout << "No DICE registers available for analysis." << std::endl;
    return;
  }

  UInt64 clockSourceNamesAddr = 0;

  // Find the clock source names base register
  for (const auto &regPair : device.diceRegisters) {
    UInt64 addr = regPair.first;
    UInt64 baseAddr = DICE_REGISTER_BASE;
    int64_t offset = addr - baseAddr;

    if (offset == DICE_REGISTER_GLOBAL_CLOCKSOURCENAMES_OFFSET) {
      UInt32 hostValue = CFSwapInt32LittleToHost(regPair.second);
      clockSourceNamesAddr = baseAddr + (hostValue * 4);
      std::cout << "Clock Source Names Base at 0x" << std::hex << addr
                << " points to 0x" << clockSourceNamesAddr << std::dec
                << std::endl;
      break;
    }
  }

  if (clockSourceNamesAddr != 0) {
    // Try to extract clock source names
    for (int i = 0; i < 16; i++) { // Assume up to 16 clock sources
      std::string clockName;
      bool foundClockName = false;
      UInt64 clockNameAddr =
          clockSourceNamesAddr + (i * 16); // Typically 16 bytes per name

      for (int j = 0; j < 4; j++) { // Up to 4 quadlets per name
        UInt64 addrToCheck = clockNameAddr + (j * 4);
        for (const auto &reg : device.diceRegisters) {
          if (reg.first == addrToCheck) {
            UInt32 nameValue = CFSwapInt32LittleToHost(reg.second);
            std::string ascii = interpretAsASCII(nameValue);
            if (!ascii.empty()) {
              clockName += ascii;
              foundClockName = true;
            }
            break;
          }
        }
      }

      if (foundClockName) {
        std::cout << "  Clock Source " << i << " at 0x" << std::hex
                  << clockNameAddr << std::dec << ": \"" << clockName << "\""
                  << std::endl;
      }
    }
  } else {
    std::cout << "  Clock Source Names Base register not found" << std::endl;
  }
}

} // namespace FWA::SCANNER
