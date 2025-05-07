#include "dice_register_readers.hpp"
#include "dice_aes_registers.hpp"
#include "dice_avs_registers.hpp"
#include "dice_clock_registers.hpp"
#include "dice_global_registers.hpp"
#include "dice_gpcsr_registers.hpp"
#include "dice_mixer_registers.hpp"

#include <iostream>

namespace FWA::SCANNER {
void readAllDiceRegisters(IOFireWireDeviceInterface **deviceInterface,
                          io_service_t service, FireWireDevice &device,
                          uint64_t globalBase, UInt32 generation) {
  std::cerr << "Debug [DICE]: Reading all DICE registers..." << std::endl;

  // Call each specialized register reader function
  readDiceGlobalRegisters(deviceInterface, service, device, globalBase,
                          generation);
  readGpcsrRegisters(deviceInterface, service, device, globalBase, generation);
  readClockControllerRegisters(deviceInterface, service, device, globalBase,
                               generation);
  readAesReceiverRegisters(deviceInterface, service, device, globalBase,
                           generation);
  readAudioMixerRegisters(deviceInterface, service, device, globalBase,
                          generation);
  readAvsRegisters(deviceInterface, service, device, globalBase, generation);

  std::cerr << "Debug [DICE]: Finished reading all DICE registers."
            << std::endl;
}

} // namespace FWA::SCANNER
