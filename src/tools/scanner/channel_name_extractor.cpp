#include "channel_name_extractor.hpp"
#include "channel_discovery.hpp" // For discoverChannelNamesAddress
#include "endianness_helpers.hpp" // For detectDeviceEndianness, deviceToHostInt32
#include "io_helpers.hpp"         // For safeReadQuadlet
#include "scanner.hpp"            // For FireWireDevice
#include "string_extraction.hpp" // For StringMatch, extractStringsFromMemory

#include <algorithm> // For std::sort
#include <iostream>
#include <map>
#include <set> // For std::set
#include <string>
#include <vector>

namespace FWA::SCANNER {

std::map<UInt64, UInt32>
collectChannelRegisters(IOFireWireDeviceInterface **deviceInterface,
                        io_service_t service, UInt64 channelNamesBaseAddr,
                        UInt32 generation, int preRange, int postRange) {
  std::map<UInt64, UInt32> channelRegisters;
  const int STEP = 1; // Scan every quadlet

  std::cout << "Scanning " << preRange * 4 << " bytes before and "
            << postRange * 4 << " bytes after 0x" << std::hex
            << channelNamesBaseAddr << std::dec << " for channel names..."
            << std::endl;

  // Collect all readable registers in the range
  for (int offset = -preRange; offset <= postRange; offset += STEP) {
    UInt64 addr = channelNamesBaseAddr + (offset * 4); // 4 bytes per quadlet
    UInt32 value = 0;

    IOReturn status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service,
                                                    addr, &value, generation);
    if (status == kIOReturnSuccess) {
      channelRegisters[addr] = value;
    }
  }

  return channelRegisters;
}

std::vector<UInt8>
processChannelRegisters(const std::map<UInt64, UInt32> &channelRegisters,
                        DeviceEndianness deviceEndianness) {
  std::vector<UInt8> byteArray;
  byteArray.reserve(channelRegisters.size() * 4); // Each register is 4 bytes

  // Sort registers by address to ensure correct byte order
  std::vector<std::pair<UInt64, UInt32>> sortedRegisters;
  for (const auto &regPair : channelRegisters) {
    sortedRegisters.push_back(regPair);
  }
  std::sort(sortedRegisters.begin(), sortedRegisters.end());

  // Fill the byte array
  for (const auto &regPair : sortedRegisters) {
    UInt32 hostValue = deviceToHostInt32(regPair.second, deviceEndianness);

    // Add bytes in the correct order based on endianness
    if (deviceEndianness == DeviceEndianness::DEVICE_BIG_ENDIAN) {
      // For big-endian, add bytes from most significant to least significant
      byteArray.push_back((hostValue >> 24) & 0xFF);
      byteArray.push_back((hostValue >> 16) & 0xFF);
      byteArray.push_back((hostValue >> 8) & 0xFF);
      byteArray.push_back(hostValue & 0xFF);
    } else // LITTLE_ENDIAN or UNKNOWN_ENDIAN
    {
      // For little-endian, add bytes from least significant to most significant
      byteArray.push_back(hostValue & 0xFF);
      byteArray.push_back((hostValue >> 8) & 0xFF);
      byteArray.push_back((hostValue >> 16) & 0xFF);
      byteArray.push_back((hostValue >> 24) & 0xFF);
    }
  }

  return byteArray;
}

std::vector<StringMatch>
combineStringResults(const std::vector<StringMatch> &quadletStrings,
                     const std::vector<std::string> &byteStrings) {
  // Combine results from both methods, avoiding duplicates
  std::set<std::string> uniqueStringTexts;
  std::vector<StringMatch> combinedStrings;

  // Add strings from quadlet-level extraction first
  for (const auto &match : quadletStrings) {
    if (uniqueStringTexts.insert(match.text).second) {
      combinedStrings.push_back(match);
    }
  }

  // Add strings from byte-level extraction if they're not already included
  for (const auto &str : byteStrings) {
    if (uniqueStringTexts.insert(str).second) {
      StringMatch match;
      match.text = str;
      match.address = 0; // We don't know the exact address
      match.isByteLevel = true;
      combinedStrings.push_back(match);
    }
  }

  return combinedStrings;
}

