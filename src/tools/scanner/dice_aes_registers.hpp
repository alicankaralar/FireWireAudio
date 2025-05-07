#ifndef DICE_AES_REGISTERS_HPP
#define DICE_AES_REGISTERS_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Read AES Receiver registers
 *
 * Reads AES Receiver registers like STAT_ALL.
 * Updates device.aesLocked based on STAT_ALL.
 * Stores values in device.diceRegisters map.
 *
 * @param deviceInterface Pointer to FireWire device interface
 * @param service FireWire service
 * @param device Device information to update
 * @param globalBase Base address for global registers (not used directly)
 * @param generation FireWire bus generation
 */
void readAesReceiverRegisters(IOFireWireDeviceInterface **deviceInterface,
                              io_service_t service, FireWireDevice &device,
                              uint64_t globalBase, UInt32 generation);

} // namespace FWA::SCANNER

#endif // DICE_AES_REGISTERS_HPP
