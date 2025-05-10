#ifndef MEMORY_EXPLORER_HPP
#define MEMORY_EXPLORER_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Performs a diagnostic scan around the specified base address.
 * Reads quadlets in a defined range and attempts to identify potential version
 * registers, coherent ASCII strings (like channel names), and structured
 * numeric data (like zero blocks). This is purely for debugging and
 * understanding memory layout.
 *
 * @param deviceInterface Pointer-to-pointer to an opened
 * IOFireWireDeviceInterface.
 * @param service The io_service_t representing the FireWire nub.
 * @param device Reference to the FireWireDevice struct (used for context, not
 * modified).
 * @param generation The current bus generation number.
 * @param baseAddr The base address to explore around.
 */
void exploreDiceMemoryLayout(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             UInt32 generation, UInt64 baseAddr);

/**
 * @brief Explores specific memory areas based on the DICE datasheet memory map.
 * This function calls exploreDiceMemoryLayout() for each key functional block.
 *
 * @param deviceInterface Pointer-to-pointer to an opened
 * IOFireWireDeviceInterface.
 * @param service The io_service_t representing the FireWire nub.
 * @param device Reference to the FireWireDevice struct (used for context, not
 * modified).
 * @param generation The current bus generation number.
 * @param discoveredDiceBase The discovered DICE base address, or
 * DICE_INVALID_OFFSET if not discovered.
 */
void exploreTargetedMemoryAreas(IOFireWireDeviceInterface **deviceInterface,
                                io_service_t service, FireWireDevice &device,
                                UInt32 generation, UInt64 discoveredDiceBase);

} // namespace FWA::SCANNER

#endif // MEMORY_EXPLORER_HPP
