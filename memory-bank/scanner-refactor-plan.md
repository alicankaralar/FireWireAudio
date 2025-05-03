# Refactoring Plan for `src/tools/firewire_scanner.cpp`

**Objective:** Refactor the existing `src/tools/firewire_scanner.cpp` (approx. 1500 lines) into smaller, more manageable files (under 250 lines each) and remove unnecessary code. This improves maintainability and aids future development, particularly when working with LLMs.

**Steps:**

1.  **Create New Directory:**
    *   A new directory `src/tools/scanner/` will be created to house the refactored code.

2.  **New File Structure:**
    *   The code will be broken down into the following files within `src/tools/scanner/`:
        *   `main.cpp`: Contains the `main()` function (~30 lines).
        *   `scanner.hpp`: `FireWireScanner` class declaration and `FireWireDevice` struct (~50 lines).
        *   `scanner.cpp`: `FireWireScanner` constructor, destructor, and the main `scanDevices()` method implementation (~150 lines).
        *   `io_helpers.hpp`/`.cpp`: Helper functions for IOKit interactions like `safeReadQuadlet`, `readQuadlet`, `getDeviceInfo`, `interpretAsASCII` (~20 + ~150 lines).
        *   `config_rom.hpp`/`.cpp`: The `parseConfigRom` function (~10 + ~150 lines).
        *   `dice_helpers.hpp`/`.cpp`: DICE-specific functions like `readDiceRegisters`, EAP functions, `setDefaultDiceConfig` (~15 + ~250 lines).
        *   `utils.hpp`/`.cpp`: Utility/debugging functions like `printDeviceInfo`, `exploreDiceMemoryLayout`, `segfaultHandler` and its globals (~15 + ~200 lines).

3.  **Code Removal:**
    *   The commented-out `bruteforceRegisterScan` function will be removed entirely.
    *   The redundant `getDeviceGuid` helper function (whose logic is already in `getDeviceInfo`) will be removed.

4.  **Build System Update:**
    *   The `src/tools/CMakeLists.txt` file will be updated:
        *   Replace `add_executable(firewire_scanner firewire_scanner.cpp)` with a target including the new `.cpp` files:
            ```cmake
            add_executable(firewire_scanner
                scanner/main.cpp
                scanner/scanner.cpp
                scanner/io_helpers.cpp
                scanner/config_rom.cpp
                scanner/dice_helpers.cpp
                scanner/utils.cpp
            )
            ```
        *   Ensure necessary include paths point correctly to the `include/` directory and potentially the new `src/tools/scanner/` directory if needed for internal includes.
        *   Maintain link dependencies: `target_link_libraries(firewire_scanner PRIVATE FWA IOKit CoreFoundation)`.

5.  **Implementation:**
    *   Switch to Code mode to perform the file creation, code moving, and CMake modifications.

**Proposed Structure Diagram:**

```mermaid
graph TD
    A[firewire_scanner (executable)] --> B(scanner/main.cpp);
    A --> C(scanner/scanner.cpp);
    A --> D(scanner/io_helpers.cpp);
    A --> E(scanner/config_rom.cpp);
    A --> F(scanner/dice_helpers.cpp);
    A --> G(scanner/utils.cpp);

    C --> H(scanner/scanner.hpp);
    D --> I(scanner/io_helpers.hpp);
    E --> J(scanner/config_rom.hpp);
    F --> K(scanner/dice_helpers.hpp);
    G --> L(scanner/utils.hpp);

    H --> M(FireWireDevice struct);
    H --> N(FireWireScanner class);
    C --> N;
    I --> N;
    J --> N;
    K --> N;
    L --> N;

    subgraph Scanner Core
        B; C; H; M; N;
    end

    subgraph IO Helpers
        D; I;
    end

    subgraph Config ROM Parsing
        E; J;
    end

    subgraph DICE Logic
        F; K;
    end

    subgraph Utilities/Debug
        G; L;
    end

    classDef file fill:#f9f,stroke:#333,stroke-width:2px;
    classDef header fill:#ccf,stroke:#333,stroke-width:1px;
    class B,C,D,E,F,G file;
    class H,I,J,K,L header;