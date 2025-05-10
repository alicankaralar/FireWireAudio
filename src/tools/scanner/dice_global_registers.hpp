#ifndef DICE_GLOBAL_REGISTERS_HPP
#define DICE_GLOBAL_REGISTERS_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Read DICE global registers
 *
 * Reads standard global registers like Owner, Notification, Clock Select,
 * Sample Rate, etc. Stores values in device.diceRegisters map.
 *
 * @param deviceInterface Pointer to FireWire device interface
 * @param service FireWire service
 * @param device Device information to update
 * @param globalBase Base address for global registers
 * @param generation FireWire bus generation
 */
void readDiceGlobalRegisters(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             UInt64 globalBase, UInt32 generation);

} // namespace FWA::SCANNER

#endif // DICE_GLOBAL_REGISTERS_HPP
