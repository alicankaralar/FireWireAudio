#include "io_helpers.hpp"
#include "scanner.hpp"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>
#include <atomic>
#include <iomanip>
#include <iostream>
#include <setjmp.h>

namespace FWA::SCANNER {
// Helper function to identify DICE register space
static std::string identifyDiceSpace(UInt64 absoluteAddr) {
  // Remove the base address to get relative offset
  UInt64 relativeAddr = absoluteAddr & 0x0FFFFFFFULL;

  // Check subsystem spaces
  if (relativeAddr >= 0xC7000000 && relativeAddr < 0xC8000000)
    return "GPCSR (offset 0x" + std::to_string(relativeAddr - 0xC7000000) + ")";
  if (relativeAddr >= 0xCE000000 && relativeAddr < 0xCF000000)
    return "DICE Subsystem (offset 0x" +
           std::to_string(relativeAddr - 0xCE000000) + ")";
  if (relativeAddr >= 0xCF000000)
    return "AVS Subsystem (offset 0x" +
           std::to_string(relativeAddr - 0xCF000000) + ")";

  // Check standard spaces
  if (relativeAddr < 0x400)
    return "Global (offset 0x" + std::to_string(relativeAddr) + ")";
  if (relativeAddr >= 0x400 && relativeAddr < 0x800)
    return "TX (offset 0x" + std::to_string(relativeAddr - 0x400) + ")";
  if (relativeAddr >= 0x800 && relativeAddr < 0xC00)
    return "RX (offset 0x" + std::to_string(relativeAddr - 0x800) + ")";

  return "Unknown space";
}

// --- IOKit Helper Function Implementations ---

FireWireDevice getDeviceInfo(io_service_t device) {
  FireWireDevice info;
  info.guid = 0;
  info.name = "[Error]";
  info.vendor = "[Error]";
  info.diceChipType = DiceChipType::Unknown;

  CFMutableDictionaryRef properties = nullptr;
  IOReturn result = IORegistryEntryCreateCFProperties(
      device, &properties, kCFAllocatorDefault, kNilOptions);

  if (result == kIOReturnSuccess && properties != nullptr) {
    CFTypeRef guidValue = CFDictionaryGetValue(properties, CFSTR("GUID"));
    if (guidValue != nullptr && CFGetTypeID(guidValue) == CFNumberGetTypeID()) {
      CFNumberRef guidNumber = (CFNumberRef)guidValue;
      if (!CFNumberGetValue(guidNumber, kCFNumberSInt64Type, &info.guid)) {
        info.guid = 0;
      }
    }

    CFTypeRef nameValue =
        CFDictionaryGetValue(properties, CFSTR("FireWire Product Name"));
    if (nameValue != nullptr && CFGetTypeID(nameValue) == CFStringGetTypeID()) {
      CFStringRef nameRef = (CFStringRef)nameValue;
      char nameBuffer[256] = {0};
      if (CFStringGetCString(nameRef, nameBuffer, sizeof(nameBuffer),
                             kCFStringEncodingUTF8)) {
        info.name = nameBuffer;
      } else {
        info.name = "[Unknown Name]";
      }
    } else {
      info.name = "[Name Not Found]";
    }

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
        info.vendor = "[Unknown Vendor]";
      }
    } else {
      info.vendor = "[Vendor Not Found]";
    }

    CFRelease(properties);
  }

  return info;
}

IOReturn safeReadQuadlet(IOFireWireDeviceInterface **deviceInterface,
                         io_service_t service, UInt64 absoluteAddr,
                         UInt32 *value, UInt32 generation) {
  g_segfaultOccurred = false;

  if (setjmp(g_jmpBuf) == 0) {
    if (!deviceInterface || !*deviceInterface) {
      return kIOReturnBadArgument;
    }
    IOReturn err = false;
    err =
        readQuadlet(deviceInterface, service, absoluteAddr, value, generation);
    if (err != kIOReturnSuccess) {
      return err;
    }
    return err;
  }
  return kIOReturnError;
}