std::vector<StringMatch>
extractChannelNames(IOFireWireDeviceInterface **deviceInterface,
                    io_service_t service, FireWireDevice &device,
                    UInt32 generation) {
  // Detect the device's endianness
  DeviceEndianness deviceEndianness =
      detectDeviceEndianness(deviceInterface, service, device, generation);
  std::cout << "Detected device endianness: "
            << (deviceEndianness == DeviceEndianness::DEVICE_BIG_ENDIAN
                    ? "BIG_ENDIAN"
                : deviceEndianness == DeviceEndianness::DEVICE_LITTLE_ENDIAN
                    ? "LITTLE_ENDIAN"
                    : "UNKNOWN_ENDIAN")
            << std::endl;

  // Store the detected endianness in the device structure
  device.deviceEndianness = deviceEndianness;

  // Dynamically discover the channel names address
  // This function now prioritizes NAMES_BASE pointers from stream registers and
  // EAP structures
  UInt64 channelNamesBaseAddr =
      discoverChannelNamesAddress(deviceInterface, service, generation);

  // Store the discovered channel names address in the device structure for
  // future reference
  device.channelNamesBaseAddr = channelNamesBaseAddr;
  std::cout << "Using channel names base address: 0x" << std::hex
            << channelNamesBaseAddr << std::dec << std::endl;

  // Collect all readable registers in the range
  std::map<UInt64, UInt32> channelRegisters = collectChannelRegisters(
      deviceInterface, service, channelNamesBaseAddr, generation);

  if (channelRegisters.empty()) {
    std::cout << "No registers could be read in the channel names area."
              << std::endl;
    return {};
  }

  std::cout << "Found " << channelRegisters.size()
            << " readable registers in the channel names area." << std::endl;

  // Convert the register map to a byte array for structure-aware parsing
  std::vector<UInt8> byteArray =
      processChannelRegisters(channelRegisters, deviceEndianness);

  // Try to determine if there's a structure size for channel names
  // Common sizes are 16, 32, or 64 bytes per name slot
  size_t structureSize = 0;

  // Check if we have structure information from NAMES_BASE pointers in TX/RX
  // streams
  if (device.txStreamCount > 0 || device.rxStreamCount > 0) {
    // Look for potential structure size indicators in the device's registers
    for (const auto &regPair : device.diceRegisters) {
      UInt32 hostValue = deviceToHostInt32(regPair.second, deviceEndianness);
      if (hostValue == 16 || hostValue == 32 || hostValue == 64) {
        // This might be a structure size indicator
        structureSize = hostValue;
        std::cout << "Detected potential structure size: " << structureSize
                  << " bytes per channel name slot" << std::endl;
        break;
      }
    }
  }

  // Extract strings using both methods
  // Method 1: Use the improved structure-aware byte-level string extraction
  std::vector<std::string> structureAwareStrings;
  if (structureSize > 0) {
    std::cout << "Using structure-aware string extraction with "
              << structureSize << " byte slots" << std::endl;
    structureAwareStrings = extractPrintableStringsFromBytes(
        byteArray.data(), byteArray.size(), structureSize);
  }

  // Method 2: Use the traditional register-based string extraction
  std::vector<StringMatch> quadletStrings =
      extractStringsFromMemory(channelRegisters, deviceEndianness);

  // Combine results from both methods
  std::vector<StringMatch> combinedStrings =
      combineStringResults(quadletStrings, structureAwareStrings);

  // Report all strings found
  std::cout << "\n--- Channel Names and Related Strings ---" << std::endl;
  for (const auto &match : combinedStrings) {
    if (match.address != 0) {
      std::cout << (match.isByteLevel ? "Byte-level" : "Quadlet-level")
                << " string at 0x" << std::hex << match.address << std::dec
                << ": \"" << match.text << "\"" << std::endl;
    } else {
      std::cout << "Structure-aware string: \"" << match.text << "\""
                << std::endl;
    }
  }

  return combinedStrings;
}

} // namespace FWA::SCANNER
