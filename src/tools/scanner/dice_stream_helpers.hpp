#pragma once

#include "io_helpers.hpp"
#include "scanner.hpp"
#include <IOKit/firewire/IOFireWireLib.h>

// Register offsets
namespace FWA::SCANNER {
extern UInt32 MODIFIED_GLOBAL_REGISTER_OFFSET;
extern UInt32 MODIFIED_GLOBAL_REGISTER_SIZE;
extern UInt32 MODIFIED_TX_REGISTER_OFFSET;
extern UInt32 MODIFIED_TX_REGISTER_SIZE;
extern UInt32 MODIFIED_RX_REGISTER_OFFSET;
extern UInt32 MODIFIED_RX_REGISTER_SIZE;
extern UInt32 MODIFIED_UNUSED1_REGISTER_OFFSET;
extern UInt32 MODIFIED_UNUSED1_REGISTER_SIZE;
extern UInt32 MODIFIED_UNUSED2_REGISTER_OFFSET;
extern UInt32 MODIFIED_UNUSED2_REGISTER_SIZE;
extern UInt32 MODIFIED_NUMBER_TX;
extern UInt32 MODIFIED_TX_SIZE;
extern UInt32 MODIFIED_NUMBER_RX;
extern UInt32 MODIFIED_RX_SIZE;

/**
 * Initializes the streaming parameters by reading DICE device registers
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param baseAddr The base address for the registers
 * @param generation The FireWire bus generation
 * @return true if initialization was successful, false otherwise
 */
bool initializeStreamingParams(IOFireWireDeviceInterface **deviceInterface,
                               io_service_t service, UInt64 baseAddr,
                               UInt32 generation);

void debugError(std::string error = "Error occurred\n");

/**
 * Reads a single quadlet from a TX register
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param i The index of the TX stream
 * @param offset The offset within the TX register space
 * @param &result Reference to store the read value
 * @param generation The FireWire bus generation
 * @return true if read was successful, false otherwise
 */
bool readTxReg(IOFireWireDeviceInterface **deviceInterface,
               io_service_t service, unsigned int i, UInt64 offset,
               UInt32 &result, UInt32 generation);
/**
 * Reads a single quadlet from a RX register
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param i The index of the RX stream
 * @param offset The offset within the RX register space
 * @param &result Reference to store the read value
 * @param generation The FireWire bus generation
 * @return true if read was successful, false otherwise
 */
bool readRxReg(IOFireWireDeviceInterface **deviceInterface,
               io_service_t service, unsigned int i, UInt64 offset,
               UInt32 &result, UInt32 generation);

/**
 * Reads a block of quadlets from a TX register
 *
 * @param deviceInterface The FireWire device interface
 * @param i The index of the TX stream
 * @param service The IO service
 * @param offset The offset within the TX register space
 * @param value Pointer to buffer to store read values
 * @param length The length in bytes to read
 * @param generation The FireWire bus generation
 * @return true if read was successful, false otherwise
 */
bool readTxRegBlock(IOFireWireDeviceInterface **deviceInterface,
                    io_service_t service, unsigned int i, UInt64 offset,
                    UInt32 &value, size_t length, UInt32 generation);

/**
 * Reads a block of quadlets from a RX register
 *
 * @param deviceInterface The FireWire device interface
 * @param i The index of the RX stream
 * @param service The IO service
 * @param offset The offset within the RX register space
 * @param value Pointer to buffer to store read values
 * @param length The length in bytes to read
 * @param generation The FireWire bus generation
 * @return true if read was successful, false otherwise
 */
bool readRxRegBlock(IOFireWireDeviceInterface **deviceInterface,
                    io_service_t service, unsigned int i, UInt64 offset,
                    UInt32 &value, size_t length, UInt32 generation);

/**
 * Reads a block of quadlets from any register
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param addr The absolute address to read from
 * @param value Pointer to buffer to store read values
 * @param length The length in bytes to read
 * @param generation The FireWire bus generation
 * @return true if read was successful, false otherwise
 */
bool readRegBlock(IOFireWireDeviceInterface **deviceInterface,
                  io_service_t service, UInt64 addr, UInt32 *value,
                  size_t length, UInt32 generation);

/**
 * Generates the offset for a TX register access
 *
 * @param i The index of the TX stream
 * @param offset The offset within the TX register space
 * @param length The length in bytes to access
 * @return The calculated offset, or DICE_INVALID_OFFSET if invalid
 */
UInt64 txOffsetGen(unsigned int i, UInt64 offset, size_t length);

/**
 * Generates the offset for a RX register access
 *
 * @param i The index of the RX stream
 * @param offset The offset within the RX register space
 * @param length The length in bytes to access
 * @return The calculated offset, or DICE_INVALID_OFFSET if invalid
 */
UInt64 rxOffsetGen(unsigned int i, UInt64 offset, size_t length);

/**
 * Reads the stream size value from a DICE device register
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param device The FireWireDevice object to update with register value
 * @param baseAddr The base address for the stream registers
 * @param offset The offset from the base address to the size register
 * @param streamType A string identifier for the stream type (e.g., "TX", "RX")
 * @param generation The FireWire bus generation
 * @param streamSizeQuadlets Reference to store the read stream size in quadlets
 * @return true if read was successful, false otherwise
 */
bool readStreamSize(IOFireWireDeviceInterface **deviceInterface,
                    io_service_t service, FireWireDevice &device,
                    UInt64 baseAddr, UInt64 offset, const char *streamType,
                    UInt32 generation, UInt32 &streamSizeQuadlets);

/**
 * Reads the stream count value from a DICE device register
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param device The FireWireDevice object to update with register value
 * @param baseAddr The base address for the stream registers
 * @param offset The offset from the base address to the count register
 * @param streamType A string identifier for the stream type (e.g., "TX", "RX")
 * @param generation The FireWire bus generation
 * @param streamCount Reference to store the read stream count
 */
void readStreamCount(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, FireWireDevice &device,
                     UInt64 baseAddr, UInt64 offset, const char *streamType,
                     UInt32 generation, UInt32 &streamCount);

/**
 * Reads all TX registers from a DICE device
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param generation The FireWire bus generation
 */
void readTXRegisters(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, UInt32 generation);

/**
 * Reads all RX registers from a DICE device
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param generation The FireWire bus generation
 */
void readRXRegisters(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, UInt32 generation);

/**
 * @brief Reads and returns the clock source names from the DICE device
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param generation The FireWire bus generation
 * @return stringlist A list of clock source names
 */
stringlist getClockSourceNameString(IOFireWireDeviceInterface **deviceInterface,
                                    io_service_t service, UInt32 generation);

/**
 * Gets the list of supported clock sources from the DICE device
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param generation The FireWire bus generation
 * @return std::vector<FWA::DICE::ClockSource> List of supported clock sources
 */
std::vector<FWA::DICE::ClockSource>
getSupportedClockSources(IOFireWireDeviceInterface **deviceInterface,
                         io_service_t service, UInt32 generation);

/**
 * Show detailed information about a DICE device
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param generation The FireWire bus generation
 */
void showDevice(IOFireWireDeviceInterface **deviceInterface,
                io_service_t service, UInt32 generation);

// Helper function to split name string on backslashes

enum E_Direction {
  E_Playback,
  E_Capture,
};
enum E_PortType {
  E_Audio,
  E_Midi,
  E_Control,
};

} // namespace FWA::SCANNER
