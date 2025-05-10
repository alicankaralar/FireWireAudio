#include "dice_global_registers.hpp"
#include "FWA/dice/DiceAbsoluteAddresses.hpp" // For DICE_REGISTER_BASE, etc.
#include "endianness_helpers.hpp"             // For deviceToHostInt32
#include "io_helpers.hpp" // For safeReadQuadlet, interpretAsASCII
#include "scanner.hpp"    // For FireWireDevice, DiceDefines.hpp constants

#include <iomanip> // For std::hex, std::setw, std::setfill
#include <iostream>
#include <map>
#include <string>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER {
// Helper to read common global DICE registers with defined relative offsets
void readDiceGlobalRegisters(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             UInt64 diceBaseRegisterAddress,
                             UInt32 generation) {
  std::cerr << "\nInfo [DICE]: Beginning Global Register reads from discovered "
               "base 0x"
            << std::hex << diceBaseRegisterAddress << std::dec << std::endl;
  std::cerr << "Info [DICE]: These registers provide core DICE chip "
               "functionality and identification"
            << std::endl;

  // Define register information including offset, name, and purpose
  struct RegisterInfo {
    UInt64 address;
    std::string name;
    std::string purpose;
  };

  std::vector<RegisterInfo> registersToRead = {
      {GLOBAL_OWNER_ADDR, "Owner",
       "Identifies the DICE chip type and capabilities"},
      {GLOBAL_NOTIFICATION_ADDR, "Notification",
       "Provides device status and event notifications"},
      {GLOBAL_CLOCK_SELECT_ADDR, "Clock Select",
       "Controls the device's clock source selection"},
      {GLOBAL_SAMPLE_RATE_ADDR, "Sample Rate",
       "Current audio sample rate configuration"},
      {DICE_REGISTER_TX_PARAMETER_SPACE_OFFSET + DICE_REGISTER_BASE,
       "TX Parameter Space", "Offset to transmit parameter configuration area"},
      {DICE_REGISTER_RX_PARAMETER_SPACE_OFFSET + DICE_REGISTER_BASE,
       "RX Parameter Space", "Offset to receive parameter configuration area"}};

  for (const auto &reg : registersToRead) {
    UInt32 value = 0;

    std::cerr << "\nInfo [DICE]: Attempting to read " << reg.name << " register"
              << std::endl;
    std::cerr << "Info [DICE]: Purpose: " << reg.purpose << std::endl;
    std::cerr << "Info [DICE]: Address: Base 0x" << std::hex
              << diceBaseRegisterAddress << " - > 0x" << reg.address << std::dec
              << std::endl;

    IOReturn status = FWA::SCANNER::safeReadQuadlet(
        deviceInterface, service, reg.address, &value, generation);

    if (status == kIOReturnSuccess) {
      device.diceRegisters[reg.address] = value; // Store raw BE value
      UInt32 hostValue = CFSwapInt32LittleToHost(value);
      std::cerr << "Info [DICE]: Successfully read " << reg.name
                << " register: 0x" << std::hex << hostValue << std::dec;

      // Special handling for Owner register which identifies DICE chip type
      if (reg.address == GLOBAL_OWNER_ADDR) {
        std::cerr << " (";
        switch (hostValue) {
        case 0x0001:
          std::cerr << "DICE I";
          break;
        case 0x0002:
          std::cerr << "DICE II";
          break;
        case 0x0003:
          std::cerr << "DICE Mini";
          break;
        case 0x0004:
          std::cerr << "DICE Jr";
          break;
        default:
          std::cerr << "Unknown DICE variant";
        }
        std::cerr << ")";
      }
      std::cerr << std::endl;
    } else {
      std::cerr << "Error [DICE]: Failed to read " << reg.name
                << " register at 0x" << std::hex << reg.address
                << " (status: 0x" << status;
      if (status == 0xe0008017) {
        std::cerr << " - likely address error";
      }
      std::cerr << ")" << std::dec << std::endl;
    }
  }

  std::cerr << "\nInfo [DICE]: Completed Global Register read attempts"
            << std::endl;
}

} // namespace FWA::SCANNER
