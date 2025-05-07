#ifndef IO_HELPERS_HPP
#define IO_HELPERS_HPP

#include <cstdint>
#include <string>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct definition

namespace FWA::SCANNER {

// Gets basic device info (GUID, Name, Vendor) from IORegistry
FireWireDevice getDeviceInfo(io_service_t device);

// Safe wrapper for ReadQuadlet that uses setjmp/longjmp for segfault recovery
// Set forceQuadlet to true to always use ReadQuadlet regardless of address (for
// DICE registers)
IOReturn safeReadQuadlet(IOFireWireDeviceInterface **deviceInterface,
                         io_service_t service, uint64_t absoluteAddr,
                         UInt32 &value, UInt32 generation,
                         bool forceQuadlet = false);

// Basic ReadQuadlet wrapper (gets generation if needed)
// Set forceQuadlet to true to always use ReadQuadlet regardless of address (for
// DICE registers)
IOReturn readQuadlet(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, uint64_t absoluteAddr, UInt32 &value,
                     UInt32 generation, bool forceQuadlet = false);

// Safe wrapper for ReadBlock that uses setjmp/longjmp for segfault recovery
IOReturn safeReadBlock(
    IOFireWireDeviceInterface **deviceInterface, io_service_t service,
    uint64_t absoluteAddr, void *buffer,
    UInt32 &numBytes, // Input: requested size, Output: actual size read
    UInt32 generation);

// Helper function to interpret a 32-bit value as a potential ASCII string
std::string interpretAsASCII(UInt32 value);

} // namespace FWA::SCANNER

#endif // IO_HELPERS_HPP
