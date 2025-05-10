#include "register_address_ranges.hpp"
#include "io_helpers.hpp" // For interpretAsASCII

#include <algorithm> // For std::sort
#include <iomanip> // For std::setw, std::setfill, std::left, std::hex, std::dec
#include <iostream>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost, CFSwapInt32BigToHost

namespace FWA::SCANNER {

std::vector<RegisterAddressRange>
groupRegistersByAddressRange(const std::map<UInt64, UInt32> &registers) {
  std::vector<RegisterAddressRange> addressRanges;

  // Sort registers by address for sequential processing
  std::vector<std::pair<UInt64, UInt32>> sortedRegisters;
  for (const auto &regPair : registers) {
    sortedRegisters.push_back(regPair);
  }
  std::sort(sortedRegisters.begin(), sortedRegisters.end());

  // Group registers by address proximity
  RegisterAddressRange currentRange;
  currentRange.baseAddress = 0;

  for (size_t i = 0; i < sortedRegisters.size(); i++) {
    UInt64 addr = sortedRegisters[i].first;
    UInt32 value = sortedRegisters[i].second;

    if (currentRange.registers.empty() ||
        addr - currentRange.registers.back().first <= 16) {
      // Within 16 bytes, consider part of the same group
      if (currentRange.registers.empty()) {
        currentRange.baseAddress = addr;
      }
      currentRange.registers.push_back({addr, value});
    } else {
      // Start a new group
      if (!currentRange.registers.empty()) {
        addressRanges.push_back(currentRange);
        currentRange.registers.clear();
      }
      currentRange.baseAddress = addr;
      currentRange.registers.push_back({addr, value});
    }
  }

  // Add the last group if not empty
  if (!currentRange.registers.empty()) {
    addressRanges.push_back(currentRange);
  }

  return addressRanges;
}

void processAddressRangesForStrings(
    const std::vector<RegisterAddressRange> &ranges) {
  // Process each address range for coherent strings
  for (const auto &range : ranges) {
    UInt64 baseAddr = range.baseAddress;
    const auto &registers = range.registers;

    // Skip very small groups
    if (registers.size() < 2)
      continue;

    std::cout << "Address Range: 0x" << std::hex << baseAddr << " - 0x"
              << registers.back().first << std::dec << " (" << registers.size()
              << " registers)" << std::endl;

    // Extract strings from this range
    std::string currentString;
    UInt64 stringStartAddr = 0;
    bool inString = false;

    for (const auto &reg : registers) {
      UInt64 addr = reg.first;
      UInt32 rawValue = reg.second;
      UInt32 hostValue =
          CFSwapInt32LittleToHost(rawValue); // Convert to host endianness
      std::string ascii = interpretAsASCII(hostValue);

      // Also try byte-swapped interpretation for little-endian strings
      std::string swappedAscii =
          interpretAsASCII(CFSwapInt32BigToHost(rawValue));
      if (swappedAscii.length() > ascii.length() ||
          (swappedAscii.length() == ascii.length() &&
           swappedAscii.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJ"
                                          "KLMNOPQRSTUVWXYZ0123456789") >
               ascii.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                                       "NOPQRSTUVWXYZ0123456789"))) {
        // Use the swapped version if it's longer or has more alphanumeric
        // characters
        ascii = swappedAscii;
      }

      if (!ascii.empty()) {
        if (!inString) {
          inString = true;
          stringStartAddr = addr;
        }
        currentString += ascii;
      } else {
        if (inString) {
          // End of string
          if (currentString.length() >= 3) { // Minimum meaningful length
            std::cout << "  String at 0x" << std::hex << stringStartAddr
                      << std::dec << ": \"" << currentString << "\""
                      << std::endl;
          }
          currentString.clear();
          inString = false;
        }
      }
    }

    // Check if we were in a string at the end
    if (inString && currentString.length() >= 3) {
      std::cout << "  String at 0x" << std::hex << stringStartAddr << std::dec
                << ": \"" << currentString << "\"" << std::endl;
    }

    // Also look for patterns where ASCII is split across registers
    std::string combinedAscii;
    UInt64 combinedStartAddr = 0;
    bool inCombinedString = false;

    for (size_t i = 0; i < registers.size(); i++) {
      UInt64 addr = registers[i].first;
      UInt32 rawValue = registers[i].second;
      UInt32 hostValue = CFSwapInt32LittleToHost(rawValue);

      // Try both byte orders for better string detection
      UInt32 swappedValue = CFSwapInt32BigToHost(rawValue);

      // Extract individual bytes and check if they form ASCII when combined
      // Try both byte orders for better string detection
      std::string normalAscii;
      std::string swappedAscii;
      bool foundNormalAscii = false;
      bool foundSwappedAscii = false;

      // Check normal byte order
      for (int bytePos = 0; bytePos < 4; bytePos++) {
        char c = static_cast<char>((hostValue >> (8 * bytePos)) & 0xFF);
        if (c >= 32 && c <= 126) {
          normalAscii += c;
          foundNormalAscii = true;
        }
      }

      // Check swapped byte order
      for (int bytePos = 0; bytePos < 4; bytePos++) {
        char c = static_cast<char>((swappedValue >> (8 * bytePos)) & 0xFF);
        if (c >= 32 && c <= 126) {
          swappedAscii += c;
          foundSwappedAscii = true;
        }
      }

      // Use the better string (more printable characters)
      std::string betterAscii;
      if (foundSwappedAscii && (swappedAscii.length() > normalAscii.length())) {
        betterAscii = swappedAscii;
      } else if (foundNormalAscii) {
        betterAscii = normalAscii;
      }

      // Process the better string
      if (!betterAscii.empty()) {
        if (!inCombinedString) {
          inCombinedString = true;
          combinedStartAddr = addr;
        }
        combinedAscii += betterAscii;
      } else {
        // No ASCII found in either byte order
        if (inCombinedString) {
          // End of string (no more ASCII in this register)
          if (combinedAscii.length() >= 3) {
            std::cout << "  Byte-level string at 0x" << std::hex
                      << combinedStartAddr << std::dec << ": \""
                      << combinedAscii << "\"" << std::endl;
          }
          combinedAscii.clear();
          inCombinedString = false;
        }
      }
    }

    // Check for any remaining combined string
    if (inCombinedString && combinedAscii.length() >= 3) {
      std::cout << "  Byte-level string at 0x" << std::hex << combinedStartAddr
                << std::dec << ": \"" << combinedAscii << "\"" << std::endl;
    }

    std::cout << std::endl;
  }
}

} // namespace FWA::SCANNER
