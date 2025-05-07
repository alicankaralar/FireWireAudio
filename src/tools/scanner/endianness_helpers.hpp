#ifndef ENDIANNESS_HELPERS_HPP
#define ENDIANNESS_HELPERS_HPP

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>
#include <cstdint>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Represents the endianness of a device
 */
enum class DeviceEndianness {
  UNKNOWN_ENDIAN,      ///< Endianness could not be determined
  DEVICE_BIG_ENDIAN,   ///< Device is big-endian (MSB first)
  DEVICE_LITTLE_ENDIAN ///< Device is little-endian (LSB first)
};

/**
 * @brief Detects the endianness of a connected FireWire device
 *
 * This function attempts to determine the endianness of the device by examining
 * known multi-byte registers that have predictable values when interpreted with
 * the correct endianness. It checks fields like VENDOR_ID, CHIP_ID, or other
 * registers that would only make sense with one endian interpretation.
 *
 * @param deviceInterface Pointer to FireWire device interface
 * @param service FireWire service
 * @param device Reference to the FireWireDevice struct
 * @param generation FireWire bus generation
 * @return The detected endianness (BIG_ENDIAN, LITTLE_ENDIAN, or
 * UNKNOWN_ENDIAN)
 */
DeviceEndianness
detectDeviceEndianness(IOFireWireDeviceInterface **deviceInterface,
                       io_service_t service, FireWireDevice &device,
                       UInt32 generation);

/**
 * @brief Converts a 32-bit value from device endianness to host endianness
 *
 * @param value The value to convert
 * @param endianness The device's endianness
 * @return The value converted to host endianness
 */
uint32_t deviceToHostInt32(uint32_t value, DeviceEndianness endianness);

/**
 * @brief Converts a 32-bit value from host endianness to device endianness
 *
 * @param value The value to convert
 * @param endianness The device's endianness
 * @return The value converted to device endianness
 */
uint32_t hostToDeviceInt32(uint32_t value, DeviceEndianness endianness);

} // namespace FWA::SCANNER

#endif // ENDIANNESS_HELPERS_HPP
