#include "dice_mixer_registers.hpp"
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
void readAudioMixerRegisters(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             uint64_t /* discoveredDiceBase */,
                             UInt32 generation) {
  std::cerr << "Debug [DICE]: Reading Audio Mixer registers using absolute "
               "addresses..."
            << std::endl;

  // Read MIXER_NUMOFCH
  uint64_t numOfChAddr = AUDIO_MIXER_NUMOFCH_ADDR; // Use absolute address from
                                                   // DiceAbsoluteAddresses.hpp
  UInt32 numOfChValue = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, numOfChAddr, numOfChValue, generation, true);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[numOfChAddr] = numOfChValue;
    uint32_t hostValue = CFSwapInt32LittleToHost(numOfChValue);
    device.mixerRxChannels = static_cast<uint8_t>(hostValue & 0xFF);
    std::cerr << "Debug [DICE]: Read AUDIO_MIXER_NUMOFCH (0x" << std::hex
              << numOfChAddr
              << "): RX Channels = " << static_cast<int>(device.mixerRxChannels)
              << std::dec << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read AUDIO_MIXER_NUMOFCH (0x"
              << std::hex << numOfChAddr << ") (status: " << status << ")"
              << std::dec << std::endl;
  }
}

} // namespace FWA::SCANNER
