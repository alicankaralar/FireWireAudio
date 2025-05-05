#include "config_rom.hpp"
#include "io_helpers.hpp" // For safeReadQuadlet
#include "scanner.hpp" // For DICE_INVALID_OFFSET definition via DiceDefines.hpp

#include <iomanip> // For std::hex
#include <iostream>
#include <map>     // Added for std::map
#include <sstream> // For std::stringstream
#include <string>  // For key descriptions

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32BigToHost
#include <IOKit/firewire/IOFireWireFamilyCommon.h> // Provides some types, but not the keys previously in FWStandardHeaders.h

namespace FWA::SCANNER {

// --- Config ROM Parsing Implementation ---

std::map<uint32_t, uint64_t>
parseConfigRomVendorKeys(IOFireWireDeviceInterface **deviceInterface,
                         io_service_t service, UInt32 generation) {
  std::cerr
      << "Debug [ConfigROM]: Parsing Config ROM for Vendor Keys (0xD0-0xFF)..."
      << std::dec << std::endl;
  const uint64_t CONFIG_ROM_BASE = 0xFFFFF0000400ULL;
  const size_t MAX_ROM_READ_QUADLETS = 64; // Read up to 256 bytes initially

  std::map<uint32_t, uint64_t>
      vendorKeyAddresses; // Map to store found vendor key addresses

  // 1. Read Bus Info Block Header (Offset 0x400) - Optional for now, assume
  // length 4 UInt32 busInfoHeader = 0; IOReturn status =
  // FWA::SCANNER::safeReadQuadlet(deviceInterface, service, CONFIG_ROM_BASE,
  // busInfoHeader, generation);
  // ... check status ...
  // busInfoHeader = CFSwapInt32BigToHost(busInfoHeader);
  // uint32_t busInfoLength = (busInfoHeader >> 24) & 0xFF; // Usually 4

  // 2. Read Root Directory Header (Offset 0x414)
  uint64_t rootDirHeaderAddr = CONFIG_ROM_BASE + (5 * 4); // 0xFFFFF0000414
  UInt32 rootDirHeader = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, rootDirHeaderAddr, rootDirHeader, generation);
  if (status != kIOReturnSuccess) {
    std::cerr << "Error [ConfigROM]: Failed to read Root Directory Header at 0x"
              << std::hex << rootDirHeaderAddr << " (status: " << status << ")"
              << std::dec << std::endl;
    return vendorKeyAddresses; // Return empty map on error
  }
  rootDirHeader = CFSwapInt32BigToHost(rootDirHeader);
  std::cerr << "  Config ROM [0x" << std::hex << rootDirHeaderAddr << "]: 0x"
            << rootDirHeader << " (Host) [Root Dir Header]" << std::dec
            << std::endl;

  // 3. Extract Root Directory Length & Calculate Offset
  uint32_t rootDirLength =
      (rootDirHeader >> 16) & 0xFFFF; // Length is in upper 16 bits
  uint64_t rootDirOffset =
      CONFIG_ROM_BASE +
      (6 * 4); // Root Dir entries start *after* the header (Offset 0x418)

  // Sanity check the length
  if (rootDirLength == 0 ||
      rootDirLength > MAX_ROM_READ_QUADLETS) { // Check against max read size
    std::cerr
        << "Error [ConfigROM]: Invalid or too large root directory length: "
        << rootDirLength << std::endl;
    return vendorKeyAddresses; // Return empty map on error
  }
  std::cerr << "Debug [ConfigROM]: Root Directory Length: " << rootDirLength
            << " quadlets." << std::endl;

  // 4. Find Unit Directory Offset in Root Directory
  uint64_t unitDirAddr = DICE_INVALID_OFFSET;
  for (uint32_t i = 0; i < rootDirLength; ++i) {
    uint64_t entryAddr = rootDirOffset + (i * 4);
    UInt32 entryValue = 0;
    IOReturn status = FWA::SCANNER::safeReadQuadlet(
        deviceInterface, service, entryAddr, entryValue, generation);
    if (status != kIOReturnSuccess) {
      std::cerr << "Error [ConfigROM]: Failed to read Root Directory entry "
                << i << " at 0x" << std::hex << entryAddr
                << " (status: " << status << ")" << std::dec << std::endl;
      return vendorKeyAddresses; // Return potentially partially filled map on
                                 // error
    }
    entryValue = CFSwapInt32BigToHost(entryValue);
    uint32_t key = (entryValue >> 24) & 0xFF;
    uint32_t value =
        entryValue & 0xFFFFFF; // Offset in quadlets relative to Config ROM Base

    std::cerr << "  Root Dir Entry " << i << " [0x" << std::hex << entryAddr
              << "]: Raw=0x" << entryValue << ", Key=0x" << key << ", Value=0x"
              << value << std::dec << std::endl;

    if (key == kConfigDirectoryKey_Unit) { // Found Unit Directory Key (0x17)
      // Offset 'value' is relative to the start of the containing directory
      // (rootDirOffset)
      unitDirAddr = rootDirOffset + (value * 4);
      std::cerr << "Debug [ConfigROM]: Found Unit_Directory key (0x17) "
                   "pointing to offset 0x"
                << std::hex << value << " relative to Root Dir (0x"
                << rootDirOffset << "). Absolute Address: 0x" << unitDirAddr
                << std::dec << std::endl;
      break;
    }
  }

  if (unitDirAddr == DICE_INVALID_OFFSET) {
    std::cerr << "Warning [ConfigROM]: Unit_Directory key (0x17) not found in "
                 "Root Directory."
              << std::dec << std::endl;
    return vendorKeyAddresses; // Return empty map
  }

