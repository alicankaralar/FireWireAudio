#include "dice_stream_registers.hpp"
#include "dice_helpers.hpp"       // For DICE_REGISTER constants
#include "endianness_helpers.hpp" // For detectDeviceEndianness, deviceToHostInt32
#include "io_helpers.hpp"         // For safeReadQuadlet, interpretAsASCII
#include "scanner.hpp" // For FireWireDevice, DiceDefines.hpp constants

#include <iomanip> // For std::hex, std::setw, std::setfill
#include <iostream>
#include <map>
#include <string>

#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER {

// Helper to read TX stream parameters using the stream count from device
void readDiceTxStreamRegisters(IOFireWireDeviceInterface **deviceInterface,
                               io_service_t service, FireWireDevice &device,
                               uint64_t discoveredDiceBase, UInt32 generation,
                               uint32_t txStreamSizeQuadlets) {
  // Skip if invalid size or stream count
  if (txStreamSizeQuadlets == 0) {
    std::cerr << "Debug [DICE]: Skipping TX stream register read (invalid size="
              << txStreamSizeQuadlets << ")." << std::endl;
    return;
  }

  if (device.txStreamCount == 0) {
    std::cerr
        << "Debug [DICE]: Skipping TX stream register read (stream count=0)."
        << std::endl;
    return;
  }

  std::cerr << "Info [DICE]: Reading TX streams based on reported count: "
            << device.txStreamCount << std::endl;

  // Find TX Parameter Space Offset (needs to be read first)
  uint64_t txParamSpaceOffsetAddr =
      discoveredDiceBase + DICE_REGISTER_TX_PAR_SPACE_OFF;
  uint32_t txParamSpaceOffsetQuadlets = 0;
  if (device.diceRegisters.count(txParamSpaceOffsetAddr)) {
    txParamSpaceOffsetQuadlets = deviceToHostInt32(
        device.diceRegisters[txParamSpaceOffsetAddr], device.deviceEndianness);
  } else {
    std::cerr << "Warning [DICE]: TX Parameter Space Offset not previously "
                 "read. Cannot read TX stream details."
              << std::endl;
    return;
  }
  uint64_t txParamSpaceBase =
      discoveredDiceBase + (txParamSpaceOffsetQuadlets * 4);
  std::cerr << "Debug [DICE]: TX Parameter Space Base Address: 0x" << std::hex
            << txParamSpaceBase << std::dec << std::endl;

  // Define all registers to read for streams
  std::map<uint64_t, std::string> allTxStreamRegs = {
      {DICE_REGISTER_TX_ISOC_BASE - DICE_REGISTER_TX_BASE, "ISOC"},
      {DICE_REGISTER_TX_NB_AUDIO_BASE - DICE_REGISTER_TX_BASE,
       "Audio Channels"},
      {DICE_REGISTER_TX_MIDI_BASE - DICE_REGISTER_TX_BASE, "MIDI"},
      {DICE_REGISTER_TX_SPEED_BASE - DICE_REGISTER_TX_BASE, "Speed"},
      {DICE_REGISTER_TX_NAMES_BASE - DICE_REGISTER_TX_BASE, "Names Base"},
      {DICE_REGISTER_TX_AC3_CAPABILITIES_BASE - DICE_REGISTER_TX_BASE,
       "AC3 Capabilities"},
      {DICE_REGISTER_TX_AC3_ENABLE_BASE - DICE_REGISTER_TX_BASE, "AC3 Enable"}};

  // Read registers for each stream based on the reported count
  for (uint32_t i = 0; i < device.txStreamCount; ++i) {
    uint64_t streamInstanceOffsetBytes = i * txStreamSizeQuadlets * 4;

    // Reduced logging - only log at higher level
    if (logger_)
      logger_->debug("Reading TX stream [{}]...", i);

    // Read all registers for this stream
    for (const auto &regPair : allTxStreamRegs) {
      uint64_t regRelativeOffset = regPair.first;
      const std::string &regName = regPair.second;
      uint64_t fullAddr =
          txParamSpaceBase + streamInstanceOffsetBytes + regRelativeOffset;
      UInt32 value = 0;

      IOReturn status = FWA::SCANNER::safeReadQuadlet(
          deviceInterface, service, fullAddr, value, generation);
      if (status == kIOReturnSuccess) {
        // Successfully read this register
        device.diceRegisters[fullAddr] = value; // Store raw BE value
        UInt32 swappedValue = deviceToHostInt32(value, device.deviceEndianness);
        std::string ascii = FWA::SCANNER::interpretAsASCII(swappedValue);

        // Reduce log verbosity - only log non-empty ASCII values or at debug
        // level
        if (!ascii.empty()) {
          std::cerr << "Info [DICE]: TX[" << i << "] " << regName << ": '"
                    << ascii << "'" << std::endl;
        }

        // Detailed register info only at debug level
        if (logger_)
          logger_->debug("TX[{}] {} (0x{:x}): 0x{:x}", i, regName, fullAddr,
                         swappedValue);

        // Validate register content based on register type
        if (regName == "ISOC") {
          // ISOC register: Check if value is between 0-63 (inclusive)
          if (swappedValue > 63 &&
              swappedValue !=
                  0xFFFFFFFF) // 0xFFFFFFFF might be used as "unassigned"
          {
            std::cerr << "Warning [DICE]: TX[" << i << "] ISOC value ("
                      << swappedValue << ") is outside valid range (0-63)"
                      << std::endl;
          }
        } else if (regName == "Audio Channels") {
          // NB_AUDIO register: Check if value is non-zero and within reasonable
          // limit
          if (swappedValue == 0) {
            std::cerr << "Warning [DICE]: TX[" << i
                      << "] Audio Channels value is zero" << std::endl;
          } else if (swappedValue > 32) // Reasonable upper limit
          {
            std::cerr << "Warning [DICE]: TX[" << i
                      << "] Audio Channels value (" << swappedValue
                      << ") exceeds reasonable limit (32)" << std::endl;
          }
        } else if (regName == "MIDI") {
          // MIDI register: Check if value is within reasonable limit
          if (swappedValue > 16) // Reasonable upper limit for MIDI channels
          {
            std::cerr << "Warning [DICE]: TX[" << i << "] MIDI value ("
                      << swappedValue << ") exceeds reasonable limit (16)"
                      << std::endl;
          }
        } else if (regName == "Speed") {
          // SPEED register: Check if value matches known FireWire speed
          // constants
          if (swappedValue != 0 && swappedValue != 1 &&
              swappedValue != 2) // S100, S200, S400
          {
            std::cerr
                << "Warning [DICE]: TX[" << i << "] Speed value ("
                << swappedValue
                << ") is not a valid FireWire speed (0=S100, 1=S200, 2=S400)"
                << std::endl;
          }
        } else if (regName == "Names Base") {
          // NAMES_BASE register: Check if value is plausible within DICE
          // address space
          if (swappedValue == 0 ||
              swappedValue > 0x1000000) // Arbitrary large value
          {
            std::cerr << "Warning [DICE]: TX[" << i << "] Names Base value (0x"
                      << std::hex << swappedValue << std::dec
                      << ") seems implausible" << std::endl;
          } else {
            // Convert quadlet offset to absolute address
            uint64_t channelNamesAddr =
                discoveredDiceBase + (static_cast<uint64_t>(swappedValue) * 4);
            std::cerr << "Info [DICE]: TX[" << i << "] Names Base points to 0x"
                      << std::hex << channelNamesAddr << std::dec << std::endl;

            // Store this as a potential channel names address if we don't have
            // one yet
            if (device.channelNamesBaseAddr == DICE_INVALID_OFFSET) {
              device.channelNamesBaseAddr = channelNamesAddr;
              std::cerr
                  << "Info [DICE]: Setting device.channelNamesBaseAddr to 0x"
                  << std::hex << channelNamesAddr << std::dec << " from TX["
                  << i << "] Names Base" << std::endl;
            }
          }
        } else if (regName.find("AC3") != std::string::npos) {
          // AC3_* registers: No specific validation yet, but could be added if
          // patterns are known
        }
      } else {
        std::cerr << "Warning [DICE]: Read failed for TX[" << i << "] "
                  << regName << " (0x" << std::hex << fullAddr
                  << ") (status: " << status << ")" << std::dec << std::endl;
      }
    }
  }
}

