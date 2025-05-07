#include "string_extraction.hpp"
#include "endianness_helpers.hpp" // For deviceToHostInt32
#include "io_helpers.hpp"         // For interpretAsASCII

#include <algorithm> // For std::sort
#include <iostream>

namespace FWA::SCANNER {

std::vector<StringMatch>
extractStringsFromMemory(const std::map<uint64_t, uint32_t> &registers,
                         DeviceEndianness endianness) {
  std::vector<StringMatch> results;

  // Sort registers by address
  std::vector<std::pair<uint64_t, uint32_t>> sortedRegisters;
  for (const auto &regPair : registers) {
    sortedRegisters.push_back(regPair);
  }
  std::sort(sortedRegisters.begin(), sortedRegisters.end());

  // Extract quadlet-level strings
  std::string currentString;
  uint64_t stringStartAddr = 0;
  bool inString = false;

  for (const auto &regPair : sortedRegisters) {
    uint64_t addr = regPair.first;
    uint32_t rawValue = regPair.second;

    // Convert to host endianness based on detected device endianness
    uint32_t hostValue = deviceToHostInt32(rawValue, endianness);
    std::string ascii = interpretAsASCII(hostValue);

    if (!ascii.empty()) {
      if (!inString) {
        inString = true;
        stringStartAddr = addr;
      }
      currentString += ascii;
    } else {
      if (inString) {
        // End of string
        if (currentString.length() >= 3) // Minimum meaningful length
        {
          StringMatch match;
          match.text = currentString;
          match.address = stringStartAddr;
          match.isByteLevel = false;
          results.push_back(match);
        }
        currentString.clear();
        inString = false;
      }
    }
  }

  // Check if we were in a string at the end
  if (inString && currentString.length() >= 3) {
    StringMatch match;
    match.text = currentString;
    match.address = stringStartAddr;
    match.isByteLevel = false;
    results.push_back(match);
  }

  // Extract byte-level strings with improved termination detection
  std::string byteString;
  uint64_t byteStringStartAddr = 0;
  bool inByteString = false;

  // Track structure information if available
  bool hasStructureInfo = false;
  int structureSize = 0; // Size of each string slot in bytes
  int currentStructureOffset = 0;

  // Check if we have structure information from NAMES_BASE pointers
  // This would typically come from TX/RX stream registers or EAP structures
  for (const auto &regPair : sortedRegisters) {
    // Look for potential structure size indicators
    // Common sizes are 16, 32, or 64 bytes per name slot
    uint32_t hostValue = deviceToHostInt32(regPair.second, endianness);
    if (hostValue == 16 || hostValue == 32 || hostValue == 64) {
      // This might be a structure size indicator
      structureSize = hostValue;
      hasStructureInfo = true;
      std::cerr << "Debug [Utils]: Detected potential structure size: "
                << structureSize << " bytes" << std::endl;
      break;
    }
  }

  for (const auto &regPair : sortedRegisters) {
    uint64_t addr = regPair.first;
    uint32_t rawValue = regPair.second;
    uint32_t hostValue = deviceToHostInt32(rawValue, endianness);

    // Extract individual bytes based on the device's endianness
    std::string bytes;
    bool foundBytes = false;
    bool foundTerminator = false;

    // Extract bytes in the correct order based on endianness
    if (endianness == DeviceEndianness::DEVICE_BIG_ENDIAN) {
      // For big-endian, extract bytes from most significant to least
      // significant
      for (int bytePos = 3; bytePos >= 0; bytePos--) {
        char c = static_cast<char>((hostValue >> (8 * bytePos)) & 0xFF);

        // Improved termination detection
        if (c >= 32 && c <= 126) // Printable ASCII
        {
          bytes += c;
          foundBytes = true;
        } else if (c == 0 && bytes.length() > 0) {
          // Null terminator - definitive end of string
          foundTerminator = true;
          break;
        } else if ((c < 32 || c > 126) && bytes.length() > 0) {
          // Non-printable character - potential end of string
          // For channel names, we're strict about termination
          foundTerminator = true;
          break;
        }
      }
    } else // LITTLE_ENDIAN or UNKNOWN_ENDIAN
    {
      // For little-endian, extract bytes from least significant to most
      // significant
      for (int bytePos = 0; bytePos < 4; bytePos++) {
        char c = static_cast<char>((hostValue >> (8 * bytePos)) & 0xFF);

        // Improved termination detection
        if (c >= 32 && c <= 126) // Printable ASCII
        {
          bytes += c;
          foundBytes = true;
        } else if (c == 0 && bytes.length() > 0) {
          // Null terminator - definitive end of string
          foundTerminator = true;
          break;
        } else if ((c < 32 || c > 126) && bytes.length() > 0) {
          // Non-printable character - potential end of string
          // For channel names, we're strict about termination
          foundTerminator = true;
          break;
        }
      }
    }

    // Process the extracted bytes
    if (!bytes.empty()) {
      if (!inByteString) {
        inByteString = true;
        byteStringStartAddr = addr;

        // If we have structure information, track the offset within the
        // structure
        if (hasStructureInfo) {
          // Calculate offset within the current structure
          currentStructureOffset = (addr % (structureSize / 4)) *
                                   4; // Convert quadlet offset to byte offset
        }
      }
      byteString += bytes;
    } else if (inByteString) {
      // Check if we should end the current string
      bool endString = false;

      if (foundTerminator) {
        // Explicit terminator found
        endString = true;
      } else if (hasStructureInfo) {
        // Check if we've reached the end of the current structure slot
        int newOffset = (addr % (structureSize / 4)) * 4;
        if (newOffset < currentStructureOffset) {
          // We've wrapped around to a new structure slot
          endString = true;
        }
        currentStructureOffset = newOffset;
      } else {
        // No structure info, no terminator - end string if no ASCII found
        endString = true;
      }

      if (endString) {
        // End the current string if it meets minimum length requirements
        if (byteString.length() >= 3) // Minimum meaningful length
        {
          StringMatch match;
          match.text = byteString;
          match.address = byteStringStartAddr;
          match.isByteLevel = true;
          results.push_back(match);
        }
        byteString.clear();
        inByteString = false;
      }
    }
  }

  // Check for any remaining byte string
  if (inByteString && byteString.length() >= 3) {
    StringMatch match;
    match.text = byteString;
    match.address = byteStringStartAddr;
    match.isByteLevel = true;
    results.push_back(match);
  }

  return results;
}

std::vector<std::string> extractPrintableStringsFromBytes(const uint8_t *bytes,
                                                          size_t length,
                                                          size_t structureSize,
                                                          size_t minLength) {
  std::vector<std::string> results;

  if (!bytes || length == 0)
    return results;

  std::string currentString;
  size_t currentStructureOffset = 0;
  bool hasStructureInfo = (structureSize > 0);

  for (size_t i = 0; i < length; i++) {
    uint8_t byte = bytes[i];

    // Check if this is a printable ASCII character
    if (byte >= 32 && byte <= 126) {
      // Printable character - add to current string
      currentString += static_cast<char>(byte);
    } else {
      // Non-printable character - potential string terminator
      bool endString = false;

      if (byte == 0) {
        // Null terminator - definitive end of string
        endString = true;
      } else if (currentString.length() > 0) {
        // Non-printable, non-null character after we've started a string
        // For channel names, we're strict about termination
        endString = true;
      }

      // If we have structure information, check if we've reached a structure
      // boundary
      if (hasStructureInfo) {
        // Calculate current position within structure
        currentStructureOffset = i % structureSize;

        // If we're at the end of a structure slot, end the string
        if (currentStructureOffset == structureSize - 1) {
          endString = true;
        }
      }

      // If we need to end the string and it meets minimum length requirements
      if (endString && currentString.length() >= minLength) {
        results.push_back(currentString);
        currentString.clear();
      } else if (endString) {
        // String too short, discard it
        currentString.clear();
      }
    }
  }

  // Check for any remaining string at the end of the buffer
  if (currentString.length() >= minLength) {
    results.push_back(currentString);
  }

  return results;
}

} // namespace FWA::SCANNER
