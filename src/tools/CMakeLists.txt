cmake_minimum_required(VERSION 3.10) # Consider matching root CMakeLists.txt version (3.26) if issues arise
project(ScannerTool CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF) # Match root CMakeLists.txt
set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS}   -stdlib=libc++") # Match root CMakeLists.txt

# --- Dependencies using FetchContent ---
include(FetchContent)
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.15.2) # Match root CMakeLists.txt tag
FetchContent_MakeAvailable(spdlog)

# Define the executable name
set(EXECUTABLE_NAME firewire_scanner) # Renamed to avoid conflict with 'scanner' directory

# Define source files relative to this CMakeLists.txt (src/tools)
set(SCANNER_SOURCES
    scanner/config_rom.cpp
    scanner/channel_count_validator.cpp
    scanner/dice_stream_registers.cpp
    scanner/eap_helpers.cpp
    scanner/endianness_helpers.cpp
    scanner/io_helpers.cpp
    scanner/main.cpp
    scanner/scanner.cpp
    scanner/utils_explore_channels.cpp
    scanner/utils_explore_general.cpp
    scanner/utils_explore.cpp
    scanner/utils_memory.cpp
    scanner/utils_print.cpp
    scanner/utils_signal.cpp
    # Removed utils_string.cpp as it's just a stub that includes other files
    # New files from dice_helpers.cpp refactoring
    scanner/dice_base_discovery.cpp
    scanner/dice_register_readers.cpp
    scanner/dice_config.cpp
    # New files from dice_register_readers.cpp refactoring
    scanner/dice_global_registers.cpp
    scanner/dice_gpcsr_registers.cpp
    scanner/dice_clock_registers.cpp
    scanner/dice_aes_registers.cpp
    scanner/dice_mixer_registers.cpp
    scanner/dice_avs_registers.cpp
    # New files from utils_string.cpp refactoring
    scanner/string_extraction.cpp
    scanner/channel_discovery.cpp
    scanner/validation_utils.cpp
    # New files from utils_explore_general.cpp refactoring
    scanner/memory_explorer.cpp
    # New files from register_string_analyzer.cpp refactoring
    scanner/register_string_analyzer.cpp
    scanner/register_known_locations.cpp
    scanner/register_address_ranges.cpp
    scanner/register_name_analyzer.cpp
    # New files from utils_explore_channels.cpp refactoring
    scanner/channel_name_extractor.cpp
    scanner/channel_pattern_matcher.cpp
    scanner/channel_summary_reporter.cpp
    # New stream helper functions 
    scanner/dice_stream_helpers.cpp
)

# Add include directories relative to this CMakeLists.txt
include_directories(
    ../../include
    scanner
)

# Add the executable
add_executable(${EXECUTABLE_NAME} ${SCANNER_SOURCES})

# Link necessary libraries and frameworks (macOS specific)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE
    spdlog::spdlog # Link spdlog
)

if(APPLE)
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework Foundation" # Add Foundation framework
    )
endif()

# Optional: Set output directory if desired (e.g., build/bin)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
