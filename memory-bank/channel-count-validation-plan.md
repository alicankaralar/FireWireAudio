---
**STATUS UPDATE (2025-05-06):** The proposals in this plan have been largely implemented as part of the `firewire_scanner` tool enhancements.
The scanner now incorporates:
*   Refined regex for name analysis and boundary detection (Section 2).
*   Cross-validation of channel counts using multiple sources (Stream Config Sums, EAP data, and Name analysis) (Section 3).
*   Improved logging and reporting of count discrepancies.
These changes contribute to a more robust channel count validation system.
---
# Plan: Validating Channel Counts in firewire_scanner

This plan outlines the analysis of the current channel counting logic in the `firewire_scanner` tool and proposes a strategy for improving validation using multiple data sources and refined regex usage.

## 1. Analysis of Current Channel Counting Logic

The current channel count information is derived from multiple sources across different files:

*   **`src/tools/scanner/utils_explore_channels.cpp` (`exploreChannelNamesArea`):**
    *   **Mechanism:** Scans a memory region (dynamically discovered) for ASCII strings. Applies specific regex patterns (`OUTPUT CH#`, `INPUT CH#`, `OUTPUT ST CH#LR`, `INPUT ST CH#LR`) to these strings.
    *   **Counting:** Counts are derived *directly* from the number of unique strings matching these patterns. `std::set` is used to avoid duplicates from the memory scan.
    *   **Source:** Inferred counts based *solely* on matching predefined name patterns in memory.

*   **`src/tools/scanner/dice_helpers.cpp` (`readDiceRegisters` & helpers):**
    *   **Mechanism:** Reads explicit stream count registers (`NB_TX`, `NB_RX`) relative to dynamically discovered TX/RX base addresses. Also utilizes EAP (Extended Audio Protocol) data via `eap_helpers.cpp` and falls back to chip-type-based defaults (`setDefaultDiceConfig`).
    *   **Source:** Explicit stream counts from registers, EAP data, or defaults. This determines the number of *streams*. It also calls functions to read per-stream details (`NB_AUDIO`, `NB_MIDI`) from `dice_stream_registers.cpp`.

*   **Summary of Sources:**
    1.  **Regex on Names:** Inferred counts from string patterns (`utils_explore_channels.cpp`).
    2.  **Explicit Stream Count Registers:** `NB_TX`, `NB_RX` values (`dice_helpers.cpp`).
    3.  **EAP Data:** Counts reported by EAP protocol (`dice_helpers.cpp` via `eap_helpers.cpp`).
    4.  **Per-Stream Config Sums:** Summing `NB_AUDIO` + `NB_MIDI` across all streams identified by Source 2 (`dice_helpers.cpp` via `dice_stream_registers.cpp`).
    5.  **Defaults:** Fallback values based on chip type (`dice_helpers.cpp`).

## 2. Proposed Plan for Improved Regex (for Validation)

Shift the role of regex in `utils_explore_channels.cpp` from direct counting to validation and boundary detection:

*   **Broaden Patterns:** Introduce more flexible regex patterns (e.g., `(Analog|ADAT|SPDIF|MIDI|Line|Mic)\s+(In|Out)\s*(\d+)`, `CH\s*\d+`) to validate if extracted strings *look like* channel names, confirming the scanned area's content.
*   **Boundary Detection:** Use regex or string analysis to identify the start/end of the channel list in memory. The number of potential entries within these boundaries can provide a count estimate, less reliant on exact naming.
*   **Quality Assessment:** Use regex success/failure on potential channel strings within detected boundaries to assess the *quality* and *consistency* of the name list, rather than deriving the primary count directly from pattern matches.

## 3. Proposed Plan for Cross-Validation

Leverage multiple data sources for robust validation:

*   **Identify Sources for Comparison:**
    *   **Source A (Names):** Count derived from name analysis in `utils_explore_channels.cpp` (potentially refined per step 2).
    *   **Source B (Stream Config Sums):**
        *   Get `NB_TX` and `NB_RX` (stream counts) from `dice_helpers.cpp`.
        *   For each stream, read `NB_AUDIO` and `NB_MIDI` registers (via `dice_stream_registers.cpp`).
        *   Calculate `Total_TX = sum(NB_AUDIO_tx + NB_MIDI_tx)` across all TX streams.
        *   Calculate `Total_RX = sum(NB_AUDIO_rx + NB_MIDI_rx)` across all RX streams.
    *   **Source C (EAP):** Extract total Audio IN/OUT and MIDI IN/OUT counts reported by EAP structures (via `eap_helpers.cpp`). Calculate `Total_TX` and `Total_RX`.

*   **Cross-Validation Process:**
    1.  Calculate `Count_Names_TX` and `Count_Names_RX` (Source A).
    2.  Calculate `Count_Streams_TX` and `Count_Streams_RX` (Source B).
    3.  Calculate `Count_EAP_TX` and `Count_EAP_RX` (Source C).
    4.  Compare counts for TX: `Count_Names_TX` vs `Count_Streams_TX` vs `Count_EAP_TX`.
    5.  Compare counts for RX: `Count_Names_RX` vs `Count_Streams_RX` vs `Count_EAP_RX`.

*   **Discrepancy Handling:**
    *   **Logging:** Log counts obtained from all sources (Names, Stream Sums, EAP) for TX and RX.
    *   **Warning:** Issue specific warnings detailing any mismatches found.
    *   **Prioritization:** Report the final count based on a priority order: EAP > Stream Sums > Names. Use the highest priority source with a valid result.
    *   **Reporting:** Modify scanner output to show counts from each source, the prioritized final count, and highlight inconsistencies.

## 4. Affected Files

*   `src/tools/scanner/utils_explore_channels.cpp`
*   `src/tools/scanner/dice_helpers.cpp`
*   `src/tools/scanner/dice_stream_registers.cpp` / `.hpp`
*   `src/tools/scanner/eap_helpers.cpp` / `.hpp`
*   `src/tools/scanner/scanner.hpp`
*   `src/tools/scanner/main.cpp` (or `utils_print.cpp`)
