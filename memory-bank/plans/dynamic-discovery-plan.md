---
**STATUS UPDATE (2025-05-06):** This plan has been successfully implemented.
The `firewire_scanner` tool now incorporates FFADO-style dynamic base address discovery. All phases (Preparation, Implementation, Testing & Validation) are complete. This has replaced hardcoded addresses in the scanner.
---
# Plan: Implement Dynamic Base Address Discovery in Scanner

**Date:** 2025-05-06

**Goal:** Modify the `firewire_scanner` tool to read DICE registers using dynamically discovered base addresses (similar to FFADO) instead of potentially incorrect absolute offsets, aiming to overcome the `kIOReturnNotResponding` error during reads within the scanner. This serves as a Proof-of-Concept before modifying the core driver.

**Rationale:** Analysis of FFADO suggests DICE devices might expect register access relative to dynamically discovered base addresses rather than fixed offsets from the main CSR base (`0xFFFFF0000000`). Implementing this in the scanner allows testing this hypothesis in isolation.

**Plan:**

**Phase 0: Documentation & Preparation**

1.  **Update Memory Bank:** *(Completed)*
    *   Recorded the decision and rationale in `decisionLog.md`.
    *   Updated 'Current Focus' and 'Open Questions' in `activeContext.md`.
    *   Added new tasks to 'Next Steps' in `progress.md`.

2.  **Create `src/tools/scanner/scanner_defines.hpp`:**
    *   **Action:** Create a new header file for scanner-specific constants, particularly the FFADO-related addresses and offsets.
    *   **Initial Content:** Include guards, namespace, constants for `FFADO_INITIAL_BASE_ADDRESS`, dynamic base pointer offsets, and key relative register offsets (values to be verified in next step).

**Phase 1: Implementation in Scanner**

3.  **Information Gathering & Verification (FFADO Offsets):**
    *   **Action:** Review `memory-bank/productContext.md` and FFADO source (`ref/libffado-2.4.9/`) to confirm exact offset values. Update `scanner_defines.hpp`.

4.  **Refactor Scanner Code for New Defines:**
    *   **Action:** Modify scanner code (`dice_helpers.cpp`, etc.) to include and use the constants from `scanner_defines.hpp`.

5.  **Implement Dynamic Discovery Logic:**
    *   **Action:** Implement logic in `dice_helpers.cpp` to read dynamic base pointers and then read target registers using `dynamic_base + relative_offset`. Add logging and error handling.

**Phase 2: Testing & Validation**

6.  **Testing and Validation:**
    *   **Action:** Compile and run the modified `firewire_scanner`.
    *   **Analyze:** Check logs for successful dynamic base discovery and resolution of `kIOReturnNotResponding` errors for relevant registers.

**Diagram:**

```mermaid
graph TD
    A[Start Plan] --> B(Update Memory Bank);
    B -- Done --> C(Create scanner_defines.hpp);
    C --> D{Define FFADO Constants};
    D --> E(Verify FFADO Offsets);
    E --> F(Update scanner_defines.hpp);
    F --> G(Refactor Scanner Code to Use Defines);
    G --> H(Implement Dynamic Base Discovery Logic);
    H --> I(Compile Scanner);
    I --> J(Run Scanner on Device);
    J --> K(Analyze Results / Check for Error Resolution);

    subgraph Preparation
        B; C; D; E; F;
    end

    subgraph Implementation
       G; H;
    end

     subgraph Testing
        I; J; K;
    end
