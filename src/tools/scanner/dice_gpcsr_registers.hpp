#ifndef DICE_GPCSR_REGISTERS_HPP
#define DICE_GPCSR_REGISTERS_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Read GPCSR (General Purpose Control and Status Registers)
 *
 * Reads GPCSR registers like CHIP_ID and AUDIO_SELECT.
 * Updates device.diceChipType based on CHIP_ID.
 * Stores values in device.diceRegisters map.
 *
 * @param deviceInterface Pointer to FireWire device interface
 * @param service FireWire service
 * @param device Device information to update
 * @param globalBase Base address for global registers (not used directly)
 * @param generation FireWire bus generation
 */
void readGpcsrRegisters(IOFireWireDeviceInterface **deviceInterface,
                        io_service_t service, FireWireDevice &device,
                        UInt64 globalBase, UInt32 generation);

} // namespace FWA::SCANNER

#endif // DICE_GPCSR_REGISTERS_HPP
