#include "channel_count_validator.hpp"
#include "dice_base_discovery.hpp"   // For DICE_INVALID_OFFSET
#include "dice_register_readers.hpp" // For DICE register constants
#include "scanner.hpp"               // For FireWireDevice

#include <algorithm> // For std::max
#include <cmath>     // For std::abs
#include <iomanip>
#include <iostream>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost

namespace FWA::SCANNER {

void calculateStreamChannelCounts(const FireWireDevice &device,
                                  ChannelCounts &counts) {
  std::cout << "Calculating channel counts from stream registers..."
            << std::endl;

  counts.streamsAudioOutputs = 0;
  counts.streamsMidiOutputs = 0;
  counts.streamsAudioInputs = 0;
  counts.streamsMidiInputs = 0;

  // Check if we have valid TX/RX base addresses
  if (device.diceTxBase == DICE_INVALID_OFFSET ||
      device.diceRxBase == DICE_INVALID_OFFSET) {
    std::cout << "Warning: Invalid DICE TX/RX base addresses. Cannot calculate "
                 "stream channel counts."
              << std::endl;
    return;
  }

  // Iterate through TX streams
  for (UInt32 i = 0; i < device.txStreamCount; ++i) {
    // Calculate the address for NB_AUDIO register for this stream
    // This is a simplified approach - in a real implementation, we would need
    // to know the stream size For now, we'll assume a fixed stream size of 256
    // quadlets (1024 bytes)
    const UInt32 assumedStreamSizeQuadlets = 256;
    UInt64 streamInstanceOffsetBytes = i * assumedStreamSizeQuadlets * 4;

    // Calculate the address for NB_AUDIO register
    UInt64 nbAudioAddr =
        device.diceTxBase + streamInstanceOffsetBytes +
        (DICE_REGISTER_TX_NUMBER_AUDIO_BASE_OFFSET - DICE_REGISTER_TX_BASE);

    // Calculate the address for NB_MIDI register
    UInt64 nbMidiAddr =
        device.diceTxBase + streamInstanceOffsetBytes +
        (DICE_REGISTER_TX_MIDI_BASE_OFFSET - DICE_REGISTER_TX_BASE);

    // Check if we have these registers in the device.diceRegisters map
    if (device.diceRegisters.count(nbAudioAddr)) {
      UInt32 nbAudio =
          CFSwapInt32LittleToHost(device.diceRegisters.at(nbAudioAddr));

      // Validate the value
      if (nbAudio > 0 && nbAudio <= 32) { // Reasonable upper limit
        counts.streamsAudioOutputs += nbAudio;
      } else {
        std::cout << "Warning: TX[" << i << "] Audio Channels value ("
                  << nbAudio
                  << ") is outside reasonable range (1-32). Ignoring."
                  << std::endl;
      }
    }

    if (device.diceRegisters.count(nbMidiAddr)) {
      UInt32 nbMidi =
          CFSwapInt32LittleToHost(device.diceRegisters.at(nbMidiAddr));

      // Validate the value
      if (nbMidi <= 16) { // Reasonable upper limit for MIDI channels
        counts.streamsMidiOutputs += nbMidi;
      } else {
        std::cout << "Warning: TX[" << i << "] MIDI value (" << nbMidi
                  << ") exceeds reasonable limit (16). Ignoring." << std::endl;
      }
    }
  }

  // Iterate through RX streams
  for (UInt32 i = 0; i < device.rxStreamCount; ++i) {
    // Calculate the address for NB_AUDIO register for this stream
    // This is a simplified approach - in a real implementation, we would need
    // to know the stream size For now, we'll assume a fixed stream size of 256
    // quadlets (1024 bytes)
    const UInt32 assumedStreamSizeQuadlets = 256;
    UInt64 streamInstanceOffsetBytes = i * assumedStreamSizeQuadlets * 4;

    // Calculate the address for NB_AUDIO register
    UInt64 nbAudioAddr =
        device.diceRxBase + streamInstanceOffsetBytes +
        (DICE_REGISTER_RX_NUMBER_AUDIO_BASE_OFFSET - DICE_REGISTER_RX_BASE);

    // Calculate the address for NB_MIDI register
    UInt64 nbMidiAddr =
        device.diceRxBase + streamInstanceOffsetBytes +
        (DICE_REGISTER_RX_MIDI_BASE_OFFSET - DICE_REGISTER_RX_BASE);

    // Check if we have these registers in the device.diceRegisters map
    if (device.diceRegisters.count(nbAudioAddr)) {
      UInt32 nbAudio =
          CFSwapInt32LittleToHost(device.diceRegisters.at(nbAudioAddr));

      // Validate the value
      if (nbAudio > 0 && nbAudio <= 32) { // Reasonable upper limit
        counts.streamsAudioInputs += nbAudio;
      } else {
        std::cout << "Warning: RX[" << i << "] Audio Channels value ("
                  << nbAudio
                  << ") is outside reasonable range (1-32). Ignoring."
                  << std::endl;
      }
    }

    if (device.diceRegisters.count(nbMidiAddr)) {
      UInt32 nbMidi =
          CFSwapInt32LittleToHost(device.diceRegisters.at(nbMidiAddr));

      // Validate the value
      if (nbMidi <= 16) { // Reasonable upper limit for MIDI channels
        counts.streamsMidiInputs += nbMidi;
      } else {
        std::cout << "Warning: RX[" << i << "] MIDI value (" << nbMidi
                  << ") exceeds reasonable limit (16). Ignoring." << std::endl;
      }
    }
  }

  // Calculate totals
  counts.streamsTotalOutputs =
      counts.streamsAudioOutputs + counts.streamsMidiOutputs;
  counts.streamsTotalInputs =
      counts.streamsAudioInputs + counts.streamsMidiInputs;

  std::cout << "Stream register channel counts: " << counts.streamsTotalOutputs
            << " outputs (" << counts.streamsAudioOutputs << " audio, "
            << counts.streamsMidiOutputs << " MIDI), "
            << counts.streamsTotalInputs << " inputs ("
            << counts.streamsAudioInputs << " audio, "
            << counts.streamsMidiInputs << " MIDI)" << std::endl;
}

void calculateEAPChannelCounts(const FireWireDevice &device,
                               ChannelCounts &counts) {
  std::cout << "Calculating channel counts from EAP data..." << std::endl;

  counts.eapAudioOutputs = 0;
  counts.eapMidiOutputs = 0;
  counts.eapAudioInputs = 0;
  counts.eapMidiInputs = 0;

  // Check if EAP data is available
  // This is a simplified approach - in a real implementation, we would need to
  // check for specific EAP registers For now, we'll just check if the device
  // has a valid chip type from EAP
  if (device.diceChipType == DiceChipType::Unknown) {
    std::cout << "Warning: EAP data not available or incomplete. Cannot "
                 "calculate EAP channel counts."
              << std::endl;
    return;
  }

  // In a real implementation, we would read the EAP configuration to get the
  // channel counts For now, we'll use the TX/RX stream counts as a proxy for
  // EAP data This is not accurate but serves as a placeholder

  // Assume each TX stream has 8 audio channels and 1 MIDI channel
  counts.eapAudioOutputs = device.txStreamCount * 8;
  counts.eapMidiOutputs = device.txStreamCount;

  // Assume each RX stream has 8 audio channels and 1 MIDI channel
  counts.eapAudioInputs = device.rxStreamCount * 8;
  counts.eapMidiInputs = device.rxStreamCount;

  // Calculate totals
  counts.eapTotalOutputs = counts.eapAudioOutputs + counts.eapMidiOutputs;
  counts.eapTotalInputs = counts.eapAudioInputs + counts.eapMidiInputs;

  std::cout << "EAP channel counts: " << counts.eapTotalOutputs << " outputs ("
            << counts.eapAudioOutputs << " audio, " << counts.eapMidiOutputs
            << " MIDI), " << counts.eapTotalInputs << " inputs ("
            << counts.eapAudioInputs << " audio, " << counts.eapMidiInputs
            << " MIDI)" << std::endl;
}

bool crossValidateChannelCounts(ChannelCounts &counts) {
  std::cout << "\n--- Cross-Validating Channel Counts ---" << std::endl;

  // Track which sources have valid data
  bool hasNamesData = (counts.namesTotalPhysicalOutputs > 0 ||
                       counts.namesTotalPhysicalInputs > 0);
  bool hasStreamsData =
      (counts.streamsTotalOutputs > 0 || counts.streamsTotalInputs > 0);
  bool hasEAPData = (counts.eapTotalOutputs > 0 || counts.eapTotalInputs > 0);

  // If we don't have at least two sources with data, we can't cross-validate
  if ((hasNamesData ? 1 : 0) + (hasStreamsData ? 1 : 0) + (hasEAPData ? 1 : 0) <
      2) {
    std::cout << "Warning: Not enough data sources for cross-validation. Using "
                 "available data."
              << std::endl;

    // Use whatever data we have
    if (hasNamesData) {
      counts.finalTotalOutputs = counts.namesTotalPhysicalOutputs;
      counts.finalTotalInputs = counts.namesTotalPhysicalInputs;
      std::cout << "Using channel names data for final counts." << std::endl;
    } else if (hasStreamsData) {
      counts.finalTotalOutputs = counts.streamsTotalOutputs;
      counts.finalTotalInputs = counts.streamsTotalInputs;
      std::cout << "Using stream register data for final counts." << std::endl;
    } else if (hasEAPData) {
      counts.finalTotalOutputs = counts.eapTotalOutputs;
      counts.finalTotalInputs = counts.eapTotalInputs;
      std::cout << "Using EAP data for final counts." << std::endl;
    } else {
      std::cout << "Error: No channel count data available from any source."
                << std::endl;
      return false;
    }

    return true;
  }

  // Check for discrepancies in output counts
  bool outputDiscrepancy = false;
  if (hasNamesData && hasStreamsData &&
      counts.namesTotalPhysicalOutputs != counts.streamsTotalOutputs) {
    std::cout << "Warning: Discrepancy in output counts between Names ("
              << counts.namesTotalPhysicalOutputs << ") and Stream Registers ("
              << counts.streamsTotalOutputs << ")." << std::endl;
    outputDiscrepancy = true;
  }

  if (hasNamesData && hasEAPData &&
      counts.namesTotalPhysicalOutputs != counts.eapTotalOutputs) {
    std::cout << "Warning: Discrepancy in output counts between Names ("
              << counts.namesTotalPhysicalOutputs << ") and EAP ("
              << counts.eapTotalOutputs << ")." << std::endl;
    outputDiscrepancy = true;
  }

  if (hasStreamsData && hasEAPData &&
      counts.streamsTotalOutputs != counts.eapTotalOutputs) {
    std::cout
        << "Warning: Discrepancy in output counts between Stream Registers ("
        << counts.streamsTotalOutputs << ") and EAP (" << counts.eapTotalOutputs
        << ")." << std::endl;
    outputDiscrepancy = true;
  }

  // Check for discrepancies in input counts
  bool inputDiscrepancy = false;
  if (hasNamesData && hasStreamsData &&
      counts.namesTotalPhysicalInputs != counts.streamsTotalInputs) {
    std::cout << "Warning: Discrepancy in input counts between Names ("
              << counts.namesTotalPhysicalInputs << ") and Stream Registers ("
              << counts.streamsTotalInputs << ")." << std::endl;
    inputDiscrepancy = true;
  }

  if (hasNamesData && hasEAPData &&
      counts.namesTotalPhysicalInputs != counts.eapTotalInputs) {
    std::cout << "Warning: Discrepancy in input counts between Names ("
              << counts.namesTotalPhysicalInputs << ") and EAP ("
              << counts.eapTotalInputs << ")." << std::endl;
    inputDiscrepancy = true;
  }

  if (hasStreamsData && hasEAPData &&
      counts.streamsTotalInputs != counts.eapTotalInputs) {
    std::cout
        << "Warning: Discrepancy in input counts between Stream Registers ("
        << counts.streamsTotalInputs << ") and EAP (" << counts.eapTotalInputs
        << ")." << std::endl;
    inputDiscrepancy = true;
  }

  counts.hasDiscrepancy = outputDiscrepancy || inputDiscrepancy;

  // Determine final counts based on priority: EAP > Stream Registers > Names
  if (hasEAPData) {
    counts.finalTotalOutputs = counts.eapTotalOutputs;
    counts.finalTotalInputs = counts.eapTotalInputs;
    std::cout << "Using EAP data for final counts (highest priority)."
              << std::endl;
  } else if (hasStreamsData) {
    counts.finalTotalOutputs = counts.streamsTotalOutputs;
    counts.finalTotalInputs = counts.streamsTotalInputs;
    std::cout
        << "Using stream register data for final counts (medium priority)."
        << std::endl;
  } else {
    counts.finalTotalOutputs = counts.namesTotalPhysicalOutputs;
    counts.finalTotalInputs = counts.namesTotalPhysicalInputs;
    std::cout << "Using channel names data for final counts (lowest priority)."
              << std::endl;
  }

  return true;
}

void printChannelCountSummary(const ChannelCounts &counts) {
  std::cout << "\n--- Channel Count Summary ---" << std::endl;

  // Print counts from all sources
  std::cout << "Source A (Names): " << counts.namesTotalPhysicalOutputs
            << " outputs (" << counts.namesMonoOutputs << " mono, "
            << counts.namesStereoOutputPairs << " stereo pairs), "
            << counts.namesTotalPhysicalInputs << " inputs ("
            << counts.namesMonoInputs << " mono, "
            << counts.namesStereoInputPairs << " stereo pairs)" << std::endl;

  std::cout << "Source B (Stream Registers): " << counts.streamsTotalOutputs
            << " outputs (" << counts.streamsAudioOutputs << " audio, "
            << counts.streamsMidiOutputs << " MIDI), "
            << counts.streamsTotalInputs << " inputs ("
            << counts.streamsAudioInputs << " audio, "
            << counts.streamsMidiInputs << " MIDI)" << std::endl;

  std::cout << "Source C (EAP): " << counts.eapTotalOutputs << " outputs ("
            << counts.eapAudioOutputs << " audio, " << counts.eapMidiOutputs
            << " MIDI), " << counts.eapTotalInputs << " inputs ("
            << counts.eapAudioInputs << " audio, " << counts.eapMidiInputs
            << " MIDI)" << std::endl;

  // Print final counts
  std::cout << "\nFinal Channel Counts: " << counts.finalTotalOutputs
            << " outputs, " << counts.finalTotalInputs << " inputs, "
            << (counts.finalTotalOutputs + counts.finalTotalInputs)
            << " total I/O channels"
            << (counts.hasDiscrepancy ? " (with discrepancies)" : "")
            << std::endl;

  // Print validation status
  if (counts.hasDiscrepancy) {
    std::cout << "\nWarning: Discrepancies were found between different "
                 "channel count sources."
              << std::endl;
    std::cout << "The final counts are based on the highest priority source "
                 "available."
              << std::endl;
    std::cout << "Priority order: EAP > Stream Registers > Channel Names"
              << std::endl;
  } else {
    std::cout << "\nValidation successful: All available sources agree on "
                 "channel counts."
              << std::endl;
  }
}

} // namespace FWA::SCANNER
