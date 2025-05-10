#include "dice_avs_registers.hpp"
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
void readAvsRegisters(IOFireWireDeviceInterface **deviceInterface,
                      io_service_t service, FireWireDevice &device,
                      UInt64 /* discoveredDiceBase */, UInt32 generation) {
  std::cerr << "Debug [DICE]: Reading AVS registers using absolute addresses..."
            << std::endl;

  // Read AVS Audio Receiver Config (for receiver 0)
  UInt64 arxCfg0Addr =
      AVS_AUDIO_RECEIVER_CFG0_ADDR; // Use absolute address from
                                    // DiceAbsoluteAddresses.hpp
  UInt32 arxCfg0Value = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, arxCfg0Addr, &arxCfg0Value, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[arxCfg0Addr] = arxCfg0Value;
    UInt32 hostValue = CFSwapInt32LittleToHost(arxCfg0Value);
    device.avsRxChannelId = static_cast<UInt8>(
        (hostValue &
         AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_MASK) >> // Use constexpr mask
        AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_SHIFT);   // Use constexpr shift
    std::cerr << "Debug [DICE]: Read ARX0_CFG0 (0x" << std::hex << arxCfg0Addr
              << "): Channel ID = " << static_cast<int>(device.avsRxChannelId)
              << std::dec << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read ARX0_CFG0 (0x" << std::hex
              << arxCfg0Addr << ") (status: " << status << ")" << std::dec
              << std::endl;
  }

  UInt64 arxCfg1Addr =
      AVS_AUDIO_RECEIVER_CFG1_ADDR; // Use absolute address from
                                    // DiceAbsoluteAddresses.hpp
  UInt32 arxCfg1Value = 0;
  status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, arxCfg1Addr,
                                         &arxCfg1Value, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[arxCfg1Addr] = arxCfg1Value;
    UInt32 hostValue = CFSwapInt32LittleToHost(arxCfg1Value);
    device.avsRxDataBlockSize = static_cast<UInt8>(
        (hostValue &
         AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_MASK) >> // Use constexpr mask
        AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_SHIFT);   // Use constexpr shift
    std::cerr << "Debug [DICE]: Read ARX0_CFG1 (0x" << std::hex << arxCfg1Addr
              << "): Specified DBS = "
              << static_cast<int>(device.avsRxDataBlockSize) << std::dec
              << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read ARX0_CFG1 (0x" << std::hex
              << arxCfg1Addr << ") (status: " << status << ")" << std::dec
              << std::endl;
  }

  // Read AVS Audio Transmitter Config (for transmitter 0)
  UInt64 atxCfgAddr =
      AVS_AUDIO_TRANSMITTER_CFG_ADDR; // Use absolute address from
                                      // DiceAbsoluteAddresses.hpp
  UInt32 atxCfgValue = 0;
  status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, atxCfgAddr,
                                         &atxCfgValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[atxCfgAddr] = atxCfgValue;
    UInt32 hostValue = CFSwapInt32LittleToHost(atxCfgValue);
    device.avsTxDataBlockSize = static_cast<UInt8>(
        (hostValue &
         AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_MASK) >> // Use constexpr
                                                            // mask
        AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_SHIFT); // Use constexpr shift
    UInt32 sysMode =
        (hostValue &
         AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_MASK) >> // Use constexpr mask
        AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_SHIFT;    // Use constexpr shift

    switch (sysMode) {
    case 0:
      device.avsTxSystemMode = AvsSystemMode::Low;
      break;
    case 1:
      device.avsTxSystemMode = AvsSystemMode::Mid;
      break;
    case 2:
      device.avsTxSystemMode = AvsSystemMode::High;
      break;
    default:
      device.avsTxSystemMode = AvsSystemMode::Unknown;
      break;
    }
    std::cerr << "Debug [DICE]: Read ATX0_CFG (0x" << std::hex << atxCfgAddr
              << "): Data Block Size = "
              << static_cast<int>(device.avsTxDataBlockSize)
              << ", System Mode = " << static_cast<int>(device.avsTxSystemMode)
              << std::dec << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read AVS_AUDIO_TRANSMITTER_CFG (0x"
              << std::hex << atxCfgAddr << ") (status: " << status << ")"
              << std::dec << std::endl;
  }
}

} // namespace FWA::SCANNER
