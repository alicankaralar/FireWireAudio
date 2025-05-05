#include "scanner.hpp"
#include "utils_explore_general.hpp" // For extractCoherentRegisterStrings
#include "utils_print.hpp"           // For printDeviceInfo
#include "utils_signal.hpp"          // For signal handler setup

#include <csignal> // For signal()
#include <iostream>
#include <memory>                            // For std::shared_ptr
#include <spdlog/sinks/stdout_color_sinks.h> // For console logging
#include <spdlog/spdlog.h>
#include <vector>

namespace FWA::SCANNER {
// Global logger instance (definition)
std::shared_ptr<spdlog::logger> logger_;
} // namespace FWA::SCANNER

int main(int argc, char *argv[]) {
  // Initialize the logger
  try {
    FWA::SCANNER::logger_ = spdlog::stdout_color_mt("scanner_logger");
    // Set desired log level, e.g., debug
    FWA::SCANNER::logger_->set_level(spdlog::level::debug);
    spdlog::set_default_logger(
        FWA::SCANNER::logger_); // Optional: set as default
    spdlog::info("Scanner logger initialized.");
  } catch (const spdlog::spdlog_ex &ex) {
    std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    return 1;
  }

  std::cerr << "Debug: Entering main()..." << std::endl;

  // Register the signal handler for segmentation faults
  // Needs the handler function from utils.cpp
  signal(SIGSEGV, FWA::SCANNER::segfaultHandler);
  std::cerr << "Debug: SIGSEGV handler registered." << std::endl;

  FWA::SCANNER::FireWireScanner scanner;
  std::cerr << "Debug: FireWireScanner object created." << std::endl;

  std::cout << "Scanning for FireWire devices..." << std::endl;
  std::cerr << "Debug: Calling scanner.scanDevices()..." << std::endl;
  std::vector<FWA::SCANNER::FireWireDevice> devices = scanner.scanDevices();
  std::cerr << "Debug: scanner.scanDevices() returned." << std::endl;

  if (devices.empty()) {
    std::cout << "No FireWire devices found." << std::endl;
  } else {
    std::cout << "Found " << devices.size() << " device(s):" << std::endl;
    for (const auto &device : devices) {
      // Use printDeviceInfo from utils.cpp
      FWA::SCANNER::printDeviceInfo(device);

      // Extract coherent ASCII strings from registers
      FWA::SCANNER::extractCoherentRegisterStrings(device);
    }
  }

  std::cout << "===========================================" << std::endl;
  std::cout << "Scan complete." << std::endl;

  return 0;
}
