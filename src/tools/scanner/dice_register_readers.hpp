#ifndef DICE_REGISTER_READERS_HPP
#define DICE_REGISTER_READERS_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "dice_aes_registers.hpp"
#include "dice_avs_registers.hpp"
#include "dice_clock_registers.hpp"
#include "dice_global_registers.hpp"
#include "dice_gpcsr_registers.hpp"
#include "dice_mixer_registers.hpp"
#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Read all DICE registers
 *
 * Delegates to specialized register reader functions for different register
 * types. This is a convenience function that calls all the individual register
 * reader functions.
 *
 * @param deviceInterface Pointer to FireWire device interface
 * @param service FireWire service
 * @param device Device information to update
 * @param globalBase Base address for global registers
 * @param generation FireWire bus generation
 */
void readAllDiceRegisters(IOFireWireDeviceInterface **deviceInterface,
                          io_service_t service, FireWireDevice &device,
                          uint64_t globalBase, UInt32 generation);

} // namespace FWA::SCANNER

#endif // DICE_REGISTER_READERS_HPP
