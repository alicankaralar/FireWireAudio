#ifndef IO_HELPERS_HPP
#define IO_HELPERS_HPP

#include <cstdint>
#include <string>
#include <vector>

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
                         io_service_t service, UInt64 absoluteAddr,
                         UInt32 *value, UInt32 generation);

// Basic ReadQuadlet wrapper (gets generation if needed)
// Set forceQuadlet to true to always use ReadQuadlet regardless of address (for
// DICE registers)
IOReturn readQuadlet(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, UInt64 absoluteAddr, UInt32 *value,
                     UInt32 generation);

// Safe wrapper for ReadBlock that uses setjmp/longjmp for segfault recovery
IOReturn safeReadBlock(IOFireWireDeviceInterface **deviceInterface,
                       io_service_t service, UInt64 absoluteAddr, UInt32 *value,
                       UInt32 *size, UInt32 generation);

// Basic ReadBlock wrapper (gets generation if needed)
IOReturn readBlock(IOFireWireDeviceInterface **deviceInterface,
                   io_service_t service, UInt64 absoluteAddr, UInt32 *value,
                   UInt32 *size, UInt32 generation);

// Helper function to interpret a 32-bit value as a potential ASCII string
std::string interpretAsASCII(UInt32 value);

IOReturn writeQuadlet(IOFireWireDeviceInterface **deviceInterface,
                      io_service_t service, UInt64 absoluteAddr, UInt32 value,
                      UInt32 generation = 0);

IOReturn safeWriteQuadlet(IOFireWireDeviceInterface **deviceInterface,
                          io_service_t service, UInt64 absoluteAddr,
                          UInt32 value, UInt32 generation = 0);

class stringlist : public std::vector<std::string> {
public:
  static stringlist splitString(std::string in, std::string delimiter) {
    stringlist result;
    size_type start = 0, end = 0;
    while (start < in.size()) {
      end = std::min(in.size(), in.find(delimiter, start));
      result.push_back(in.substr(start, end - start));
      start = end + delimiter.size();
    }
    return result;
  }

  std::string join(std::string joiner = "") {
    std::string result;
    for (stringlist::iterator it = begin(); it != end();) {
      result += *it;
      it++;
      if (it != end()) {
        result += joiner;
      }
    }
    return result;
  }
};

} // namespace FWA::SCANNER

#endif // IO_HELPERS_HPP
