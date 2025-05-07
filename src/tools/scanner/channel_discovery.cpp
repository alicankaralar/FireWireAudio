#include "channel_discovery.hpp"
#include "endianness_helpers.hpp" // For detectDeviceEndianness, deviceToHostInt32
#include "io_helpers.hpp"         // For safeReadQuadlet
#include "scanner.hpp"         // For FireWireDevice, DiceDefines.hpp constants
#include "scanner_defines.hpp" // For DICE_REL_OFFSET constants
#include "string_extraction.hpp" // For StringMatch, extractStringsFromMemory

#include <iomanip> // For std::hex, std::dec
#include <iostream>
#include <map>
#include <regex> // For std::regex, std::smatch

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost

namespace FWA::SCANNER {
// Forward declaration of internal function from dice_helpers.cpp
bool discoverDiceBaseAddressesInternal(
    IOFireWireDeviceInterface **deviceInterface, io_service_t service,
    UInt32 generation, uint64_t &globalBase, uint64_t &txBase, uint64_t &rxBase,
    std::string &method);

uint64_t
discoverChannelNamesAddress(IOFireWireDeviceInterface **deviceInterface,
                            io_service_t service, UInt32 generation) {
  std::cerr << "Debug [Utils]: Attempting to discover channel names address..."
            << std::endl;

  // Create a temporary device struct to detect endianness
  FireWireDevice tempDevice;

  // Detect the device's endianness
  DeviceEndianness deviceEndianness =
      detectDeviceEndianness(deviceInterface, service, tempDevice, generation);
  std::cerr << "Debug [Utils]: Detected device endianness: "
            << (deviceEndianness == DeviceEndianness::DEVICE_BIG_ENDIAN
                    ? "BIG_ENDIAN"
                : deviceEndianness == DeviceEndianness::DEVICE_LITTLE_ENDIAN
                    ? "LITTLE_ENDIAN"
                    : "UNKNOWN_ENDIAN")
            << std::endl;

  // Step 1: Check for NAMES_BASE pointers from TX/RX stream registers
  std::vector<uint64_t> namesBasePointers;

  // First, try to get the DICE base addresses
  uint64_t globalBase = DICE_INVALID_OFFSET;
  uint64_t txBase = DICE_INVALID_OFFSET;
  uint64_t rxBase = DICE_INVALID_OFFSET;
  std::string method;

  // Use the same discovery logic as in dice_helpers.cpp
  if (FWA::SCANNER::discoverDiceBaseAddressesInternal(deviceInterface, service,
                                                      generation, globalBase,
                                                      txBase, rxBase, method)) {
    std::cerr << "Debug [Utils]: Successfully discovered DICE bases using "
              << method << std::endl;

    // Read TX stream count and size
    uint32_t txStreamCount = 0;
    uint32_t txStreamSizeQuadlets = 256; // Default size

    // Read TX stream count
    if (txBase != DICE_INVALID_OFFSET) {
      uint64_t txCountAddr = txBase + FWA::Scanner::DICE_REL_OFFSET_TX_NB_TX;
      UInt32 rawTxCount = 0;
      if (safeReadQuadlet(deviceInterface, service, txCountAddr, rawTxCount,
                          generation) == kIOReturnSuccess) {
        txStreamCount = deviceToHostInt32(rawTxCount, deviceEndianness);
        std::cerr << "Debug [Utils]: Found " << txStreamCount << " TX streams"
                  << std::endl;

        // Read TX stream size
        uint64_t txSizeAddr = txBase + FWA::Scanner::DICE_REL_OFFSET_TX_SZ_TX;
        UInt32 rawTxSize = 0;
        if (safeReadQuadlet(deviceInterface, service, txSizeAddr, rawTxSize,
                            generation) == kIOReturnSuccess) {
          txStreamSizeQuadlets = deviceToHostInt32(rawTxSize, deviceEndianness);
          std::cerr << "Debug [Utils]: TX stream size: " << txStreamSizeQuadlets
                    << " quadlets" << std::endl;
        }
      }

      // Read TX Parameter Space Offset
      uint64_t txParamSpaceOffsetAddr =
          globalBase + DICE_REGISTER_TX_PAR_SPACE_OFF;
      UInt32 txParamSpaceOffsetQuadlets = 0;
      if (safeReadQuadlet(deviceInterface, service, txParamSpaceOffsetAddr,
                          txParamSpaceOffsetQuadlets,
                          generation) == kIOReturnSuccess) {
        txParamSpaceOffsetQuadlets =
            deviceToHostInt32(txParamSpaceOffsetQuadlets, deviceEndianness);
        uint64_t txParamSpaceBase =
            globalBase + (txParamSpaceOffsetQuadlets * 4);

        // Check each TX stream for NAMES_BASE
        for (uint32_t i = 0; i < txStreamCount && i < 8;
             i++) // Limit to 8 streams for safety
        {
          uint64_t streamInstanceOffsetBytes = i * txStreamSizeQuadlets * 4;
          uint64_t namesBaseAddr = txParamSpaceBase +
                                   streamInstanceOffsetBytes +
                                   DICE_REGISTER_TX_NAMES_BASE;

          UInt32 namesBaseValue = 0;
          if (safeReadQuadlet(deviceInterface, service, namesBaseAddr,
                              namesBaseValue, generation) == kIOReturnSuccess) {
            uint32_t namesBaseOffset = CFSwapInt32LittleToHost(namesBaseValue);
            if (namesBaseOffset != 0) {
              // Convert quadlet offset to absolute address
              uint64_t channelNamesAddr = globalBase + (namesBaseOffset * 4);
              std::cerr << "Debug [Utils]: Found TX[" << i
                        << "] NAMES_BASE pointer: 0x" << std::hex
                        << channelNamesAddr << std::dec << std::endl;
              namesBasePointers.push_back(channelNamesAddr);
            }
          }
        }
      }
    }

    // Read RX stream count and size
    uint32_t rxStreamCount = 0;
    uint32_t rxStreamSizeQuadlets = 256; // Default size

    // Read RX stream count
    if (rxBase != DICE_INVALID_OFFSET) {
      uint64_t rxCountAddr = rxBase + FWA::Scanner::DICE_REL_OFFSET_RX_NB_RX;
      UInt32 rawRxCount = 0;
      if (safeReadQuadlet(deviceInterface, service, rxCountAddr, rawRxCount,
                          generation) == kIOReturnSuccess) {
        rxStreamCount = deviceToHostInt32(rawRxCount, deviceEndianness);
        std::cerr << "Debug [Utils]: Found " << rxStreamCount << " RX streams"
                  << std::endl;

        // Read RX stream size
        uint64_t rxSizeAddr = rxBase + FWA::Scanner::DICE_REL_OFFSET_RX_SZ_RX;
        UInt32 rawRxSize = 0;
        if (safeReadQuadlet(deviceInterface, service, rxSizeAddr, rawRxSize,
                            generation) == kIOReturnSuccess) {
          rxStreamSizeQuadlets = CFSwapInt32LittleToHost(rawRxSize);
          std::cerr << "Debug [Utils]: RX stream size: " << rxStreamSizeQuadlets
                    << " quadlets" << std::endl;
        }
      }

      // Read RX Parameter Space Offset
      uint64_t rxParamSpaceOffsetAddr =
          globalBase + DICE_REGISTER_RX_PAR_SPACE_OFF;
      UInt32 rxParamSpaceOffsetQuadlets = 0;
      if (safeReadQuadlet(deviceInterface, service, rxParamSpaceOffsetAddr,
                          rxParamSpaceOffsetQuadlets,
                          generation) == kIOReturnSuccess) {
        rxParamSpaceOffsetQuadlets =
            deviceToHostInt32(rxParamSpaceOffsetQuadlets, deviceEndianness);
        uint64_t rxParamSpaceBase =
            globalBase + (rxParamSpaceOffsetQuadlets * 4);

        // Check each RX stream for NAMES_BASE
        for (uint32_t i = 0; i < rxStreamCount && i < 8;
             i++) // Limit to 8 streams for safety
        {
          uint64_t streamInstanceOffsetBytes = i * rxStreamSizeQuadlets * 4;
          uint64_t namesBaseAddr = rxParamSpaceBase +
                                   streamInstanceOffsetBytes +
                                   DICE_REGISTER_RX_NAMES_BASE;

          UInt32 namesBaseValue = 0;
          if (safeReadQuadlet(deviceInterface, service, namesBaseAddr,
                              namesBaseValue, generation) == kIOReturnSuccess) {
            uint32_t namesBaseOffset = CFSwapInt32LittleToHost(namesBaseValue);
            if (namesBaseOffset != 0) {
              // Convert quadlet offset to absolute address
              uint64_t channelNamesAddr = globalBase + (namesBaseOffset * 4);
              std::cerr << "Debug [Utils]: Found RX[" << i
                        << "] NAMES_BASE pointer: 0x" << std::hex
                        << channelNamesAddr << std::dec << std::endl;
              namesBasePointers.push_back(channelNamesAddr);
            }
          }
        }
      }
    }
  }

  // Step 2: Check EAP structures for channel name pointers
  // Try to read EAP Current Config Space Offset
  uint64_t eapCurrCfgOffsetAddr = DICE_EAP_CURR_CFG_SPACE_ADDR;
  UInt32 eapCurrCfgOffsetQuadlets = 0;
  if (safeReadQuadlet(deviceInterface, service, eapCurrCfgOffsetAddr,
                      eapCurrCfgOffsetQuadlets,
                      generation) == kIOReturnSuccess) {
    uint64_t eapCurrCfgBaseAddr =
        DICE_REGISTER_BASE +
        (deviceToHostInt32(eapCurrCfgOffsetQuadlets, deviceEndianness) * 4);
    std::cerr << "Debug [Utils]: Found EAP Current Config Space at 0x"
              << std::hex << eapCurrCfgBaseAddr << std::dec << std::endl;

    // Check LOW, MID, and HIGH stream configs
    std::vector<uint64_t> streamConfigOffsets = {DICE_EAP_CURRCFG_LOW_STREAM,
                                                 DICE_EAP_CURRCFG_MID_STREAM,
                                                 DICE_EAP_CURRCFG_HIGH_STREAM};

    for (uint64_t offset : streamConfigOffsets) {
      uint64_t configBlockBase = eapCurrCfgBaseAddr + offset;

      // Read the first few quadlets to check for channel name pointers
      for (int i = 0; i < 16; i++) // Check first 16 quadlets
      {
        UInt32 value = 0;
        if (safeReadQuadlet(deviceInterface, service, configBlockBase + (i * 4),
                            value, generation) == kIOReturnSuccess) {
          // If this looks like a valid pointer (non-zero and within reasonable
          // range)
          uint32_t hostValue = CFSwapInt32LittleToHost(value);
          if (hostValue != 0 &&
              hostValue < 0x100000) // Reasonable quadlet offset
          {
            uint64_t potentialAddr = DICE_REGISTER_BASE + (hostValue * 4);
            namesBasePointers.push_back(potentialAddr);
            std::cerr
                << "Debug [Utils]: Found potential EAP channel name pointer: 0x"
                << std::hex << potentialAddr << std::dec << std::endl;
          }
        }
      }
    }
  }

  // Step 3: Validate each potential NAMES_BASE pointer
  for (uint64_t addr : namesBasePointers) {
    std::cerr << "Debug [Utils]: Validating potential channel names address 0x"
              << std::hex << addr << std::dec << std::endl;

    // Read a block of memory at this address
    const int BLOCK_SIZE = 256; // 256 quadlets = 1024 bytes
    std::map<uint64_t, uint32_t> memoryBlock;
    bool validBlock = true;

    for (int i = 0; i < BLOCK_SIZE; i++) {
      UInt32 value = 0;
      IOReturn status = safeReadQuadlet(deviceInterface, service,
                                        addr + (i * 4), value, generation);
      if (status != kIOReturnSuccess) {
        if (i == 0) {
          // If we can't read the first quadlet, this address is invalid
          validBlock = false;
          break;
        }
        // Otherwise, we've read some quadlets, so continue with what we have
        break;
      }
      memoryBlock[addr + (i * 4)] = value;
    }

    if (!validBlock || memoryBlock.empty()) {
      std::cerr << "Debug [Utils]: Address 0x" << std::hex << addr << std::dec
                << " is not readable" << std::endl;
      continue;
    }

    // Extract strings from the memory block
    std::vector<StringMatch> strings =
        extractStringsFromMemory(memoryBlock, deviceEndianness);

    // Check if any of the strings match channel name patterns
    // Enhanced regex pattern for channel name detection in dynamic discovery
    std::regex channelPattern(
        R"((?:OUT|OUTPUT|IN|INPUT)[\s\-_]*(?:ST|STEREO)?[\s\-_]*(?:CH)?\d+(?:[LR])?)");
    bool foundChannelNames = false;

    for (const auto &match : strings) {
      std::smatch regexMatch;
      if (std::regex_search(match.text, regexMatch, channelPattern)) {
        std::cerr << "Debug [Utils]: Found channel name pattern '"
                  << regexMatch.str() << "' at address 0x" << std::hex
                  << match.address << std::dec << std::endl;
        foundChannelNames = true;
        break;
      }
    }

    if (foundChannelNames) {
      std::cerr
          << "Debug [Utils]: Dynamically discovered channel names at address 0x"
          << std::hex << addr << std::dec << std::endl;
      return addr;
    }
  }

  // Step 4: Fall back to known hardcoded addresses if dynamic discovery failed
  std::cerr << "Debug [Utils]: Dynamic discovery failed, falling back to "
               "hardcoded addresses..."
            << std::endl;

  // Known potential addresses for channel names
  std::vector<uint64_t> potentialAddresses = {
      DICE_CHANNEL_NAMES_ADDR_1, // Known address from previous scans
      DICE_CHANNEL_NAMES_ADDR_2, // Channel Configuration area
      DICE_CHANNEL_NAMES_ADDR_3, // Another potential area
      DICE_CHANNEL_NAMES_ADDR_4  // Another potential area
  };

  // Try each address and look for channel name patterns
  for (uint64_t addr : potentialAddresses) {
    std::cerr << "Debug [Utils]: Checking address 0x" << std::hex << addr
              << std::dec << " for channel names..." << std::endl;

    // Read a block of memory at this address
    const int BLOCK_SIZE = 256; // 256 quadlets = 1024 bytes
    std::map<uint64_t, uint32_t> memoryBlock;
    bool validBlock = true;

    for (int i = 0; i < BLOCK_SIZE; i++) {
      UInt32 value = 0;
      IOReturn status = safeReadQuadlet(deviceInterface, service,
                                        addr + (i * 4), value, generation);
      if (status != kIOReturnSuccess) {
        std::cerr << "Debug [Utils]: Failed to read quadlet at 0x" << std::hex
                  << addr + (i * 4) << std::dec << std::endl;
        if (i == 0) {
          // If we can't read the first quadlet, this address is invalid
          validBlock = false;
          break;
        }
        // Otherwise, we've read some quadlets, so continue with what we have
        break;
      }
      memoryBlock[addr + (i * 4)] = value;
    }

    if (!validBlock || memoryBlock.empty()) {
      std::cerr << "Debug [Utils]: Address 0x" << std::hex << addr << std::dec
                << " is not readable" << std::endl;
      continue;
    }

    // Extract strings from the memory block
    std::vector<StringMatch> strings = extractStringsFromMemory(memoryBlock);

    // Check if any of the strings match channel name patterns
    // Enhanced regex pattern for channel name detection in fallback addresses
    std::regex channelPattern(
        R"((?:OUT|OUTPUT|IN|INPUT)[\s\-_]*(?:ST|STEREO)?[\s\-_]*(?:CH)?\d+(?:[LR])?)");
    bool foundChannelNames = false;

    for (const auto &match : strings) {
      std::smatch regexMatch;
      if (std::regex_search(match.text, regexMatch, channelPattern)) {
        std::cerr << "Debug [Utils]: Found channel name pattern '"
                  << regexMatch.str() << "' at address 0x" << std::hex
                  << match.address << std::dec << std::endl;
        foundChannelNames = true;
        break;
      }
    }

    if (foundChannelNames) {
      std::cerr << "Debug [Utils]: Discovered channel names at address 0x"
                << std::hex << addr << std::dec << std::endl;
      return addr;
    }
  }

  // If no address was found, return the known address as a fallback
  std::cerr << "Debug [Utils]: No channel names address discovered, using "
               "fallback address 0x"
            << std::hex << DICE_CHANNEL_NAMES_ADDR_1 << std::dec << std::endl;
  return DICE_CHANNEL_NAMES_ADDR_1;
}

} // namespace FWA::SCANNER
