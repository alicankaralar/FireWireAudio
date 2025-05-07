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
static std::string identifyDiceSpace(uint64_t absoluteAddr) {
  // Remove the base address to get relative offset
  uint64_t relativeAddr = absoluteAddr & 0x0FFFFFFFULL;

  // Check EAP space first (it's a large range)
  if (relativeAddr >= DICE_EAP_BASE &&
      relativeAddr < (DICE_EAP_BASE + DICE_EAP_MAX_SIZE)) {
    uint64_t eapOffset = relativeAddr - DICE_EAP_BASE;
    std::string subspace;
    if (eapOffset < 0x0004)
      subspace = "Capability";
    else if (eapOffset < 0x000C)
      subspace = "Command";
    else if (eapOffset < 0x0014)
      subspace = "Mixer";
    else if (eapOffset < 0x001C)
      subspace = "Peak";
    else if (eapOffset < 0x0024)
      subspace = "NewRouting";
    else if (eapOffset < 0x002C)
      subspace = "NewStreamCfg";
    else if (eapOffset < 0x0034)
      subspace = "CurrCfg";
    else if (eapOffset < 0x003C)
      subspace = "StandAloneCfg";
    else if (eapOffset < 0x0044)
      subspace = "App";
    else
      subspace = "Unknown";
    return "EAP:" + subspace + " (offset 0x" + std::to_string(eapOffset) + ")";
  }

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
                         io_service_t service, uint64_t absoluteAddr,
                         UInt32 &value, UInt32 generation, bool forceQuadlet) {
  g_segfaultOccurred = false;

  if (setjmp(g_jmpBuf) == 0) {
    if (!deviceInterface || !*deviceInterface) {
      return kIOReturnBadArgument;
    }

    FWAddress addr;
    addr.addressHi = static_cast<UInt16>((absoluteAddr >> 32) & 0xFFFF);
    addr.addressLo = static_cast<UInt32>(absoluteAddr & 0xFFFFFFFF);

    IOReturn err =
        FWA::SCANNER::readQuadlet(deviceInterface, service, absoluteAddr, value,
                                  generation, forceQuadlet);

    return err;
  }

  return kIOReturnError;
}

