#ifndef DICE_MIXER_REGISTERS_HPP
#define DICE_MIXER_REGISTERS_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Read Audio Mixer registers
 *
 * Reads Audio Mixer registers like NUMOFCH.
 * Updates device.mixerRxChannels based on NUMOFCH.
 * Stores values in device.diceRegisters map.
 *
 * @param deviceInterface Pointer to FireWire device interface
 * @param service FireWire service
 * @param device Device information to update
 * @param globalBase Base address for global registers (not used directly)
 * @param generation FireWire bus generation
 */
void readAudioMixerRegisters(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             UInt64 globalBase, UInt32 generation);

} // namespace FWA::SCANNER

#endif // DICE_MIXER_REGISTERS_HPP