// Helper to read RX stream parameters using the stream count from device
void readDiceRxStreamRegisters(IOFireWireDeviceInterface **deviceInterface,
                               io_service_t service, FireWireDevice &device,
                               uint64_t discoveredDiceBase, UInt32 generation,
                               uint32_t rxStreamSizeQuadlets) {
  // Skip if invalid size or stream count
  if (rxStreamSizeQuadlets == 0) {
    std::cerr << "Debug [DICE]: Skipping RX stream register read (invalid size="
              << rxStreamSizeQuadlets << ")." << std::endl;
    return;
  }

  if (device.rxStreamCount == 0) {
    std::cerr
        << "Debug [DICE]: Skipping RX stream register read (stream count=0)."
        << std::endl;
    return;
  }

  std::cerr << "Info [DICE]: Reading RX streams based on reported count: "
            << device.rxStreamCount << std::endl;

  // Find RX Parameter Space Offset (needs to be read first)
  uint64_t rxParamSpaceOffsetAddr =
      discoveredDiceBase + DICE_REGISTER_RX_PAR_SPACE_OFF;
  uint32_t rxParamSpaceOffsetQuadlets = 0;
  if (device.diceRegisters.count(rxParamSpaceOffsetAddr)) {
    rxParamSpaceOffsetQuadlets = deviceToHostInt32(
        device.diceRegisters[rxParamSpaceOffsetAddr], device.deviceEndianness);
  } else {
    std::cerr << "Warning [DICE]: RX Parameter Space Offset not previously "
                 "read. Cannot read RX stream details."
              << std::endl;
    return;
  }
  uint64_t rxParamSpaceBase =
      discoveredDiceBase + (rxParamSpaceOffsetQuadlets * 4);
  std::cerr << "Debug [DICE]: RX Parameter Space Base Address: 0x" << std::hex
            << rxParamSpaceBase << std::dec << std::endl;

  // Define all registers to read for streams
  std::map<uint64_t, std::string> allRxStreamRegs = {
      {DICE_REGISTER_RX_ISOC_BASE - DICE_REGISTER_RX_BASE, "ISOC"},
      {DICE_REGISTER_RX_SEQ_START_BASE - DICE_REGISTER_RX_BASE,
       "Sequence Start"},
      {DICE_REGISTER_RX_NB_AUDIO_BASE - DICE_REGISTER_RX_BASE,
       "Audio Channels"},
      {DICE_REGISTER_RX_MIDI_BASE - DICE_REGISTER_RX_BASE, "MIDI"},
      {DICE_REGISTER_RX_NAMES_BASE - DICE_REGISTER_RX_BASE, "Names Base"},
      {DICE_REGISTER_RX_AC3_CAPABILITIES_BASE - DICE_REGISTER_RX_BASE,
       "AC3 Capabilities"},
      {DICE_REGISTER_RX_AC3_ENABLE_BASE - DICE_REGISTER_RX_BASE, "AC3 Enable"}};

  // Read registers for each stream based on the reported count
  for (uint32_t i = 0; i < device.rxStreamCount; ++i) {
    uint64_t streamInstanceOffsetBytes = i * rxStreamSizeQuadlets * 4;

    // Reduced logging - only log at higher level
    if (logger_)
      logger_->debug("Reading RX stream [{}]...", i);

    // Read all registers for this stream
    for (const auto &regPair : allRxStreamRegs) {
      uint64_t regRelativeOffset = regPair.first;
      const std::string &regName = regPair.second;
      uint64_t fullAddr =
          rxParamSpaceBase + streamInstanceOffsetBytes + regRelativeOffset;
      UInt32 value = 0;

      IOReturn status = FWA::SCANNER::safeReadQuadlet(
          deviceInterface, service, fullAddr, value, generation);
      if (status == kIOReturnSuccess) {
        // Successfully read this register
        device.diceRegisters[fullAddr] = value; // Store raw BE value
        UInt32 swappedValue = deviceToHostInt32(value, device.deviceEndianness);
        std::string ascii = FWA::SCANNER::interpretAsASCII(swappedValue);

        // Reduce log verbosity - only log non-empty ASCII values or at debug
        // level
        if (!ascii.empty()) {
          std::cerr << "Info [DICE]: RX[" << i << "] " << regName << ": '"
                    << ascii << "'" << std::endl;
        }

        // Detailed register info only at debug level
        if (logger_)
          logger_->debug("RX[{}] {} (0x{:x}): 0x{:x}", i, regName, fullAddr,
                         swappedValue);

        // Validate register content based on register type
        if (regName == "ISOC") {
          // ISOC register: Check if value is between 0-63 (inclusive)
          if (swappedValue > 63 &&
              swappedValue !=
                  0xFFFFFFFF) // 0xFFFFFFFF might be used as "unassigned"
          {
            std::cerr << "Warning [DICE]: RX[" << i << "] ISOC value ("
                      << swappedValue << ") is outside valid range (0-63)"
                      << std::endl;
          }
        } else if (regName == "Audio Channels") {
          // NB_AUDIO register: Check if value is non-zero and within reasonable
          // limit
          if (swappedValue == 0) {
            std::cerr << "Warning [DICE]: RX[" << i
                      << "] Audio Channels value is zero" << std::endl;
          } else if (swappedValue > 32) // Reasonable upper limit
          {
            std::cerr << "Warning [DICE]: RX[" << i
                      << "] Audio Channels value (" << swappedValue
                      << ") exceeds reasonable limit (32)" << std::endl;
          }
        } else if (regName == "MIDI") {
          // MIDI register: Check if value is within reasonable limit
          if (swappedValue > 16) // Reasonable upper limit for MIDI channels
          {
            std::cerr << "Warning [DICE]: RX[" << i << "] MIDI value ("
                      << swappedValue << ") exceeds reasonable limit (16)"
                      << std::endl;
          }
        } else if (regName == "Sequence Start") {
          // SEQ_START register: No specific validation criteria yet
        } else if (regName == "Names Base") {
          // NAMES_BASE register: Check if value is plausible within DICE
          // address space
          if (swappedValue == 0 ||
              swappedValue > 0x1000000) // Arbitrary large value
          {
            std::cerr << "Warning [DICE]: RX[" << i << "] Names Base value (0x"
                      << std::hex << swappedValue << std::dec
                      << ") seems implausible" << std::endl;
          } else {
            // Convert quadlet offset to absolute address
            uint64_t channelNamesAddr =
                discoveredDiceBase + (static_cast<uint64_t>(swappedValue) * 4);
            std::cerr << "Info [DICE]: RX[" << i << "] Names Base points to 0x"
                      << std::hex << channelNamesAddr << std::dec << std::endl;

            // Store this as a potential channel names address if we don't have
            // one yet
            if (device.channelNamesBaseAddr == DICE_INVALID_OFFSET) {
              device.channelNamesBaseAddr = channelNamesAddr;
              std::cerr
                  << "Info [DICE]: Setting device.channelNamesBaseAddr to 0x"
                  << std::hex << channelNamesAddr << std::dec << " from RX["
                  << i << "] Names Base" << std::endl;
            }
          }
        } else if (regName.find("AC3") != std::string::npos) {
          // AC3_* registers: No specific validation yet, but could be added if
          // patterns are known
        }
      } else {
        std::cerr << "Warning [DICE]: Read failed for RX[" << i << "] "
                  << regName << " (0x" << std::hex << fullAddr
                  << ") (status: " << status << ")" << std::dec << std::endl;
      }
    }
  }
}

} // namespace FWA::SCANNER