  // 5. Parse the Unit Directory to find vendor keys (0xD0 - 0xFF)
  UInt32 unitDirHeader = 0;
  status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, unitDirAddr, unitDirHeader,
      generation); // Assign to existing status variable
  if (status != kIOReturnSuccess) {
    std::cerr << "Error [ConfigROM]: Failed to read Unit Directory header at 0x"
              << std::hex << unitDirAddr << " (status: " << status << ")"
              << std::dec << std::endl;
    return vendorKeyAddresses; // Return empty map
  }
  unitDirHeader = CFSwapInt32BigToHost(unitDirHeader);
  uint32_t unitDirLength = unitDirHeader & 0xFFFF; // Lower 16 bits
  std::cerr << "Debug [ConfigROM]: Unit Directory Header: 0x" << std::hex
            << unitDirHeader << ", Length: " << std::dec << unitDirLength
            << " quadlets." << std::endl;

  if (unitDirLength == 0 ||
      unitDirLength > MAX_ROM_READ_QUADLETS) { // Sanity check
    std::cerr
        << "Error [ConfigROM]: Invalid or too large unit directory length: "
        << unitDirLength << std::endl;
    return vendorKeyAddresses; // Return empty map
  }

  uint64_t unitDirEntryOffset = unitDirAddr + 4; // Start after header
  for (uint32_t i = 0; i < unitDirLength; ++i) {
    uint64_t entryAddr = unitDirEntryOffset + (i * 4);
    UInt32 entryValue = 0;
    status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, entryAddr,
                                           entryValue, generation);
    if (status != kIOReturnSuccess) {
      std::cerr << "Error [ConfigROM]: Failed to read Unit Directory entry "
                << i << " at 0x" << std::hex << entryAddr
                << " (status: " << status << ")" << std::dec << std::endl;
      return vendorKeyAddresses; // Return potentially partially filled map
    }
    entryValue = CFSwapInt32BigToHost(entryValue);
    uint32_t key = (entryValue >> 24) & 0xFF;
    uint32_t value =
        entryValue & 0xFFFFFF; // Offset relative to the start of the containing
                               // directory (unitDirAddr)

    // More detailed logging within the Unit Directory loop
    std::cerr << "  Unit Dir Entry " << i << " [0x" << std::hex << entryAddr
              << "]: Raw=0x" << entryValue << ", Key=0x" << key << ", Value=0x"
              << value << std::dec;

    // Add descriptions for common keys found within Unit Directory
    std::string keyDesc = "";
    if (key >= 0xD0 && key <= 0xFF) {
      keyDesc = " (Vendor Specific)";
    } else {
      switch (key) {
      case kConfigDirectoryKey_Unit_Spec_ID:
        keyDesc = " (Unit_Spec_ID)";
        break;
      case kConfigDirectoryKey_Unit_SW_Version:
        keyDesc = " (Unit_SW_Version)";
        break;
      case kConfigDirectoryKey_Model:
        keyDesc = " (Model)";
        break;
      case kConfigDirectoryKey_Unit_Dependent:
        keyDesc = " (Unit_Dependent)";
        break;
        // Add other relevant non-vendor keys if known
      }
    }
    std::cerr << keyDesc << std::endl;

    // Check if this entry is a vendor-specific key (0xD0 - 0xFF)
    if (key >= 0xD0 && key <= 0xFF) {
      uint64_t absoluteAddr = unitDirAddr + (value * 4);
      std::cerr << "Debug [ConfigROM]: Found Vendor Specific key (0x"
                << std::hex << key << ") pointing to offset 0x" << value
                << " relative to Unit Dir (0x" << unitDirAddr
                << "). Absolute Address: 0x" << absoluteAddr << std::dec
                << std::endl;
      vendorKeyAddresses[key] = absoluteAddr;
      // Continue searching for other vendor keys
    }

    // Log other relevant information based on key (optional, can be removed if
    // only vendor keys are needed)
    switch (key) {
    case kConfigDirectoryKey_Unit_Spec_ID: // 0x1C (or 0x03?)
      std::cerr << " - Found Unit_Spec_ID: 0x" << std::hex << value << std::dec
                << std::endl;
      break;
    case kConfigDirectoryKey_Unit_SW_Version: // 0x1D (or 0x0C?)
      std::cerr << " - Found Software Version: 0x" << std::hex << value
                << std::dec << std::endl;
      break;
    case kConfigDirectoryKey_Model: // 0x07
      std::cerr << " - Found Model string offset: 0x" << std::hex << value
                << std::dec << std::endl;
      break;
    case kConfigDirectoryKey_Unit_Dependent: // 0xC1
      std::cerr << " - Found Unit_Dependent_Directory key (0xC1) pointing to "
                   "offset 0x"
                << std::hex << value << " relative to Unit Dir (0x"
                << unitDirAddr << "). Absolute Address: 0x"
                << (unitDirAddr + (value * 4)) << std::dec << std::endl;
      break;
      // Vendor keys are handled above
    default:
      if (!(key >= 0xD0 &&
            key <= 0xFF)) { // Avoid double newline for vendor keys
        std::cerr
            << std::endl; // Newline for entries without specific descriptions
      }
      break;
    }
  }

  // Finished parsing the Unit Directory
  std::cerr << "Debug [ConfigROM]: Finished parsing Unit Directory. Found "
            << vendorKeyAddresses.size() << " vendor keys." << std::dec
            << std::endl;
  return vendorKeyAddresses;
}

} // namespace FWA::SCANNER
