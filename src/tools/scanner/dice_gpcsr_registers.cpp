#include "dice_gpcsr_registers.hpp"
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
void readGpcsrRegisters(IOFireWireDeviceInterface **deviceInterface,
                        io_service_t service, FireWireDevice &device,
                        UInt64 /* discoveredDiceBase */, UInt32 generation) {
  std::cerr
      << "Debug [DICE]: Reading GPCSR registers using absolute addresses..."
      << std::endl;

  // Read GPCSR_CHIP_ID
  UInt64 chipIdAddr = GPCSR_CHIP_ID_ADDR;
  UInt32 chipIdValue = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, chipIdAddr, &chipIdValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[chipIdAddr] = chipIdValue;
    UInt32 hostValue = CFSwapInt32LittleToHost(chipIdValue);
    UInt32 chipType = (hostValue & GPCSR_CHIP_ID_CHIP_TYPE_MASK) >>
                      GPCSR_CHIP_ID_CHIP_TYPE_SHIFT;

    switch (chipType) {
    case static_cast<UInt32>(DiceChipType::DiceII):
      device.diceChipType = DiceChipType::DiceII;
      std::cerr << "Debug [DICE]: Detected Chip Type: DICE II" << std::endl;
      break;
    case static_cast<UInt32>(DiceChipType::DiceMini):
      device.diceChipType = DiceChipType::DiceMini;
      std::cerr << "Debug [DICE]: Detected Chip Type: DICE Mini" << std::endl;
      break;
    case static_cast<UInt32>(DiceChipType::DiceJr):
      device.diceChipType = DiceChipType::DiceJr;
      std::cerr << "Debug [DICE]: Detected Chip Type: DICE Jr" << std::endl;
      break;
    default:
      device.diceChipType = DiceChipType::Unknown;
      std::cerr << "Warning [DICE]: Unknown Chip Type: " << chipType
                << std::endl;
      break;
    }
  } else {
    std::cerr << "Warning [DICE]: Failed to read GPCSR_CHIP_ID (0x" << std::hex
              << chipIdAddr << ") (status: " << status << ")" << std::dec
              << std::endl;
  }

  // Read GPCSR_AUDIO_SELECT
  UInt64 audioSelectAddr = GPCSR_AUDIO_SELECT_ADDR;

  UInt32 audioSelectValue = 0;
  status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, audioSelectAddr, &audioSelectValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[audioSelectAddr] = audioSelectValue;
    UInt32 hostValue = CFSwapInt32LittleToHost(audioSelectValue);
    std::cerr << "Debug [DICE]: Read GPCSR_AUDIO_SELECT (0x" << std::hex
              << audioSelectAddr << "): 0x" << hostValue << std::dec
              << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read GPCSR_AUDIO_SELECT (0x"
              << std::hex << audioSelectAddr << ") (status: " << status << ")"
              << std::dec << std::endl;
  }
}

} // namespace FWA::SCANNER
