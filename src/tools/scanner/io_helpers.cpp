#include "io_helpers.hpp"
#include "scanner.hpp" // Include scanner.hpp for g_jmpBuf, g_segfaultOccurred

#include <atomic>
#include <iomanip> // For std::hex
#include <iostream>
#include <setjmp.h> // For setjmp/longjmp

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER {

// --- IOKit Helper Function Implementations ---

FireWireDevice getDeviceInfo(io_service_t device) {
  std::cerr << "Debug [IO]: Entering getDeviceInfo for service: " << device
            << std::endl;
  FireWireDevice info;
  info.guid = 0; // Initialize
  info.name = "[Error]";
  info.vendor = "[Error]";
  info.diceChipType = FWA::DICE::DiceChipType::Unknown; // Ensure initialized

  CFMutableDictionaryRef properties = nullptr;
  IOReturn result = IORegistryEntryCreateCFProperties(
      device, &properties, kCFAllocatorDefault, kNilOptions);

  if (result == kIOReturnSuccess && properties != nullptr) {
    // Get GUID
    CFTypeRef guidValue = CFDictionaryGetValue(properties, CFSTR("GUID"));
    if (guidValue != nullptr && CFGetTypeID(guidValue) == CFNumberGetTypeID()) {
      CFNumberRef guidNumber = (CFNumberRef)guidValue;
      if (!CFNumberGetValue(guidNumber, kCFNumberSInt64Type, &info.guid)) {
        std::cerr << "Warning [IO]: Failed to get SInt64 value for GUID."
                  << std::endl;
        info.guid = 0; // Reset on failure
      }
    } else {
      std::cerr << "Warning [IO]: GUID property not found or not a CFNumber."
                << std::endl;
      info.guid = 0; // Ensure it's 0 if not found
    }

    // Get Name
    CFTypeRef nameValue =
        CFDictionaryGetValue(properties, CFSTR("FireWire Product Name"));
    if (nameValue != nullptr && CFGetTypeID(nameValue) == CFStringGetTypeID()) {
      CFStringRef nameRef = (CFStringRef)nameValue;
      char nameBuffer[256] = {0};
      if (CFStringGetCString(nameRef, nameBuffer, sizeof(nameBuffer),
                             kCFStringEncodingUTF8)) {
        info.name = nameBuffer;
      } else {
        std::cerr << "Warning [IO]: Failed to get C string for FireWire "
                     "Product Name (GUID: 0x"
                  << std::hex << info.guid << std::dec << ")." << std::endl;
        info.name = "[Unknown Name]";
      }
    } else {
      info.name = "[Name Not Found]";
    }

    // Get Vendor
    CFTypeRef vendorValue =
        CFDictionaryGetValue(properties, CFSTR("FireWire Vendor Name"));
    if (vendorValue != nullptr &&
        CFGetTypeID(vendorValue) == CFStringGetTypeID()) {
      CFStringRef vendorRef = (CFStringRef)vendorValue;
      char vendorBuffer[256] = {0};
      if (CFStringGetCString(vendorRef, vendorBuffer, sizeof(vendorBuffer),
                             kCFStringEncodingUTF8)) {
        info.vendor = vendorBuffer;
      } else {
        std::cerr << "Warning [IO]: Failed to get C string for FireWire Vendor "
                     "Name (GUID: 0x"
                  << std::hex << info.guid << std::dec << ")." << std::endl;
        info.vendor = "[Unknown Vendor]";
      }
    } else {
      info.vendor = "[Vendor Not Found]";
    }

    CFRelease(properties);
  } else {
    std::cerr
        << "Warning [IO]: Failed to get device properties for service object: "
        << device << " (IOReturn: " << result << ")" << std::endl;
    // Keep initialized error values for name/vendor/guid
  }

  return info;
}

IOReturn safeReadQuadlet(IOFireWireDeviceInterface **deviceInterface,
                         io_service_t service, uint64_t absoluteAddr,
                         UInt32 &value, UInt32 generation) {
  // Reset the segfault flag before attempting the read
  g_segfaultOccurred = false;

  // Save execution context for longjmp recovery
  if (setjmp(g_jmpBuf) == 0) {
    // First time through - perform the read operation
    // Guard against invalid interface pointer
    if (!deviceInterface || !*deviceInterface) {
      std::cerr << "Error [safeReadQuadlet]: Invalid device interface.\n";
      return kIOReturnBadArgument;
    }

    // Build the 64-bit FireWire address structure
    FWAddress addr;
    addr.addressHi = static_cast<UInt16>((absoluteAddr >> 32) & 0xFFFF);
    addr.addressLo = static_cast<UInt32>(absoluteAddr & 0xFFFFFFFF);

    // Perform the quadlet read using the correct service handle
    // Call the existing readQuadlet function which handles high/low addresses
    // internally
    IOReturn err = FWA::SCANNER::readQuadlet(deviceInterface, service,
                                             absoluteAddr, value, generation);
    return err;
  } else {
    // We get here if a segfault occurred and longjmp was called
    std::cerr << "Recovered from segfault during read at address 0x" << std::hex
              << absoluteAddr << std::dec << std::endl;
    return kIOReturnError; // Return a generic error code
  }
}

IOReturn readQuadlet(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, uint64_t absoluteAddr, UInt32 &value,
                     UInt32 generation) {
  // Guard against invalid interface pointer
  if (!deviceInterface || !*deviceInterface) {
    std::cerr << "Error [readQuadlet]: Invalid device interface.\n";
    return kIOReturnBadArgument;
  }

  // If caller didn't provide generation, fetch it
  if (generation == 0) {
    IOReturn genErr =
        (*deviceInterface)->GetBusGeneration(deviceInterface, &generation);
    if (genErr != kIOReturnSuccess) {
      std::cerr << "Error [readQuadlet]: Unable to get bus generation ("
                << std::hex << genErr << ").\n";
      return genErr;
    }
  }

  // Build the 64-bit FireWire address structure
  FWAddress addr;
  addr.addressHi = static_cast<UInt16>((absoluteAddr >> 32) & 0xFFFF);
  addr.addressLo = static_cast<UInt32>(absoluteAddr & 0xFFFFFFFF);

  IOReturn err = kIOReturnSuccess;
  const uint64_t HIGH_ADDRESS_THRESHOLD = 0xFFFFF0000000ULL;

  if (absoluteAddr >= HIGH_ADDRESS_THRESHOLD) {
    // Use Read for high addresses
    UInt32 numBytesExpected = sizeof(UInt32); // Expected bytes for a quadlet
    UInt32 numBytesToRead = numBytesExpected; // Request this many bytes, will
                                              // be updated with actual read
    // Use the 'Read' method from IOFireWireDeviceInterface
    // Hypothesis: Signature is Read(..., service, addr*, buffer*, numBytes*,
    // failOnReset, generation)
    err = (*deviceInterface)
              ->Read(deviceInterface, service, &addr, &value, &numBytesToRead,
                     kFWFailOnReset, generation);
    if (err != kIOReturnSuccess) {
      std::cerr << "Error [readQuadlet/Read]: Read failed at 0x" << std::hex
                << absoluteAddr << " (status: " << err << ").\n"
                << std::dec;
    }
    // Check if the actual bytes read (now in numBytesToRead) match requested
    else if (numBytesToRead != numBytesExpected) {
      // This shouldn't happen for a successful quadlet-sized read, but check
      // anyway
      std::cerr << "Warning [readQuadlet/Read]: Read at 0x" << std::hex
                << absoluteAddr << " returned success but read " << std::dec
                << numBytesToRead << " bytes instead of expected "
                << numBytesExpected << ".\n";
      // Consider returning an error or handling partial read if necessary
      err =
          kIOReturnUnderrun; // Treat partial read as an error for quadlet read
    }
    // If successful, 'value' already contains the data read. Endian swap might
    // be needed later.
  } else {
    // Use ReadQuadlet for lower addresses
    err = (*deviceInterface)
              ->ReadQuadlet(deviceInterface, service, &addr, &value,
                            kFWFailOnReset, generation);
    if (err != kIOReturnSuccess) {
      std::cerr << "Error [readQuadlet/ReadQuadlet]: Read failed at 0x"
                << std::hex << absoluteAddr << " (status: " << err << ").\n"
                << std::dec;
    }
  }

  return err;
}

std::string interpretAsASCII(UInt32 value) {
  std::string result;
  // Check if any of the bytes are ASCII printable characters
  // Process in Big Endian order (as read from device)
  for (int i = 0; i < 4; i++) {
    char c = static_cast<char>((value >> (8 * (3 - i))) & 0xFF);
    if (c >= 32 && c <= 126) { // Printable ASCII range
      result += c;
    } else {
      // If one character is non-printable, maybe the whole thing isn't ASCII
      // Or handle differently? For now, just append printable ones.
    }
  }
  // Return only if the entire string seems printable? Or just return what we
  // found? Let's return what we found, but maybe filter later if needed.
  return result;
}

} // namespace FWA::SCANNER
