#ifndef DICE_CONFIG_HPP
#define DICE_CONFIG_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Sets default TX/RX stream counts based on the detected chip type.
 *
 * Used as a fallback if EAP or standard register reads fail.
 * Updates device.txStreamCount and device.rxStreamCount based on
 * device.diceChipType.
 *
 * @param device Reference to the FireWireDevice struct to update.
 */
void setDefaultDiceConfig(FireWireDevice &device);

/**
 * @brief Main function to read all DICE registers
 *
 * This function orchestrates the reading of all DICE registers by:
 * 1. Discovering DICE base addresses (Global, TX, RX)
 * 2. Reading EAP capabilities and configuration
 * 3. Reading various register groups (Global, GPCSR, Clock Controller, etc.)
 * 4. Reading TX/RX stream registers
 * 5. Exploring memory layout and channel names
 *
 * @param deviceInterface Pointer to FireWire device interface
 * @param service FireWire service
 * @param device Device information to update
 */
void readDiceRegisters(IOFireWireDeviceInterface **deviceInterface,
                       io_service_t service, FireWireDevice &device);

} // namespace FWA::SCANNER

#endif // DICE_CONFIG_HPP
