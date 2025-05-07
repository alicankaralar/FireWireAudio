#include "dice_aes_registers.hpp"
#include "FWA/DiceAbsoluteAddresses.hpp" // For DICE_REGISTER_BASE, etc.
#include "endianness_helpers.hpp"        // For deviceToHostInt32
#include "io_helpers.hpp" // For safeReadQuadlet, interpretAsASCII
#include "scanner.hpp"    // For FireWireDevice, DiceDefines.hpp constants

#include <iomanip> // For std::hex, std::setw, std::setfill
#include <iostream>
#include <map>
#include <string>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER {
void readAesReceiverRegisters(IOFireWireDeviceInterface **deviceInterface,
                              io_service_t service, FireWireDevice &device,
                              uint64_t /* discoveredDiceBase */,
                              UInt32 generation) {
  std::cerr << "Debug [DICE]: Reading AES Receiver registers using absolute "
               "addresses..."
            << std::endl;

  // Read STAT_ALL
  uint64_t statAllAddr =
      AES_RECEIVER_STAT_ALL_ADDR; // Use absolute address from
                                  // DiceAbsoluteAddresses.hpp
  UInt32 statAllValue = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, statAllAddr, statAllValue, generation, true);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[statAllAddr] = statAllValue;
    uint32_t hostValue = CFSwapInt32LittleToHost(statAllValue);
    device.aesLocked =
        (hostValue & AES_RECEIVER_STAT_ALL_LOCK_BIT) != 0; // Use constexpr mask
    std::cerr << "Debug [DICE]: Read AES_RECEIVER_STAT_ALL (0x" << std::hex
              << statAllAddr
              << "): Locked = " << (device.aesLocked ? "true" : "false")
              << std::dec << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read AES_RECEIVER_STAT_ALL (0x"
              << std::hex << statAllAddr << ") (status: " << status << ")"
              << std::dec << std::endl;
  }
}

} // namespace FWA::SCANNER
