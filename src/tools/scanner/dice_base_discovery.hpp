#ifndef DICE_BASE_DISCOVERY_HPP
#define DICE_BASE_DISCOVERY_HPP

#include <cstdint>
#include <string>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Discover DICE base addresses (Global, TX, RX)
 *
 * Attempts to discover the DICE base addresses using multiple methods:
 * 1. Config ROM Vendor Keys
 * 2. FFADO pointer discovery base
 * 3. Legacy DICE_REGISTER_BASE fallback
 *
 * @param deviceInterface Pointer to FireWire device interface
 * @param service FireWire service
 * @param generation FireWire bus generation
 * @param globalBase Output parameter for discovered global base address
 * @param txBase Output parameter for discovered TX base address
 * @param rxBase Output parameter for discovered RX base address
 * @param method Output parameter for the method used to discover the base
 * addresses
 * @return true if base addresses were successfully discovered, false otherwise
 */
bool discoverDiceBaseAddresses(IOFireWireDeviceInterface **deviceInterface,
                               io_service_t service, UInt32 generation,
                               uint64_t &globalBase, uint64_t &txBase,
                               uint64_t &rxBase, std::string &method);

/**
 * @brief Test register access across different spaces
 *
 * Tests access to various register spaces including core registers,
 * subsystem registers, and EAP registers.
 *
 * @param deviceInterface Pointer to FireWire device interface
 * @param service FireWire service
 * @param generation FireWire bus generation
 * @return true if any registers were successfully accessed, false otherwise
 */
bool testRegisterSpaceAccess(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, UInt32 generation);

} // namespace FWA::SCANNER

#endif // DICE_BASE_DISCOVERY_HPP
