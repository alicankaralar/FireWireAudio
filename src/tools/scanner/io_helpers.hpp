#ifndef IO_HELPERS_HPP
#define IO_HELPERS_HPP

#include <string>
#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct definition

namespace FWA::SCANNER
{

	// Gets basic device info (GUID, Name, Vendor) from IORegistry
	FireWireDevice getDeviceInfo(io_service_t device);

	// Safe wrapper for ReadQuadlet that uses setjmp/longjmp for segfault recovery
	IOReturn safeReadQuadlet(IOFireWireDeviceInterface **deviceInterface,
													 io_service_t service,
													 uint64_t absoluteAddr,
													 UInt32 &value,
													 UInt32 generation);

	// Basic ReadQuadlet wrapper (gets generation if needed)
	IOReturn readQuadlet(IOFireWireDeviceInterface **deviceInterface,
											 io_service_t service,
											 uint64_t absoluteAddr,
											 UInt32 &value,
											 UInt32 generation);

	// Helper function to interpret a 32-bit value as a potential ASCII string
	std::string interpretAsASCII(UInt32 value);

} // namespace FWA::SCANNER

#endif // IO_HELPERS_HPP