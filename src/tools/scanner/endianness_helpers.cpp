#include "endianness_helpers.hpp"
#include "FWA/dice/DiceAbsoluteAddresses.hpp"
#include "io_helpers.hpp" // For safeReadQuadlet
#include "scanner.hpp"    // For FireWireDevice

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost, CFSwapInt32BigToHost
#include <iostream>

namespace FWA::SCANNER {
// Host system endianness detection
static bool isHostLittleEndian() {
  static const UInt32 test = 0x01020304;
  static const bool isLittle =
      (*reinterpret_cast<const UInt8 *>(&test) == 0x04);
  return isLittle;
}

DeviceEndianness
detectDeviceEndianness(IOFireWireDeviceInterface **deviceInterface,
                       io_service_t service, FireWireDevice &device,
                       UInt32 generation) {
  std::cerr << "Info [Endianness]: Detecting device endianness..." << std::endl;

  // Method 1: Check GPCSR_CHIP_ID register
  // This register has a specific format where the chip type is in a specific
  // bit position If interpreted with the wrong endianness, the chip type would
  // be invalid
  UInt64 chipIdAddr = GPCSR_CHIP_ID_ADDR;
  UInt32 chipIdValue = 0;
  IOReturn status = safeReadQuadlet(deviceInterface, service, chipIdAddr,
                                    &chipIdValue, generation);

  if (status == kIOReturnSuccess) {
    // Try little-endian interpretation
    UInt32 littleEndianValue = CFSwapInt32LittleToHost(chipIdValue);
    UInt32 chipTypeLittle =
        (littleEndianValue & GPCSR_CHIP_ID_CHIP_TYPE_MASK) >>
        GPCSR_CHIP_ID_CHIP_TYPE_SHIFT;

    // Try big-endian interpretation
    UInt32 bigEndianValue = CFSwapInt32BigToHost(chipIdValue);
    UInt32 chipTypeBig = (bigEndianValue & GPCSR_CHIP_ID_CHIP_TYPE_MASK) >>
                         GPCSR_CHIP_ID_CHIP_TYPE_SHIFT;

    // Check which interpretation gives a valid chip type
    bool validLittle =
        (chipTypeLittle >= static_cast<UInt32>(DiceChipType::DiceII) &&
         chipTypeLittle <= static_cast<UInt32>(DiceChipType::DiceJr));

    bool validBig = (chipTypeBig >= static_cast<UInt32>(DiceChipType::DiceII) &&
                     chipTypeBig <= static_cast<UInt32>(DiceChipType::DiceJr));

    if (validLittle && !validBig) {
      std::cerr << "Info [Endianness]: Device detected as LITTLE_ENDIAN based "
                   "on CHIP_ID register."
                << std::endl;
      return DeviceEndianness::DEVICE_LITTLE_ENDIAN;
    } else if (validBig && !validLittle) {
      std::cerr << "Info [Endianness]: Device detected as BIG_ENDIAN based on "
                   "CHIP_ID register."
                << std::endl;
      return DeviceEndianness::DEVICE_BIG_ENDIAN;
    } else if (validBig && validLittle) {
      std::cerr << "Warning [Endianness]: Both endianness interpretations give "
                   "valid CHIP_ID values. Continuing with other methods."
                << std::endl;
    } else {
      std::cerr << "Warning [Endianness]: Neither endianness interpretation "
                   "gives valid CHIP_ID values. Continuing with other methods."
                << std::endl;
    }
  }

  // Method 2: Check BUS_INFO_BLOCK in Config ROM
  // The BUS_INFO_BLOCK contains fields like VENDOR_ID that should have valid
  // values when interpreted with the correct endianness
  if (!device.configRomVendorKeys.empty()) {
    for (const auto &pair : device.configRomVendorKeys) {
      UInt32 key = pair.first;
      if (key == 0x03 || key == 0x0C) // Common vendor key IDs
      {
        // Try to read the value at this address
        UInt64 vendorAddr = pair.second;
        UInt32 vendorValue = 0;
        IOReturn status = safeReadQuadlet(deviceInterface, service, vendorAddr,
                                          &vendorValue, generation);

        if (status == kIOReturnSuccess) {
          // For vendor IDs, big-endian is typically the correct interpretation
          // as FireWire is a big-endian bus
          UInt32 bigEndianValue = CFSwapInt32BigToHost(vendorValue);

          // Check if the value looks like a valid vendor ID (non-zero,
          // reasonable range)
          if (bigEndianValue != 0 && bigEndianValue < 0x00FFFFFF) {
            std::cerr << "Info [Endianness]: Device detected as BIG_ENDIAN "
                         "based on Config ROM vendor key."
                      << std::endl;
            return DeviceEndianness::DEVICE_BIG_ENDIAN;
          }
        }
      }
    }
  }

  // Method 3: Check for ASCII strings in device registers
  // If we can find valid ASCII strings when interpreted with a specific
  // endianness, that's likely the correct endianness

  // Try to read the device name or other string registers
  UInt64 deviceNameAddr = DICE_DEVICE_NAME_ADDR;
  UInt32 nameValue = 0;
  status = safeReadQuadlet(deviceInterface, service, deviceNameAddr, &nameValue,
                           generation);

  if (status == kIOReturnSuccess) {
    // Try little-endian interpretation
    UInt32 littleEndianValue = CFSwapInt32LittleToHost(nameValue);
    std::string littleEndianStr;
    for (int i = 0; i < 4; i++) {
      char c = static_cast<char>((littleEndianValue >> (i * 8)) & 0xFF);
      if (c >= 32 && c <= 126) // Printable ASCII
        littleEndianStr += c;
    }

    // Try big-endian interpretation
    UInt32 bigEndianValue = CFSwapInt32BigToHost(nameValue);
    std::string bigEndianStr;
    for (int i = 0; i < 4; i++) {
      char c = static_cast<char>((bigEndianValue >> ((3 - i) * 8)) & 0xFF);
      if (c >= 32 && c <= 126) // Printable ASCII
        bigEndianStr += c;
    }

    // Check which interpretation gives more valid ASCII characters
    if (littleEndianStr.length() > bigEndianStr.length()) {
      std::cerr << "Info [Endianness]: Device detected as LITTLE_ENDIAN based "
                   "on string analysis."
                << std::endl;
      return DeviceEndianness::DEVICE_LITTLE_ENDIAN;
    } else if (bigEndianStr.length() > littleEndianStr.length()) {
      std::cerr << "Info [Endianness]: Device detected as BIG_ENDIAN based on "
                   "string analysis."
                << std::endl;
      return DeviceEndianness::DEVICE_BIG_ENDIAN;
    }
  }

  // Method 4: Default to big-endian as FireWire is a big-endian bus
  std::cerr << "Warning [Endianness]: Could not definitively determine device "
               "endianness. Defaulting to BIG_ENDIAN (FireWire standard)."
            << std::endl;
  return DeviceEndianness::DEVICE_BIG_ENDIAN;
}

UInt32 deviceToHostInt32(UInt32 value, DeviceEndianness endianness) {
  switch (endianness) {
  case DeviceEndianness::DEVICE_BIG_ENDIAN:
    return CFSwapInt32BigToHost(value);
  case DeviceEndianness::DEVICE_LITTLE_ENDIAN:
    return CFSwapInt32LittleToHost(value);
  case DeviceEndianness::UNKNOWN_ENDIAN:
  default:
    // For unknown endianness, try both and return the one that seems more
    // reasonable This is a fallback to the previous heuristic approach
    UInt32 bigEndianValue = CFSwapInt32BigToHost(value);
    UInt32 littleEndianValue = CFSwapInt32LittleToHost(value);

    // Simple heuristic: if the value is a potential ASCII string, check which
    // has more printable chars
    int bigEndianPrintable = 0;
    int littleEndianPrintable = 0;

    for (int i = 0; i < 4; i++) {
      char bigChar =
          static_cast<char>((bigEndianValue >> ((3 - i) * 8)) & 0xFF);
      if (bigChar >= 32 && bigChar <= 126)
        bigEndianPrintable++;

      char littleChar =
          static_cast<char>((littleEndianValue >> (i * 8)) & 0xFF);
      if (littleChar >= 32 && littleChar <= 126)
        littleEndianPrintable++;
    }

    if (bigEndianPrintable > littleEndianPrintable)
      return bigEndianValue;
    else
      return littleEndianValue;
  }
}

UInt32 hostToDeviceInt32(UInt32 value, DeviceEndianness endianness) {
  switch (endianness) {
  case DeviceEndianness::DEVICE_BIG_ENDIAN:
    return CFSwapInt32HostToBig(value);
  case DeviceEndianness::DEVICE_LITTLE_ENDIAN:
    return CFSwapInt32HostToLittle(value);
  case DeviceEndianness::UNKNOWN_ENDIAN:
  default:
    // For unknown endianness, default to big-endian (FireWire standard)
    return CFSwapInt32HostToBig(value);
  }
}

void byteSwapFromBus(UInt32 *data, size_t length) {
  if (!data || length == 0) {
    return;
  }

  // FireWire bus data is always in big-endian format (network byte order)
  // If the host is little-endian, we need to swap bytes for each quadlet
  if (isHostLittleEndian()) {
    for (size_t i = 0; i < length; i++) {
      data[i] = CFSwapInt32BigToHost(data[i]);
    }
  }
  // If the host is big-endian, no swap is needed as the bus and host formats
  // match
}

} // namespace FWA::SCANNER
