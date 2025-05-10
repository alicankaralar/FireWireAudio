#include "dice_clock_registers.hpp"
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
void readClockControllerRegisters(IOFireWireDeviceInterface **deviceInterface,
                                  io_service_t service, FireWireDevice &device,
                                  UInt64 /* discoveredDiceBase */,
                                  UInt32 generation) {
  std::cerr << "Debug [DICE]: Reading Clock Controller registers using "
               "absolute addresses..."
            << std::endl;

  // Read SYNC_CTRL
  UInt64 syncCtrlAddr =
      CLOCK_CONTROLLER_SYNC_CTRL_ADDR; // Use absolute address from
                                       // DiceAbsoluteAddresses.hpp
  UInt32 syncCtrlValue = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, syncCtrlAddr, &syncCtrlValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[syncCtrlAddr] = syncCtrlValue;
    UInt32 hostValue = CFSwapInt32LittleToHost(syncCtrlValue);
    UInt32 syncSrc =
        (hostValue &
         CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_MASK) >> // Use constexpr mask
        CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_SHIFT;    // Use constexpr shift

    switch (syncSrc) {
    case 0:
      device.syncSource = FWA::DICE::ClockSourceEnum::AES0;
      break;
    case 1:
      device.syncSource = FWA::DICE::ClockSourceEnum::AES1;
      break;
    case 2:
      device.syncSource = FWA::DICE::ClockSourceEnum::AES2;
      break;
    case 3:
      device.syncSource = FWA::DICE::ClockSourceEnum::AES3;
      break;
    case 4:
      device.syncSource = FWA::DICE::ClockSourceEnum::SlaveInputs;
      break;
    case 5:
      device.syncSource = FWA::DICE::ClockSourceEnum::HPLL;
      break;
    case 6:
      device.syncSource = FWA::DICE::ClockSourceEnum::Internal;
      break;
    default:
      device.syncSource = FWA::DICE::ClockSourceEnum::Unknown;
      break;
    }
    std::cerr << "Debug [DICE]: Read SYNC_CTRL (0x" << std::hex << syncCtrlAddr
              << "): Sync Source = " << static_cast<int>(device.syncSource)
              << std::dec << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read CLOCK_CONTROLLER_SYNC_CTRL (0x"
              << std::hex << syncCtrlAddr << ") (status: " << status << ")"
              << std::dec << std::endl;
  }

  // Read DOMAIN_CTRL
  UInt64 domainCtrlAddr =
      CLOCK_CONTROLLER_DOMAIN_CTRL_ADDR; // Use absolute address from
                                         // DiceAbsoluteAddresses.hpp
  UInt32 domainCtrlValue = 0;
  status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, domainCtrlAddr, &domainCtrlValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[domainCtrlAddr] = domainCtrlValue;
    UInt32 hostValue = CFSwapInt32LittleToHost(domainCtrlValue);
    UInt32 rtrFs =
        (hostValue &
         CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_MASK) >> // Use constexpr mask
        CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_SHIFT;    // Use constexpr shift

    switch (rtrFs) {
    case 0:
      device.routerFsMode = RouterFrameSyncMode::BaseRate;
      break;
    case 1:
      device.routerFsMode = RouterFrameSyncMode::DoubleRate;
      break;
    case 2:
      device.routerFsMode = RouterFrameSyncMode::QuadRate;
      break;
    default:
      device.routerFsMode = RouterFrameSyncMode::Unknown;
      break;
    }
    std::cerr << "Debug [DICE]: Read DOMAIN_CTRL (0x" << std::hex
              << domainCtrlAddr
              << "): Router FS Mode = " << static_cast<int>(device.routerFsMode)
              << std::dec << std::endl;
  } else {
    std::cerr
        << "Warning [DICE]: Failed to read CLOCK_CONTROLLER_DOMAIN_CTRL (0x"
        << std::hex << domainCtrlAddr << ") (status: " << status << ")"
        << std::dec << std::endl;
  }
}

} // namespace FWA::SCANNER