IOReturn safeReadBlock(IOFireWireDeviceInterface **deviceInterface,
                       io_service_t service, UInt64 absoluteAddr, UInt32 *value,
                       UInt32 *size, UInt32 generation) {
  if (setjmp(g_jmpBuf) == 0) {
    if (!deviceInterface || !*deviceInterface) {
      return kIOReturnBadArgument;
    }
    IOReturn err = false;
    readBlock(deviceInterface, service, absoluteAddr, value, size, generation);
    if (err != kIOReturnSuccess) {
      return err;
    }
    return err;
  }
  return kIOReturnError;
}

IOReturn readQuadlet(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, UInt64 absoluteAddr, UInt32 *value,
                     UInt32 generation) {
  if (generation == 0) {
    IOReturn genErr =
        (*deviceInterface)->GetBusGeneration(deviceInterface, &generation);
    if (genErr != kIOReturnSuccess) {
      return genErr;
    }
  }

  // Build the 64-bit FireWire address structure
  FWAddress addr;
  addr.addressHi = static_cast<UInt16>((absoluteAddr >> 32) & 0xFFFF);
  addr.addressLo = static_cast<UInt32>(absoluteAddr & 0xFFFFFFFF);

  uint32_t tempValue;
  IOReturn err = (*deviceInterface)
                     ->ReadQuadlet(deviceInterface, service, &addr, &tempValue,
                                   kFWFailOnReset, generation);

  if (err == kIOReturnSuccess) {

    *value = static_cast<UInt32>(CFSwapInt32BigToHost(tempValue));
  }
  return err;
};

IOReturn writeQuadlet(IOFireWireDeviceInterface **deviceInterface,
                      io_service_t service, UInt64 absoluteAddr, UInt32 value,
                      UInt32 generation) {
  if (generation == 0) {
    IOReturn genErr =
        (*deviceInterface)->GetBusGeneration(deviceInterface, &generation);
    if (genErr != kIOReturnSuccess) {
      return genErr;
    }
  }

  // Build the 64-bit FireWire address structure
  FWAddress addr;
  addr.addressHi = static_cast<UInt16>((absoluteAddr >> 32) & 0xFFFF);
  addr.addressLo = static_cast<UInt32>(absoluteAddr & 0xFFFFFFFF);

  uint32_t tempValue = CFSwapInt32HostToBig(value);
  IOReturn err = (*deviceInterface)
                     ->WriteQuadlet(deviceInterface, service, &addr, tempValue,
                                    kFWFailOnReset, generation);

  return err;
};

IOReturn safeWriteQuadlet(IOFireWireDeviceInterface **deviceInterface,
                          io_service_t service, UInt64 absoluteAddr,
                          UInt32 value, UInt32 generation) {
  g_segfaultOccurred = false;

  if (setjmp(g_jmpBuf) == 0) {
    if (!deviceInterface || !*deviceInterface) {
      return kIOReturnBadArgument;
    }
    IOReturn err = false;
    err =
        writeQuadlet(deviceInterface, service, absoluteAddr, value, generation);
    if (err != kIOReturnSuccess) {
      return err;
    }
    return err;
  }
  return kIOReturnError;
}

void byteSwapBlock(UInt32 *data, unsigned int nb_elements) {
  for (unsigned int i = 0; i < nb_elements; i++) {
    data[i] = CFSwapInt32BigToHost(data[i]);
  }
}

IOReturn readBlock(IOFireWireDeviceInterface **deviceInterface,
                   io_service_t service, UInt64 absoluteAddr, UInt32 *value,
                   UInt32 *size, UInt32 generation) {
  if (generation == 0) {
    IOReturn genErr =
        (*deviceInterface)->GetBusGeneration(deviceInterface, &generation);
    if (genErr != kIOReturnSuccess) {
      return genErr;
    }
  }
  // Build the 64-bit FireWire address structure
  FWAddress addr;
  addr.addressHi = static_cast<UInt16>((absoluteAddr >> 32) & 0xFFFF);
  addr.addressLo = static_cast<UInt32>(absoluteAddr & 0xFFFFFFFF);

  IOReturn err = (*deviceInterface)
                     ->Read(deviceInterface, service, &addr, value, size,
                            kFWFailOnReset, generation);

  if (err == kIOReturnSuccess) {
    // Use BigToHost for string data to preserve correct byte order
    byteSwapBlock(value, *size / 4);
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
