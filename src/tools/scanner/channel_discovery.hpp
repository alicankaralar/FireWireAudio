#ifndef CHANNEL_DISCOVERY_HPP
#define CHANNEL_DISCOVERY_HPP

#include <cstdint>
#include <string>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER {
/**
 * @brief Dynamically discovers the address where channel names are stored
 *
 * This function tries several potential addresses where channel names might be
 * stored and looks for patterns like "OUTPUT CH1" or "INPUT CH1" to identify
 * the correct address.
 *
 * @param deviceInterface Pointer-to-pointer to an opened
 * IOFireWireDeviceInterface
 * @param service The io_service_t representing the FireWire nub
 * @param generation The current bus generation number
 * @return The discovered address, or a default fallback if none is found
 */
uint64_t
discoverChannelNamesAddress(IOFireWireDeviceInterface **deviceInterface,
                            io_service_t service, UInt32 generation);

} // namespace FWA::SCANNER

#endif // CHANNEL_DISCOVERY_HPP
