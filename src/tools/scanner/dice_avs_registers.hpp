#ifndef DICE_AVS_REGISTERS_HPP
#define DICE_AVS_REGISTERS_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Read AVS (Audio Video System) registers
 *
 * Reads AVS registers like ARX0_CFG0, ARX0_CFG1, and ATX0_CFG.
 * Updates device.avsRxChannelId, device.avsRxDataBlockSize,
 * device.avsTxDataBlockSize, and device.avsTxSystemMode. Stores values in
 * device.diceRegisters map.
 *
 * @param deviceInterface Pointer to FireWire device interface
 * @param service FireWire service
 * @param device Device information to update
 * @param globalBase Base address for global registers (not used directly)
 * @param generation FireWire bus generation
 */
void readAvsRegisters(IOFireWireDeviceInterface **deviceInterface,
                      io_service_t service, FireWireDevice &device,
                      uint64_t globalBase, UInt32 generation);

} // namespace FWA::SCANNER

#endif // DICE_AVS_REGISTERS_HPP