IOReturn readQuadlet(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, uint64_t absoluteAddr, UInt32 &value,
                     UInt32 generation, bool forceQuadlet) {
  if (!deviceInterface || !*deviceInterface) {
    return kIOReturnBadArgument;
  }

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

  IOReturn err = kIOReturnSuccess;

  std::string spaceInfo = identifyDiceSpace(absoluteAddr);
  uint64_t relativeAddr = absoluteAddr & 0x0FFFFFFFULL;

  // Enhanced logging for EAP space analysis
  if (relativeAddr >= DICE_EAP_BASE &&
      relativeAddr < (DICE_EAP_BASE + DICE_EAP_MAX_SIZE)) {
    std::cerr << "Debug [EAP]: Accessing EAP space at 0x" << std::hex
              << absoluteAddr << " (relative: 0x" << relativeAddr << ")"
              << std::dec << std::endl;
  }

  // Determine read method and address transformation based on register space
  bool useQuadlet = forceQuadlet;
  uint64_t transformedAddr = absoluteAddr;
  if (!useQuadlet) {
    uint64_t relativeAddr = absoluteAddr & 0x0FFFFFFFULL;

    // Transform subsystem and EAP addresses to match FireWire address space
    // format
    if (relativeAddr >= 0xC7000000) {
      // Extract subsystem offset
      uint64_t subsystemOffset = relativeAddr & 0x00FFFFFFULL;

      // Map to standard FireWire address space
      transformedAddr = 0xFFFFE0000000ULL | subsystemOffset;

      std::cerr << "Debug [Transform]: Subsystem address transformation"
                << std::endl
                << "  Original:    0x" << std::hex << absoluteAddr << std::endl
                << "  Relative:    0x" << relativeAddr << std::endl
                << "  Offset:      0x" << subsystemOffset << std::endl
                << "  Transformed: 0x" << transformedAddr << std::dec
                << std::endl;

      // Update address structure with transformed address
      addr.addressHi = static_cast<UInt16>((transformedAddr >> 32) & 0xFFFF);
      addr.addressLo = static_cast<UInt32>(transformedAddr & 0xFFFFFFFF);
    } else if (relativeAddr >= DICE_EAP_BASE &&
               relativeAddr < (DICE_EAP_BASE + DICE_EAP_MAX_SIZE)) {
      // Extract EAP offset
      uint64_t eapOffset = relativeAddr - DICE_EAP_BASE;

      // Try similar transformation as subsystem addresses
      uint64_t originalTransformedAddr = transformedAddr;
      transformedAddr = 0xFFFFE0000000ULL | eapOffset;

      std::cerr << "Debug [Transform]: Testing EAP address transformation"
                << std::endl
                << "  Original:    0x" << std::hex << absoluteAddr << std::endl
                << "  Relative:    0x" << relativeAddr << std::endl
                << "  EAP Offset:  0x" << eapOffset << std::endl
                << "  Original TR: 0x" << originalTransformedAddr << std::endl
                << "  Test TR:     0x" << transformedAddr << std::dec
                << std::endl;

      // Update address structure with transformed address
      addr.addressHi = static_cast<UInt16>((transformedAddr >> 32) & 0xFFFF);
      addr.addressLo = static_cast<UInt32>(transformedAddr & 0xFFFFFFFF);
    }
    // Log potential EAP space transformations
    else if (relativeAddr >= DICE_EAP_BASE &&
             relativeAddr < (DICE_EAP_BASE + DICE_EAP_MAX_SIZE)) {
      // Experimental: Try transforming EAP addresses similar to subsystem
      // addresses
      uint64_t eapOffset = relativeAddr - DICE_EAP_BASE;
      uint64_t transformedEapAddr = 0xFFFFE0200000ULL | eapOffset;

      std::cerr << "Debug [Transform]: EAP address analysis" << std::endl;
      std::cerr << "  Original:    0x" << std::hex << absoluteAddr << std::endl;
      std::cerr << "  Relative:    0x" << relativeAddr << std::endl;
      std::cerr << "  EAP Offset:  0x" << eapOffset << std::endl;
      std::cerr << "  Section:     0x" << (eapOffset & 0xFFF0);

      // Identify EAP section
      switch (eapOffset & 0xFFF0) {
      case DICE_EAP_CAPABILITY_SPACE_OFF:
        std::cerr << " (Capability)";
        break;
      case DICE_EAP_CMD_SPACE_OFF:
        std::cerr << " (Command)";
        break;
      case DICE_EAP_MIXER_SPACE_OFF:
        std::cerr << " (Mixer)";
        break;
      case DICE_EAP_PEAK_SPACE_OFF:
        std::cerr << " (Peak)";
        break;
      case DICE_EAP_NEW_ROUTING_SPACE_OFF:
        std::cerr << " (New Routing)";
        break;
      case DICE_EAP_NEW_STREAM_CFG_SPACE_OFF:
        std::cerr << " (Stream Config)";
        break;
      case DICE_EAP_CURR_CFG_SPACE_OFF:
        std::cerr << " (Current Config)";
        break;
      case DICE_EAP_STAND_ALONE_CFG_SPACE_OFF:
        std::cerr << " (Standalone Config)";
        break;
      case DICE_EAP_APP_SPACE_OFF:
        std::cerr << " (App)";
        break;
      default:
        std::cerr << " (Unknown)";
      }
      std::cerr << std::dec << std::endl;

      std::cerr << "  Sub-offset: 0x" << std::hex << (eapOffset & 0x000F)
                << std::endl;
      std::cerr << "  Transform:  0x" << transformedEapAddr << std::dec
                << std::endl;

      // Try the transformed address
      transformedAddr = transformedEapAddr;
      addr.addressHi = static_cast<UInt16>((transformedAddr >> 32) & 0xFFFF);
      addr.addressLo = static_cast<UInt32>(transformedAddr & 0xFFFFFFFF);
    }

    // Always use ReadQuadlet for standard spaces
    if (relativeAddr < 0xC00) {
      useQuadlet = true;
    }
    // Use Read for EAP space
    else if (relativeAddr >= DICE_EAP_BASE &&
             relativeAddr < (DICE_EAP_BASE + DICE_EAP_MAX_SIZE)) {
      useQuadlet = false;
    }
    // Use Read for transformed subsystem addresses
    else if (relativeAddr >= 0xC7000000) {
      useQuadlet = false;
    }
    // Default to ReadQuadlet for unknown spaces
    else {
      useQuadlet = true;
    }
  }

  if (useQuadlet) {
    // Use ReadQuadlet
    err = (*deviceInterface)
              ->ReadQuadlet(deviceInterface, service, &addr, &value,
                            kFWFailOnReset, generation);
    // No logging for success
  } else {
    // Use Read for high addresses
    UInt32 numBytesExpected = sizeof(UInt32);
    UInt32 numBytesToRead = numBytesExpected;

    err = (*deviceInterface)
              ->Read(deviceInterface, service, &addr, &value, &numBytesToRead,
                     kFWFailOnReset, generation);

    // Enhanced EAP space access logging
    if (relativeAddr >= DICE_EAP_BASE &&
        relativeAddr < (DICE_EAP_BASE + DICE_EAP_MAX_SIZE)) {
      // Calculate EAP-specific offsets and ranges
      uint64_t eapOffset = relativeAddr - DICE_EAP_BASE;
      uint64_t eapSection = eapOffset & 0xFFF0;
      uint64_t eapSubOffset = eapOffset & 0x000F;

      std::cerr << "Debug [EAP]: Access analysis" << std::endl;
      std::cerr << "  Address:     0x" << std::hex << absoluteAddr << std::dec
                << std::endl;
      std::cerr << "  Base:        0x" << std::hex << DICE_EAP_BASE << std::dec
                << std::endl;
      std::cerr << "  Offset:      0x" << std::hex << eapOffset << std::dec
                << std::endl;
      std::cerr << "  Section:     0x" << std::hex << eapSection << std::dec;

      // Identify EAP section
      switch (eapSection) {
      case DICE_EAP_CAPABILITY_SPACE_OFF:
        std::cerr << " (Capability)";
        break;
      case DICE_EAP_CMD_SPACE_OFF:
        std::cerr << " (Command)";
        break;
      case DICE_EAP_MIXER_SPACE_OFF:
        std::cerr << " (Mixer)";
        break;
      case DICE_EAP_PEAK_SPACE_OFF:
        std::cerr << " (Peak)";
        break;
      case DICE_EAP_NEW_ROUTING_SPACE_OFF:
        std::cerr << " (New Routing)";
        break;
      case DICE_EAP_NEW_STREAM_CFG_SPACE_OFF:
        std::cerr << " (Stream Config)";
        break;
      case DICE_EAP_CURR_CFG_SPACE_OFF:
        std::cerr << " (Current Config)";
        break;
      case DICE_EAP_STAND_ALONE_CFG_SPACE_OFF:
        std::cerr << " (Standalone Config)";
        break;
      case DICE_EAP_APP_SPACE_OFF:
        std::cerr << " (App)";
        break;
      default:
        std::cerr << " (Unknown)";
      }
      std::cerr << std::endl;
      std::cerr << "  Sub-offset:  0x" << std::hex << eapSubOffset << std::dec
                << std::endl;

      // Log read results
      if (err == kIOReturnSuccess) {
        std::cerr << "  Result:      Success" << std::endl;
        std::cerr << "  Value:       0x" << std::hex << value << std::dec
                  << std::endl;
        std::cerr << "  Bytes read:  " << numBytesToRead << "/"
                  << numBytesExpected << std::endl;
        // Try to interpret value as ASCII if it looks like text
        std::string ascii = interpretAsASCII(value);
        if (!ascii.empty()) {
          std::cerr << "  ASCII:       \"" << ascii << "\"" << std::endl;
        }
      } else {
        std::cerr << "  Result:      Failed (0x" << std::hex << err << std::dec
                  << ")" << std::endl;
        std::cerr << "  Bytes read:  " << numBytesToRead << "/"
                  << numBytesExpected << std::endl;
      }
      std::cerr << std::endl;
    }

    if (err == kIOReturnSuccess && numBytesToRead != numBytesExpected) {
      err = kIOReturnUnderrun;
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
