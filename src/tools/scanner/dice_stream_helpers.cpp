#include "dice_stream_helpers.hpp"
#include "FWA/dice/DiceDefines.hpp"
#include "endianness_helpers.hpp" // For deviceToHostInt32
#include "io_helpers.hpp"         // For safeReadQuadlet
#include <Error.h>

#include <expected>
#include <iomanip>
#include <iostream>

namespace FWA::SCANNER {
UInt32 MODIFIED_GLOBAL_REGISTER_OFFSET = 0;
UInt32 MODIFIED_GLOBAL_REGISTER_SIZE = 0;
UInt32 MODIFIED_TX_REGISTER_OFFSET = 0;
UInt32 MODIFIED_TX_REGISTER_SIZE = 0;
UInt32 MODIFIED_RX_REGISTER_OFFSET = 0;
UInt32 MODIFIED_RX_REGISTER_SIZE = 0;
UInt32 MODIFIED_UNUSED1_REGISTER_OFFSET = 0;
UInt32 MODIFIED_UNUSED1_REGISTER_SIZE = 0;
UInt32 MODIFIED_UNUSED2_REGISTER_OFFSET = 0;
UInt32 MODIFIED_UNUSED2_REGISTER_SIZE = 0;
UInt32 MODIFIED_NUMBER_TX = 0;
UInt32 MODIFIED_TX_SIZE = 0;
UInt32 MODIFIED_NUMBER_RX = 0;
UInt32 MODIFIED_RX_SIZE = 0;
UInt32 AUDIO_BASE_REGISTER = 0;
UInt32 MIDI_BASE_REGISTER = 0;
char dir[3];

bool (*writeFunc)(IOFireWireDeviceInterface **deviceInterface,
                  io_service_t service, unsigned int i, UInt64 offset,
                  UInt32 data, UInt32 generation);
bool (*readFunc)(IOFireWireDeviceInterface **deviceInterface,
                 io_service_t service, unsigned int i, UInt64 offset,
                 UInt32 &result, UInt32 generation);

UInt64 getAbsoluteAddr(UInt64 baseAddr, UInt32 offset) {

  return baseAddr + offset;
}

bool initializeStreamingParams(IOFireWireDeviceInterface **deviceInterface,
                               io_service_t service, UInt64 baseAddr,
                               UInt32 generation) {
  // offsets and sizes are returned in quadlets, but we use byte values
  std::cerr << "INFO: initializing streaming params" << std::endl;
  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr,
                          DICE_REGISTER_GLOBAL_PARAMETER_SPACE_OFFSET),
          &MODIFIED_GLOBAL_REGISTER_OFFSET, generation)) {
    std::cerr << "Could not initialize MODIFIED_GLOBAL_REGISTER_OFFSET"
              << std::endl;
    return false;
  }
  MODIFIED_GLOBAL_REGISTER_OFFSET *= 4;

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, DICE_REGISTER_GLOBAL_PARAMETER_SPACE_SIZE),
          &MODIFIED_GLOBAL_REGISTER_SIZE, generation)) {
    std::cerr << "Could not initialize MODIFIED_GLOBAL_REGISTER_SIZE"
              << std::endl;
    return false;
  }
  MODIFIED_GLOBAL_REGISTER_SIZE *= 4;

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, DICE_REGISTER_TX_PARAMETER_SPACE_OFFSET),
          &MODIFIED_TX_REGISTER_OFFSET, generation)) {
    std::cerr << "Could not initialize MODIFIED_TX_REGISTER_OFFSET"
              << std::endl;
    return false;
  }
  MODIFIED_TX_REGISTER_OFFSET *= 4;

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, DICE_REGISTER_TX_PARAMETER_SPACE_SIZE),
          &MODIFIED_TX_REGISTER_SIZE, generation)) {
    std::cerr << "Could not initialize MODIFIED_TX_REGISTER_SIZE" << std::endl;
    return false;
  }
  MODIFIED_TX_REGISTER_SIZE *= 4;

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, DICE_REGISTER_RX_PARAMETER_SPACE_OFFSET),
          &MODIFIED_RX_REGISTER_OFFSET, generation)) {
    std::cerr << "Could not initialize MODIFIED_RX_REGISTER_OFFSET"
              << std::endl;
    return false;
  }
  MODIFIED_RX_REGISTER_OFFSET *= 4;

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, DICE_REGISTER_RX_PARAMETER_SPACE_SIZE),
          &MODIFIED_RX_REGISTER_SIZE, generation)) {
    std::cerr << "Could not initialize MODIFIED_RX_REGISTER_SIZE" << std::endl;
    return false;
  }
  MODIFIED_RX_REGISTER_SIZE *= 4;

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, DICE_REGISTER_UNUSED1_SPACE_OFFSET),
          &MODIFIED_UNUSED1_REGISTER_OFFSET, generation)) {
    std::cerr << "Could not initialize MODIFIED_UNUSED1_REGISTER_OFFSET"
              << std::endl;
    return false;
  }
  MODIFIED_UNUSED1_REGISTER_OFFSET *= 4;

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, DICE_REGISTER_UNUSED1_SPACE_SIZE),
          &MODIFIED_UNUSED1_REGISTER_SIZE, generation)) {
    std::cerr << "Could not initialize MODIFIED_UNUSED1_REGISTER_SIZE"
              << std::endl;
    return false;
  }
  MODIFIED_UNUSED1_REGISTER_SIZE *= 4;

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, DICE_REGISTER_UNUSED2_SPACE_OFFSET),
          &MODIFIED_UNUSED2_REGISTER_OFFSET, generation)) {
    std::cerr << "Could not initialize MODIFIED_UNUSED2_REGISTER_OFFSET"
              << std::endl;
    return false;
  }
  MODIFIED_UNUSED2_REGISTER_OFFSET *= 4;

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, DICE_REGISTER_UNUSED2_SPACE_SIZE),
          &MODIFIED_UNUSED2_REGISTER_SIZE, generation)) {
    std::cerr << "Could not initialize MODIFIED_UNUSED2_REGISTER_SIZE"
              << std::endl;
    return false;
  }
  MODIFIED_UNUSED2_REGISTER_SIZE *= 4;

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, MODIFIED_TX_REGISTER_OFFSET +
                                        DICE_REGISTER_TX_NUMBER_TX_OFFSET),
          &MODIFIED_NUMBER_TX, generation)) {
    std::cerr << "Could not initialize MODIFIED_NUMBER_TX" << std::endl;
    return false;
  }

  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, MODIFIED_TX_REGISTER_OFFSET +
                                        DICE_REGISTER_TX_SIZE_TX_OFFSET),
          &MODIFIED_TX_SIZE, generation)) {
    std::cerr << "Could not initialize MODIFIED_TX_SIZE" << std::endl;
    return false;
  }
  MODIFIED_TX_SIZE *= 4;

  // USING MODIFIED_TX_OFFSET instead of MODIFIED_RX_OFFSET
  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, MODIFIED_RX_REGISTER_OFFSET +
                                        DICE_REGISTER_RX_NUMBER_RX_OFFSET),
          &MODIFIED_NUMBER_RX, generation)) {
    std::cerr << "Could not initialize MODIFIED_NUMBER_RX" << std::endl;
    return false;
  }

  // USING MODIFIED_TX_OFFSET instead of MODIFIED_RX_OFFSET
  if (safeReadQuadlet(
          deviceInterface, service,
          getAbsoluteAddr(baseAddr, MODIFIED_RX_REGISTER_OFFSET +
                                        DICE_REGISTER_RX_SIZE_RX_OFFSET),
          &MODIFIED_RX_SIZE, generation)) {
    std::cerr << "Could not initialize MODIFIED_RX_SIZE" << std::endl;
    return false;
  }
  MODIFIED_RX_SIZE *= 4;

  std::cerr << "DICE Parameter Space info:" << std::endl;
  std::cerr << " Global  : offset=0x" << std::hex
            << MODIFIED_GLOBAL_REGISTER_OFFSET << std::dec
            << " size=" << MODIFIED_GLOBAL_REGISTER_SIZE << std::endl;
  std::cerr << " TX      : offset=0x" << std::hex << MODIFIED_TX_REGISTER_OFFSET
            << std::dec << " size=" << MODIFIED_TX_REGISTER_SIZE << std::endl;
  std::cerr << "               nb=" << MODIFIED_NUMBER_TX
            << " size=" << MODIFIED_TX_SIZE << std::endl;
  std::cerr << " RX      : offset=0x" << std::hex << MODIFIED_RX_REGISTER_OFFSET
            << std::dec << " size=" << MODIFIED_RX_REGISTER_SIZE << std::endl;
  std::cerr << "               nb=" << MODIFIED_NUMBER_RX
            << " size=" << MODIFIED_RX_SIZE << std::endl;
  std::cerr << " UNUSED1 : offset=0x" << std::hex
            << MODIFIED_UNUSED1_REGISTER_OFFSET << std::dec
            << " size=" << MODIFIED_UNUSED1_REGISTER_SIZE << std::endl;
  std::cerr << " UNUSED2 : offset=0x" << std::hex
            << MODIFIED_UNUSED2_REGISTER_OFFSET << std::dec
            << " size=" << MODIFIED_UNUSED2_REGISTER_SIZE << std::endl;

  return true;
}

bool readRegBlock(IOFireWireDeviceInterface **deviceInterface,
                  io_service_t service, UInt64 addr, UInt32 *value,
                  size_t length, UInt32 generation) {
  const int max_blocksize_bytes = 512;

  int bytes_done = 0;
  while (bytes_done < length) {
    UInt64 curr_addr = addr + bytes_done;
    UInt32 *curr_data = value + bytes_done;
    UInt32 bytes_todo = length - bytes_done;

    if (bytes_todo > max_blocksize_bytes) {
      bytes_todo = max_blocksize_bytes;
    }

    if (!safeReadBlock(deviceInterface, service, curr_addr, curr_data,
                       &bytes_todo, generation)) {
      return false;
    }
    bytes_done += bytes_todo;
  }

  return true;
}

bool writeReg(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
              UInt64 absoluteAddr, UInt32 data, UInt32 generation) {
  UInt64 offset = absoluteAddr - DICE_REGISTER_BASE;
  std::cerr << "Writing base register offset: " << offset << " data: " << data
            << std::endl;

  if (offset >= DICE_INVALID_OFFSET) {
    std::cerr << "invalid offset: " << offset << std::endl;
    return false;
  }

  if (!safeWriteQuadlet(deviceInterface, service, absoluteAddr, data,
                        generation)) {
    std::cerr << "Could not write to base addr " << absoluteAddr << std::endl;
    return false;
  }
  {
    std::cerr << "Could not write to generation " << generation << " addr "
              << absoluteAddr << std::endl;
    return false;
  }
  return true;
}

bool readStreamReg(IOFireWireDeviceInterface **deviceInterface,
                   io_service_t service, unsigned int i, UInt64 offset,
                   UInt32 &result, UInt32 generation,
                   UInt64 (*offsetGen)(unsigned int, UInt64, size_t),
                   UInt32 registerOffset, const char *streamType) {
  IOReturn err = kIOReturnSuccess;
  UInt64 offset_stream = offsetGen(i, offset, sizeof(UInt32));
  UInt64 addr = DICE_REGISTER_BASE + registerOffset + offset_stream;
  err = safeReadQuadlet(deviceInterface, service, addr, &result, generation);
  if (err != kIOReturnSuccess) {
    std::cerr << "Could not read " << streamType << " register block"
              << std::endl;
    return false;
  }
  return true;
}

bool readStreamRegBlock(IOFireWireDeviceInterface **deviceInterface,
                        io_service_t service, unsigned int i, UInt64 offset,
                        UInt32 &value, size_t length, UInt32 generation,
                        UInt64 (*offsetGen)(unsigned int, UInt64, size_t),
                        UInt32 registerOffset, const char *streamType) {
  IOReturn err = kIOReturnSuccess;
  UInt32 offset_stream = offsetGen(i, offset, length);
  UInt64 addr = DICE_REGISTER_BASE + registerOffset + offset_stream;
  err =
      readRegBlock(deviceInterface, service, addr, &value, length, generation);
  if (err != kIOReturnSuccess) {
    std::cerr << "Could not read " << streamType << " register block"
              << std::endl;
    return false;
  }
  return true;
}

bool readTxReg(IOFireWireDeviceInterface **deviceInterface,
               io_service_t service, unsigned int i, UInt64 offset,
               UInt32 &result, UInt32 generation) {
  return readStreamReg(deviceInterface, service, i, offset, result, generation,
                       txOffsetGen, MODIFIED_TX_REGISTER_OFFSET, "TX");
}
bool writeTxReg(IOFireWireDeviceInterface **deviceInterface,
                io_service_t service, unsigned int i, UInt64 offset,
                UInt32 data, UInt32 generation) {
  std::cerr << "Writing tx " << i << " register offset 0x" << std::hex << offset
            << ", data: 0x" << data << std::dec << std::endl;

  UInt64 offset_tx = txOffsetGen(i, offset, sizeof(UInt32));
  return writeReg(deviceInterface, service,
                  MODIFIED_TX_REGISTER_OFFSET + offset_tx, data, generation);
}

bool readRxReg(IOFireWireDeviceInterface **deviceInterface,
               io_service_t service, unsigned int i, UInt64 offset,
               UInt32 &result, UInt32 generation) {
  return readStreamReg(deviceInterface, service, i, offset, result, generation,
                       rxOffsetGen, MODIFIED_RX_REGISTER_OFFSET, "RX");
}

bool writeRxReg(IOFireWireDeviceInterface **deviceInterface,
                io_service_t service, unsigned int i, UInt64 offset,
                UInt32 data, UInt32 generation) {
  std::cerr << "Writing rx " << i << " register offset 0x" << std::hex << offset
            << ", data: 0x" << data << std::dec << std::endl;

  UInt64 offset_rx = rxOffsetGen(i, offset, sizeof(UInt32));
  return writeReg(deviceInterface, service,
                  MODIFIED_RX_REGISTER_OFFSET + offset_rx, data, generation);
}

bool readTxRegBlock(IOFireWireDeviceInterface **deviceInterface,
                    io_service_t service, unsigned int i, UInt64 offset,
                    UInt32 &value, size_t length, UInt32 generation) {
  return readStreamRegBlock(deviceInterface, service, i, offset, value, length,
                            generation, txOffsetGen,
                            MODIFIED_TX_REGISTER_OFFSET, "TX");
}

bool readRxRegBlock(IOFireWireDeviceInterface **deviceInterface,
                    io_service_t service, unsigned int i, UInt64 offset,
                    UInt32 &value, size_t length, UInt32 generation) {
  return readStreamRegBlock(deviceInterface, service, i, offset, value, length,
                            generation, rxOffsetGen,
                            MODIFIED_RX_REGISTER_OFFSET, "RX");
}

bool validateStreamOffsetParams(unsigned int i, size_t length, bool isTx,
                                UInt64 streamOffset) {
  const char *streamType = isTx ? "TX" : "RX";
  UInt32 registerOffset =
      isTx ? MODIFIED_TX_REGISTER_OFFSET : MODIFIED_RX_REGISTER_OFFSET;
  UInt32 streamCount = isTx ? MODIFIED_NUMBER_TX : MODIFIED_NUMBER_RX;
  UInt32 streamSize = isTx ? MODIFIED_TX_SIZE : MODIFIED_RX_SIZE;
  UInt32 registerSize =
      isTx ? MODIFIED_TX_REGISTER_SIZE : MODIFIED_RX_REGISTER_SIZE;

  // registry offsets should always be smaller than 0x7FFFFFFF
  // because otherwise base + offset > 64bit
  if (registerOffset & 0x80000000) {
    std::cerr << "register offset not initialized yet" << std::endl;
    return false;
  }
  if (streamCount & 0x80000000) {
    std::cerr << streamType << " count not initialized yet" << std::endl;
    return false;
  }
  if (streamSize & 0x80000000) {
    std::cerr << streamType << " size not initialized yet" << std::endl;
    return false;
  }
  if (i >= streamCount) {
    std::cerr << streamType << " index out of range" << std::endl;
    return false;
  }

  // out-of-range check
  if (streamOffset + length > registerOffset + 4 + registerSize * streamCount) {
    std::cerr << "register offset+length too large: 0x" << std::hex
              << streamOffset << " + " << length << std::dec << std::endl;
    return false;
  }
  return true;
}

UInt64 streamOffsetGen(unsigned int i, UInt64 offset, size_t length,
                       bool isTx) {
  UInt32 streamSize = isTx ? MODIFIED_TX_SIZE : MODIFIED_RX_SIZE;
  UInt64 streamOffset = isTx ? DICE_REGISTER_TX_PARAM(streamSize, i, offset)
                             : DICE_REGISTER_RX_PARAM(streamSize, i, offset);

  if (!validateStreamOffsetParams(i, length, isTx, streamOffset)) {
    return DICE_INVALID_OFFSET;
  }

  return streamOffset;
}

UInt64 txOffsetGen(unsigned int i, UInt64 offset, size_t length) {
  return streamOffsetGen(i, offset, length, true);
}

UInt64 rxOffsetGen(unsigned int i, UInt64 offset, size_t length) {
  return streamOffsetGen(i, offset, length, false);
}

stringlist splitDiceNameString(std::string in) {
  in = in.substr(0, in.find("\\\\"));
  return stringlist::splitString(in, "\\");
}

stringlist getStreamNameString(IOFireWireDeviceInterface **deviceInterface,
                               io_service_t service, unsigned int i,
                               UInt32 generation, bool isTx) {
  stringlist names;
  size_t nameSize = isTx ? DICE_TX_NAMES_SIZE : DICE_RX_NAMES_SIZE;
  // Use fixed size array with maximum possible size
  char namestring[std::max(DICE_TX_NAMES_SIZE, DICE_RX_NAMES_SIZE) + 1];
  auto readFunc = isTx ? readTxRegBlock : readRxRegBlock;
  UInt64 namesOffset = isTx ? DICE_REGISTER_TX_NAMES_BASE_OFFSET
                            : DICE_REGISTER_RX_NAMES_BASE_OFFSET;

  if (!readFunc(deviceInterface, service, i, namesOffset, (UInt32 &)namestring,
                nameSize, generation)) {
    std::cerr << "Could not read " << (isTx ? "TX" : "RX") << " name string"
              << std::endl;
    return names;
  }

  namestring[nameSize] = '\0';
  return splitDiceNameString(std::string(namestring));
}

stringlist getTxNameString(IOFireWireDeviceInterface **deviceInterface,
                           io_service_t service, unsigned int i,
                           UInt32 generation) {
  return getStreamNameString(deviceInterface, service, i, generation, true);
}

stringlist getRxNameString(IOFireWireDeviceInterface **deviceInterface,
                           io_service_t service, unsigned int i,
                           UInt32 generation) {
  return getStreamNameString(deviceInterface, service, i, generation, false);
}

void readStreamRegisters(IOFireWireDeviceInterface **deviceInterface,
                         io_service_t service, UInt32 generation, bool isTx) {
  UInt32 tmp_quadlet = 0;
  const char *streamType = isTx ? "TX" : "RX";
  const char *streamName = isTx ? "Transmitter" : "Receiver";
  UInt32 streamCount = isTx ? MODIFIED_NUMBER_TX : MODIFIED_NUMBER_RX;
  auto readRegFunc = isTx ? readTxReg : readRxReg;
  auto getNameFunc = isTx ? getTxNameString : getRxNameString;

  std::cerr << " " << streamType << " param space:" << std::endl;
  std::cerr << "  Nb of " << (isTx ? "xmit" : "recv") << "        : "
            << streamCount << std::endl;

  for (unsigned int i = 0; i < streamCount; i++) {
    std::cerr << "  " << streamName << " " << i << ":" << std::endl;

    readRegFunc(deviceInterface, service, i,
                isTx ? DICE_REGISTER_TX_ISOCHRONOUS_BASE_OFFSET
                     : DICE_REGISTER_RX_ISOCHRONOUS_BASE_OFFSET,
                tmp_quadlet, generation);
    std::cerr << "   ISO channel       : " << tmp_quadlet << std::endl;

    if (isTx) {
      readRegFunc(deviceInterface, service, i,
                  DICE_REGISTER_TX_SPEED_BASE_OFFSET, tmp_quadlet, generation);
      std::cerr << "   ISO speed         : " << tmp_quadlet << std::endl;
    } else {
      readRegFunc(deviceInterface, service, i,
                  DICE_REGISTER_RX_SEQ_START_BASE_OFFSET, tmp_quadlet,
                  generation);
      std::cerr << "   Sequence Start    : " << tmp_quadlet << std::endl;
    }

    readRegFunc(deviceInterface, service, i,
                isTx ? DICE_REGISTER_TX_NUMBER_AUDIO_BASE_OFFSET
                     : DICE_REGISTER_RX_NUMBER_AUDIO_BASE_OFFSET,
                tmp_quadlet, generation);
    std::cerr << "   Nb audio channels : " << tmp_quadlet << std::endl;

    readRegFunc(deviceInterface, service, i,
                isTx ? DICE_REGISTER_TX_MIDI_BASE_OFFSET
                     : DICE_REGISTER_RX_MIDI_BASE_OFFSET,
                tmp_quadlet, generation);
    std::cerr << "   Nb midi channels  : " << tmp_quadlet << std::endl;

    readRegFunc(deviceInterface, service, i,
                isTx ? DICE_REGISTER_TX_AC3_CAPABILITIES_BASE_OFFSET
                     : DICE_REGISTER_RX_AC3_CAPABILITIES_BASE_OFFSET,
                tmp_quadlet, generation);
    std::cerr << "   AC3 caps          : 0x" << tmp_quadlet << std::endl;

    readRegFunc(deviceInterface, service, i,
                isTx ? DICE_REGISTER_TX_AC3_ENABLE_BASE_OFFSET
                     : DICE_REGISTER_RX_AC3_ENABLE_BASE_OFFSET,
                tmp_quadlet, generation);
    std::cerr << "   AC3 enable        : 0x" << tmp_quadlet << std::endl;

    stringlist channel_names =
        getNameFunc(deviceInterface, service, i, generation);
    std::cerr << "   Channel names     :" << std::endl;
    for (stringlist::iterator it = channel_names.begin();
         it != channel_names.end(); ++it) {
      std::cerr << "     " << (*it).c_str() << std::endl;
    }
  }
}

void readTXRegisters(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, UInt32 generation) {
  readStreamRegisters(deviceInterface, service, generation, true);
}

void readRXRegisters(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, UInt32 generation) {
  readStreamRegisters(deviceInterface, service, generation, false);
}

UInt64 globalOffsetGen(UInt64 offset, size_t length) {

  // registry offsets should always be smaller than 0x7FFFFFFF
  // because otherwise base + offset > 64bit
  if (MODIFIED_GLOBAL_REGISTER_OFFSET & 0x80000000) {
    std::cerr << "register offset not initialized yet" << std::endl;
    return DICE_INVALID_OFFSET;
  }
  // out-of-range check
  if (offset + length >
      MODIFIED_GLOBAL_REGISTER_OFFSET + MODIFIED_GLOBAL_REGISTER_SIZE) {
    std::cerr << "register offset+length too large: 0x" << std::hex << offset
              << " + " << length << std::dec << std::endl;
    return DICE_INVALID_OFFSET;
  }

  return offset;
}

bool readGlobalReg(IOFireWireDeviceInterface **deviceInterface,
                   io_service_t service, UInt64 offset, UInt32 result,
                   UInt32 generation) {
  UInt64 offset_gl = globalOffsetGen(offset, sizeof(UInt32));
  IOReturn err = kIOReturnSuccess;
  err = safeReadQuadlet(deviceInterface, service,
                        DICE_REGISTER_BASE + MODIFIED_GLOBAL_REGISTER_OFFSET +
                            offset_gl,
                        &result, generation);
  if (err != kIOReturnSuccess) {
    std::cerr << "Could not read global register" << std::endl;
    return false;
  }
  return true;
}

bool readGlobalRegBlock(IOFireWireDeviceInterface **deviceInterface,
                        io_service_t service, UInt64 offset, UInt32 *data,
                        size_t length, UInt32 generation) {
  UInt64 offset_gl = globalOffsetGen(offset, length);
  IOReturn err = kIOReturnSuccess;
  err = readRegBlock(deviceInterface, service,
                     DICE_REGISTER_BASE + MODIFIED_GLOBAL_REGISTER_OFFSET +
                         offset_gl,
                     data, length, generation);
  if (err != kIOReturnSuccess) {
    std::cerr << "Could not read global register block" << std::endl;
    return false;
  }
  return true;
}

bool writeGlobalReg(IOFireWireDeviceInterface **deviceInterface,
                    io_service_t service, UInt64 offset, UInt32 data,
                    UInt32 generation) {
  std::cerr << "Writing global register offset " << offset << " data: " << data
            << std::endl;

  UInt64 offset_gl = globalOffsetGen(offset, sizeof(UInt32));
  return writeReg(deviceInterface, service,
                  DICE_REGISTER_BASE + MODIFIED_GLOBAL_REGISTER_OFFSET +
                      offset_gl,
                  data, generation);
}

std::string getNickname(IOFireWireDeviceInterface **deviceInterface,
                        io_service_t service, UInt32 generation) {
  char namestring[DICE_NICK_NAME_SIZE + 1];

  if (!readGlobalRegBlock(
          deviceInterface, service, DICE_REGISTER_GLOBAL_NICK_NAME_OFFSET,
          (UInt32 *)namestring, DICE_NICK_NAME_SIZE, generation)) {
    std::cerr << "Could not read nickname string" << std::endl;
    return std::string("(unknown)");
  }

  namestring[DICE_NICK_NAME_SIZE] = '\0';
  return std::string(namestring);
}

bool maskedCheckZeroGlobalReg(IOFireWireDeviceInterface **deviceInterface,
                              io_service_t service, UInt64 offset, UInt32 mask,
                              UInt32 generation) {
  UInt32 result;
  readGlobalReg(deviceInterface, service, DICE_REGISTER_BASE + offset, result,
                generation);
  return ((result & mask) == 0);
}

bool maskedCheckNotZeroGlobalReg(IOFireWireDeviceInterface **deviceInterface,
                                 io_service_t service, uint32_t offset,
                                 uint32_t mask, UInt32 generation) {
  bool result = maskedCheckZeroGlobalReg(deviceInterface, service, offset, mask,
                                         generation);
  if (!result) {
    return false;
  }
  return true;
}

stringlist getClockSourceNameString(IOFireWireDeviceInterface **deviceInterface,
                                    io_service_t service, UInt32 generation) {
  stringlist names;
  char namestring[DICE_CLOCKSOURCENAMES_SIZE + 1];

  if (!readGlobalRegBlock(deviceInterface, service,
                          DICE_REGISTER_GLOBAL_CLOCKSOURCENAMES_OFFSET,
                          (UInt32 *)namestring, DICE_CLOCKSOURCENAMES_SIZE,
                          generation)) {
    std::cerr << "Could not read CLOCKSOURCE name string" << std::endl;
    return names;
  }

  namestring[DICE_CLOCKSOURCENAMES_SIZE] = '\0';
  names = splitDiceNameString(std::string(namestring));
  return names;
}

bool isClockSourceIdSlipping(unsigned int id, UInt32 ext_status_reg) {
  switch (id) {
  default:
    return false;
  case DICE_CLOCKSOURCE_AES1:
    return ext_status_reg & DICE_EXT_STATUS_AES0_SLIP;
  case DICE_CLOCKSOURCE_AES2:
    return ext_status_reg & DICE_EXT_STATUS_AES1_SLIP;
  case DICE_CLOCKSOURCE_AES3:
    return ext_status_reg & DICE_EXT_STATUS_AES2_SLIP;
  case DICE_CLOCKSOURCE_AES4:
    return ext_status_reg & DICE_EXT_STATUS_AES3_SLIP;
  case DICE_CLOCKSOURCE_AES_ANY:
    return false;
  case DICE_CLOCKSOURCE_ADAT:
    return ext_status_reg & DICE_EXT_STATUS_ADAT_SLIP;
  case DICE_CLOCKSOURCE_TDIF:
    return ext_status_reg & DICE_EXT_STATUS_TDIF_SLIP;
  case DICE_CLOCKSOURCE_ARX1:
    return ext_status_reg & DICE_EXT_STATUS_ARX1_SLIP;
  case DICE_CLOCKSOURCE_ARX2:
    return ext_status_reg & DICE_EXT_STATUS_ARX2_SLIP;
  case DICE_CLOCKSOURCE_ARX3:
    return ext_status_reg & DICE_EXT_STATUS_ARX3_SLIP;
  case DICE_CLOCKSOURCE_ARX4:
    return ext_status_reg & DICE_EXT_STATUS_ARX4_SLIP;
  case DICE_CLOCKSOURCE_WC: // FIXME: not in driver spec, so non-existant
    return ext_status_reg & DICE_EXT_STATUS_WC_SLIP;
  }
}

bool isClockSourceIdLocked(unsigned int id, UInt32 ext_status_reg) {
  switch (id) {
  default:
    return true;
  case DICE_CLOCKSOURCE_AES1:
    return ext_status_reg & DICE_EXT_STATUS_AES0_LOCKED;
  case DICE_CLOCKSOURCE_AES2:
    return ext_status_reg & DICE_EXT_STATUS_AES1_LOCKED;
  case DICE_CLOCKSOURCE_AES3:
    return ext_status_reg & DICE_EXT_STATUS_AES2_LOCKED;
  case DICE_CLOCKSOURCE_AES4:
    return ext_status_reg & DICE_EXT_STATUS_AES3_LOCKED;
  case DICE_CLOCKSOURCE_AES_ANY:
    return ext_status_reg & DICE_EXT_STATUS_AES_ANY_LOCKED;
  case DICE_CLOCKSOURCE_ADAT:
    return ext_status_reg & DICE_EXT_STATUS_ADAT_LOCKED;
  case DICE_CLOCKSOURCE_TDIF:
    return ext_status_reg & DICE_EXT_STATUS_TDIF_LOCKED;
  case DICE_CLOCKSOURCE_ARX1:
    return ext_status_reg & DICE_EXT_STATUS_ARX1_LOCKED;
  case DICE_CLOCKSOURCE_ARX2:
    return ext_status_reg & DICE_EXT_STATUS_ARX2_LOCKED;
  case DICE_CLOCKSOURCE_ARX3:
    return ext_status_reg & DICE_EXT_STATUS_ARX3_LOCKED;
  case DICE_CLOCKSOURCE_ARX4:
    return ext_status_reg & DICE_EXT_STATUS_ARX4_LOCKED;
  case DICE_CLOCKSOURCE_WC:
    return ext_status_reg & DICE_EXT_STATUS_WC_LOCKED;
  }
}

enum FWA::DICE::eClockSourceType clockIdToType(unsigned int id) {
  switch (id) {
  default:
    return DICE::eCT_Invalid;
  case DICE_CLOCKSOURCE_AES1:
  case DICE_CLOCKSOURCE_AES2:
  case DICE_CLOCKSOURCE_AES3:
  case DICE_CLOCKSOURCE_AES4:
  case DICE_CLOCKSOURCE_AES_ANY:
    return DICE::eCT_AES;
  case DICE_CLOCKSOURCE_ADAT:
    return DICE::eCT_ADAT;
  case DICE_CLOCKSOURCE_TDIF:
    return DICE::eCT_TDIF;
  case DICE_CLOCKSOURCE_ARX1:
  case DICE_CLOCKSOURCE_ARX2:
  case DICE_CLOCKSOURCE_ARX3:
  case DICE_CLOCKSOURCE_ARX4:
    return DICE::eCT_SytMatch;
  case DICE_CLOCKSOURCE_WC:
    return DICE::eCT_WordClock;
  case DICE_CLOCKSOURCE_INTERNAL:
    return DICE::eCT_Internal;
  }
}

std::vector<ClockSource>
getSupportedClockSources(IOFireWireDeviceInterface **deviceInterface,
                         io_service_t service, UInt32 generation) {
  std::vector<ClockSource> r;

  UInt32 clock_caps;
  readGlobalReg(deviceInterface, service,
                DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES_OFFSET, clock_caps,
                generation);
  uint16_t clocks_supported = (clock_caps >> 16) & 0xFFFF;
  std::cerr << " Clock caps: " << clock_caps
            << ", supported=" << clocks_supported << std::endl;

  UInt32 clock_select;
  readGlobalReg(deviceInterface, service,
                DICE_REGISTER_GLOBAL_CLOCK_SELECT_OFFSET, clock_select,
                generation);
  UInt8 clock_selected = (clock_select) & 0xFF;
  std::cerr << " Clock select: " << clock_select << ", selected=0x"
            << clock_selected << std::endl;

  UInt32 extended_status;
  readGlobalReg(deviceInterface, service,
                DICE_REGISTER_GLOBAL_EXTENDED_STATUS_OFFSET, extended_status,
                generation);

  stringlist names =
      getClockSourceNameString(deviceInterface, service, generation);
  if (names.size() < DICE_CLOCKSOURCE_COUNT) {
    std::cerr << "Not enough clock source names on device" << std::endl;
    return r;
  }
  for (unsigned int i = 0; i < DICE_CLOCKSOURCE_COUNT; i++) {
    bool supported = (((clocks_supported >> i) & 0x01) == 1);
    if (supported) {
      ClockSource source;
      source.type = clockIdToType(i);
      source.id = i;
      source.valid = true;
      source.locked = isClockSourceIdLocked(i, extended_status);
      source.slipping = isClockSourceIdSlipping(i, extended_status);
      source.active = (clock_selected == i);
      source.description = names.at(i);
      r.push_back(source);
    } else {
      std::cerr << "Clock source id " << i << " name: " << names.at(i)
                << " not supported by device" << std::endl;
    }
  }
  return r;
}

int getSamplingFrequency(IOFireWireDeviceInterface **deviceInterface,
                         io_service_t service, UInt32 generation) {
  int samplingFrequency;

  UInt32 clockreg;
  if (!readGlobalReg(deviceInterface, service,
                     DICE_REGISTER_GLOBAL_CLOCK_SELECT_OFFSET, clockreg,
                     generation)) {
    std::cerr << "Could not read CLOCK_SELECT register" << std::endl;
    return false;
  }

  clockreg = DICE_GET_RATE(clockreg);

  switch (clockreg) {
  case DICE_RATE_32K:
    samplingFrequency = 32000;
    break;
  case DICE_RATE_44K1:
    samplingFrequency = 44100;
    break;
  case DICE_RATE_48K:
    samplingFrequency = 48000;
    break;
  case DICE_RATE_88K2:
    samplingFrequency = 88200;
    break;
  case DICE_RATE_96K:
    samplingFrequency = 96000;
    break;
  case DICE_RATE_176K4:
    samplingFrequency = 176400;
    break;
  case DICE_RATE_192K:
    samplingFrequency = 192000;
    break;
  case DICE_RATE_ANY_LOW:
    samplingFrequency = 0;
    break;
  case DICE_RATE_ANY_MID:
    samplingFrequency = 0;
    break;
  case DICE_RATE_ANY_HIGH:
    samplingFrequency = 0;
    break;
  case DICE_RATE_NONE:
    samplingFrequency = 0;
    break;
  default:
    samplingFrequency = 0;
    break;
  }

  return samplingFrequency;
}

bool onSamplerateChange(IOFireWireDeviceInterface **deviceInterface,
                        io_service_t service, int oldSamplingFrequency,
                        UInt32 generation) {
  int current_sr = getSamplingFrequency(deviceInterface, service, generation);
  std::cerr << "Current sample rate is: " << current_sr << std::endl;
  std::cerr << "Previous sample rate was: " << oldSamplingFrequency
            << std::endl;

  if (current_sr != oldSamplingFrequency) {
    // Update for the new samplerate
    // if (m_eap)
    // {
    //     m_eap->update();
    // }
    if (!initializeStreamingParams(deviceInterface, service, DICE_REGISTER_BASE,
                                   generation)) {
      std::cerr << "Could not initialize streaming parameters" << std::endl;
      return false;
    }
    {
      std::cerr << "Could not initialize I/O functions" << std::endl;
      return false;
    }
    showDevice(deviceInterface, service, generation);
    return true;
  }
  return false;
}

bool setSamplingFrequency(IOFireWireDeviceInterface **deviceInterface,
                          io_service_t service, int samplingFrequency,
                          UInt32 generation) {
  std::cerr << "Setting sample rate: " << samplingFrequency << std::endl;

  bool supported = false;
  UInt32 select = 0x0;
  UInt32 clockreg;

  int current_sr = getSamplingFrequency(deviceInterface, service, generation);
  switch (samplingFrequency) {
  default:
  case 22050:
  case 24000:
    supported = false;
    break;
  case 32000:
    supported = maskedCheckNotZeroGlobalReg(
        deviceInterface, service, DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES_OFFSET,
        DICE_CLOCKCAP_RATE_32K, generation);
    select = DICE_RATE_32K;
    break;
  case 44100:
    supported = maskedCheckNotZeroGlobalReg(
        deviceInterface, service, DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES_OFFSET,
        DICE_CLOCKCAP_RATE_44K1, generation);
    select = DICE_RATE_44K1;
    break;
  case 48000:
    supported = maskedCheckNotZeroGlobalReg(
        deviceInterface, service, DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES_OFFSET,
        DICE_CLOCKCAP_RATE_48K, generation);
    select = DICE_RATE_48K;
    break;
  case 88200:
    supported = maskedCheckNotZeroGlobalReg(
        deviceInterface, service, DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES_OFFSET,
        DICE_CLOCKCAP_RATE_88K2, generation);
    select = DICE_RATE_88K2;
    break;
  case 96000:
    supported = maskedCheckNotZeroGlobalReg(
        deviceInterface, service, DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES_OFFSET,
        DICE_CLOCKCAP_RATE_96K, generation);
    select = DICE_RATE_96K;
    break;

// We currently can't support 4x rates on these devices.  See note
// in getSupportedSamplingFrequencies().
#if 0
        case 176400:
            supported=maskedCheckNotZeroGlobalReg(
                        DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES_OFFSET,
                        DICE_CLOCKCAP_RATE_176K4);
            select=DICE_RATE_176K4;
            break;
        case 192000:
            supported=maskedCheckNotZeroGlobalReg(
                        DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES_OFFSET,
                        DICE_CLOCKCAP_RATE_192K);
            select=DICE_RATE_192K;
            break;
#endif

    if (!supported) {
      std::cerr << "Unsupported sample rate: " << samplingFrequency
                << std::endl;
      return false;
    }

#if USE_OLD_DEFENSIVE_STREAMING_PROTECTION
    if (isIsoStreamingEnabled()) {
      std::cerr << "Cannot change samplerate while streaming is enabled"
                << std::endl;
      return false;
    }
#endif

    if (!readGlobalReg(deviceInterface, service,
                       DICE_REGISTER_GLOBAL_CLOCK_SELECT_OFFSET, clockreg,
                       generation)) {
      std::cerr << "Could not read CLOCK_SELECT register" << std::endl;
      return false;
    }

    clockreg = DICE_SET_RATE(clockreg, select);

    if (!writeGlobalReg(deviceInterface, service,
                        DICE_REGISTER_BASE +
                            DICE_REGISTER_GLOBAL_CLOCK_SELECT_OFFSET,
                        clockreg, generation)) {
      std::cerr << "Could not write CLOCK_SELECT register" << std::endl;
      return false;
    }

    // check if the write succeeded
    UInt32 clockreg_verify;
    if (!readGlobalReg(deviceInterface, service,
                       DICE_REGISTER_GLOBAL_CLOCK_SELECT_OFFSET,
                       clockreg_verify, generation)) {
      std::cerr << "Could not read CLOCK_SELECT register" << std::endl;
      return false;
    }

    if (clockreg != clockreg_verify) {
      std::cerr << "Samplerate register write failed" << std::endl;
      return false;
    }
  }

  // Wait up to 2s for the device to lock to the desired samplerate
  UInt32 statusreg;
  readGlobalReg(deviceInterface, service, DICE_REGISTER_GLOBAL_STATUS_OFFSET,
                statusreg, generation);
  int n_it = 0;
  while (((statusreg & 0x1) == 0 ||
          ((clockreg >> 8) & 0xFF) != ((statusreg >> 8) & 0xFF)) &&
         n_it < 20) {
    usleep(100000);
    readGlobalReg(deviceInterface, service, DICE_REGISTER_GLOBAL_STATUS_OFFSET,
                  statusreg, generation);
    n_it++;
  }
  if (n_it == 20) {
    std::cerr << "Initialization started before device was locked" << std::endl;
  }

  // Update for the new samplerate
  if (onSamplerateChange(deviceInterface, service, current_sr, generation)) {
    std::cerr << "Device configuration updated" << std::endl;
  }

  return true;
}

bool setActiveClockSource(IOFireWireDeviceInterface **deviceInterface,
                          io_service_t service, ClockSource source,
                          UInt32 generation)

{
  UInt32 clockreg;
  if (!readGlobalReg(deviceInterface, service,
                     DICE_REGISTER_GLOBAL_CLOCK_SELECT_OFFSET, clockreg,
                     generation)) {
    std::cerr << "Could not read CLOCK_SELECT register" << std::endl;
    return false;
  }

  clockreg = DICE_SET_CLOCKSOURCE(clockreg, source.id);

  if (!writeGlobalReg(deviceInterface, service,
                      DICE_REGISTER_GLOBAL_CLOCK_SELECT_OFFSET, clockreg,
                      generation)) {
    std::cerr << "Could not write CLOCK_SELECT register" << std::endl;
    return false;
  }

  // check if the write succeeded
  UInt32 clockreg_verify;
  if (!readGlobalReg(deviceInterface, service,
                     DICE_REGISTER_GLOBAL_CLOCK_SELECT_OFFSET, clockreg,
                     generation)) {
    std::cerr << "Could not read CLOCK_SELECT register" << std::endl;
    return false;
  }

  if (clockreg != clockreg_verify) {
    std::cerr << "CLOCK_SELECT register write failed" << std::endl;
    return false;
  }

  return DICE_GET_CLOCKSOURCE(clockreg_verify) == source.id;
}

bool readStreamSize(IOFireWireDeviceInterface **deviceInterface,
                    io_service_t service, FireWireDevice &device,
                    UInt64 baseAddr, UInt64 offset, const char *streamType,
                    UInt32 generation, UInt32 &streamSizeQuadlets) {
  UInt64 sizeAddr = baseAddr + offset;
  UInt32 rawSize = 0;
  IOReturn sizeStatus = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, sizeAddr, &rawSize, generation);
  if (sizeStatus == kIOReturnSuccess) {
    device.diceRegisters[sizeAddr] = rawSize;
    streamSizeQuadlets = deviceToHostInt32(rawSize, device.deviceEndianness);
    std::cerr << "Info [DICE]: Read " << streamType
              << " stream size from register (0x" << std::hex << sizeAddr
              << std::dec << "): " << streamSizeQuadlets << std::endl;
    return true;
  } else {
    std::cerr << "Warning [DICE]: Could not read " << streamType
              << " stream size register (0x" << std::hex << sizeAddr << std::dec
              << ") (status: " << sizeStatus << "). Assuming 256." << std::endl;
    return false;
  }
}

void readStreamCount(IOFireWireDeviceInterface **deviceInterface,
                     io_service_t service, FireWireDevice &device,
                     UInt64 baseAddr, UInt64 offset, const char *streamType,
                     UInt32 generation, UInt32 &streamCount) {
  UInt64 countAddr = baseAddr + offset;
  UInt32 rawCount = 0;
  IOReturn countStatus = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, countAddr, &rawCount, generation);
  if (countStatus == kIOReturnSuccess) {
    device.diceRegisters[countAddr] = rawCount;
    UInt32 readCount = deviceToHostInt32(rawCount, device.deviceEndianness);
    std::cerr << "Info [DICE]: Read " << streamType
              << " stream count from register (0x" << std::hex << countAddr
              << std::dec << "): " << readCount << std::endl;
    streamCount = readCount;
  } else {
    std::cerr << "Warning [DICE]: Failed to read " << streamType
              << " stream count register (0x" << std::hex << countAddr
              << std::dec << ") (status: " << countStatus
              << "). Using default/EAP " << streamType
              << " stream count: " << streamCount << std::endl;
  }
}

void showDevice(IOFireWireDeviceInterface **deviceInterface,
                io_service_t service, UInt32 generation) {
  UInt32 tmp_quadlet;
  UInt8 tmp_octlet;

  std::cerr << "Device is a DICE device" << std::endl;
  std::cerr << " DICE Parameter Space info:" << std::endl;
  std::cerr << "  Global  : offset=0x" << std::hex
            << MODIFIED_GLOBAL_REGISTER_OFFSET << std::dec
            << " size=" << MODIFIED_GLOBAL_REGISTER_SIZE << std::endl;
  std::cerr << "  TX      : offset=0x" << std::hex
            << MODIFIED_TX_REGISTER_OFFSET << std::dec
            << " size=" << MODIFIED_TX_REGISTER_SIZE << std::endl;
  std::cerr << "                nb=" << MODIFIED_NUMBER_TX
            << " size=" << MODIFIED_TX_SIZE << std::endl;
  std::cerr << "  RX      : offset=0x" << std::hex
            << MODIFIED_RX_REGISTER_OFFSET << std::dec
            << " size=" << MODIFIED_RX_REGISTER_SIZE << std::endl;
  std::cerr << "                nb=" << MODIFIED_NUMBER_RX
            << " size=" << MODIFIED_RX_SIZE << std::endl;
  std::cerr << "  UNUSED1 : offset=0x" << std::hex
            << MODIFIED_UNUSED1_REGISTER_OFFSET << std::dec
            << " size=" << MODIFIED_UNUSED1_REGISTER_SIZE << std::endl;
  std::cerr << "  UNUSED2 : offset=0x" << std::hex
            << MODIFIED_UNUSED2_REGISTER_OFFSET << std::dec
            << " size=" << MODIFIED_UNUSED2_REGISTER_SIZE << std::endl;

  std::cerr << " Global param space:" << std::endl;
  readGlobalRegBlock(
      deviceInterface, service, DICE_REGISTER_GLOBAL_OWNER_OFFSET,
      reinterpret_cast<UInt32 *>(&tmp_octlet), sizeof(UInt8), generation);
  std::cerr << "  Owner            : 0x" << std::hex << tmp_octlet << std::dec
            << std::endl;

  readGlobalReg(deviceInterface, service,
                DICE_REGISTER_GLOBAL_NOTIFICATION_OFFSET, tmp_quadlet,
                generation);
  std::cerr << "  Notification     : 0x" << std::hex << tmp_quadlet << std::dec
            << std::endl;

  readGlobalReg(deviceInterface, service,
                DICE_REGISTER_GLOBAL_NOTIFICATION_OFFSET, tmp_quadlet,
                generation);
  std::cerr << "  Nick name        : " << std::hex
            << getNickname(deviceInterface, service, generation).c_str()
            << std::dec << std::endl;

  readGlobalReg(deviceInterface, service,
                DICE_REGISTER_GLOBAL_CLOCK_SELECT_OFFSET, tmp_quadlet,
                generation);
  std::cerr << "  Clock Select     : " << std::hex
            << ((tmp_quadlet >> 8) & 0xFF) << (tmp_quadlet & 0xFF) << std::dec
            << std::endl;

  readGlobalReg(deviceInterface, service, DICE_REGISTER_GLOBAL_ENABLE_OFFSET,
                tmp_quadlet, generation);
  std::cerr << "  Enable           : " << (tmp_quadlet & 0x1 ? "true" : "false")
            << std::endl;

  readGlobalReg(deviceInterface, service, DICE_REGISTER_GLOBAL_STATUS_OFFSET,
                tmp_quadlet, generation);
  std::cerr << "  Clock Status     : "
            << (tmp_quadlet & 0x1 ? "locked " : "not locked ") << std::hex
            << ((tmp_quadlet >> 8) & 0xFF) << std::dec << std::endl;

  readGlobalReg(deviceInterface, service,
                DICE_REGISTER_GLOBAL_EXTENDED_STATUS_OFFSET, tmp_quadlet,
                generation);
  std::cerr << "  Extended Status  : 0x" << std::hex << tmp_quadlet << std::dec
            << std::endl;

  readGlobalReg(deviceInterface, service,
                DICE_REGISTER_GLOBAL_SAMPLE_RATE_OFFSET, tmp_quadlet,
                generation);
  std::cerr << "  Samplerate       : 0x" << std::hex << tmp_quadlet << std::dec
            << "(" << tmp_quadlet << ")" << "("
            << (getSamplingFrequency(deviceInterface, service, generation))
            << ")" << std::endl;

  readGlobalRegBlock(
      deviceInterface, service, DICE_REGISTER_GLOBAL_VERSION_OFFSET,
      reinterpret_cast<UInt32 *>(&tmp_quadlet), sizeof(UInt32), generation);
  std::cerr << "  Version          : 0x" << std::hex << tmp_quadlet << std::dec
            << std::endl;

  readGlobalReg(deviceInterface, service, DICE_REGISTER_GLOBAL_VERSION_OFFSET,
                tmp_quadlet, generation);
  std::cerr << "  Version          : 0x" << std::hex << tmp_quadlet << std::dec
            << " (" << DICE_DRIVER_SPEC_VERSION_NUMBER_GET_A(tmp_quadlet) << "."
            << DICE_DRIVER_SPEC_VERSION_NUMBER_GET_B(tmp_quadlet) << "."
            << DICE_DRIVER_SPEC_VERSION_NUMBER_GET_C(tmp_quadlet) << "."
            << DICE_DRIVER_SPEC_VERSION_NUMBER_GET_D(tmp_quadlet) << ")"
            << std::endl;
  readGlobalReg(deviceInterface, service,
                DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES_OFFSET, tmp_quadlet,
                generation);
  std::cerr << "  Clock caps       : 0x" << std::hex << tmp_quadlet << std::dec
            << std::endl;

  getSupportedClockSources(deviceInterface, service, generation);
  stringlist names =
      getClockSourceNameString(deviceInterface, service, generation);
  std::cerr << "  Clock sources    :" << std::endl;

  for (stringlist::iterator it = names.begin(); it != names.end(); ++it) {
    std::cerr << "    " << (*it).c_str() << std::endl;
  }

  std::cerr << " TX param space:" << std::endl;
  std::cerr << "  Nb of xmit        : " << MODIFIED_NUMBER_TX << std::endl;
  for (unsigned int i = 0; i < MODIFIED_NUMBER_TX; i++) {
    std::cerr << "  Transmitter " << i << ":" << std::endl;
    readTxReg(deviceInterface, service, i,
              DICE_REGISTER_TX_ISOCHRONOUS_BASE_OFFSET, tmp_quadlet,
              generation);
    std::cerr << "   ISO channel       : " << std::hex << tmp_quadlet
              << std::dec << std::endl;
    readTxReg(deviceInterface, service, i, DICE_REGISTER_TX_SPEED_BASE_OFFSET,
              tmp_quadlet, generation);
    std::cerr << "   ISO speed         : " << std::hex << tmp_quadlet
              << std::dec << std::endl;

    readTxReg(deviceInterface, service, i,
              DICE_REGISTER_TX_NUMBER_AUDIO_BASE_OFFSET, tmp_quadlet,
              generation);
    std::cerr << "   Nb audio channels : " << tmp_quadlet << std::endl;
    readTxReg(deviceInterface, service, i, DICE_REGISTER_TX_MIDI_BASE_OFFSET,
              tmp_quadlet, generation);
    std::cerr << "   Nb midi channels  : " << tmp_quadlet << std::endl;

    readTxReg(deviceInterface, service, i,
              DICE_REGISTER_TX_AC3_CAPABILITIES_BASE_OFFSET, tmp_quadlet,
              generation);
    std::cerr << "   AC3 caps          : 0x" << std::hex << tmp_quadlet
              << std::dec << std::endl;
    readTxReg(deviceInterface, service, i,
              DICE_REGISTER_TX_AC3_ENABLE_BASE_OFFSET, tmp_quadlet, generation);
    std::cerr << "   AC3 enable        : 0x" << std::hex << tmp_quadlet
              << std::dec << std::endl;

    stringlist channel_names =
        getTxNameString(deviceInterface, service, i, generation);
    std::cerr << "   Channel names     :" << std::endl;
    for (stringlist::iterator it = channel_names.begin();
         it != channel_names.end(); ++it) {
      std::cerr << "     " << (*it).c_str() << std::endl;
    }
  }

  std::cerr << " RX param space:" << std::endl;
  std::cerr << "  Nb of recv        : " << MODIFIED_NUMBER_RX << std::endl;
  for (unsigned int i = 0; i < MODIFIED_NUMBER_RX; i++) {
    std::cerr << "  Receiver " << i << ":" << std::endl;

    readRxReg(deviceInterface, service, i,
              DICE_REGISTER_RX_ISOCHRONOUS_BASE_OFFSET, tmp_quadlet,
              generation);
    std::cerr << "   ISO channel       : " << std::hex << tmp_quadlet
              << std::dec << std::endl;
    readRxReg(deviceInterface, service, i,
              DICE_REGISTER_RX_SEQ_START_BASE_OFFSET, tmp_quadlet, generation);
    std::cerr << "   Sequence start    : " << std::hex << tmp_quadlet
              << std::dec << std::endl;

    readRxReg(deviceInterface, service, i,
              DICE_REGISTER_RX_NUMBER_AUDIO_BASE_OFFSET, tmp_quadlet,
              generation);
    std::cerr << "   Nb audio channels : " << tmp_quadlet << std::endl;
    readRxReg(deviceInterface, service, i, DICE_REGISTER_RX_MIDI_BASE_OFFSET,
              tmp_quadlet, generation);
    std::cerr << "   Nb midi channels  : " << tmp_quadlet << std::endl;

    readRxReg(deviceInterface, service, i,
              DICE_REGISTER_RX_AC3_CAPABILITIES_BASE_OFFSET, tmp_quadlet,
              generation);
    std::cerr << "   AC3 caps          : 0x" << std::hex << tmp_quadlet
              << std::dec << std::endl;
    readRxReg(deviceInterface, service, i,
              DICE_REGISTER_RX_AC3_ENABLE_BASE_OFFSET, tmp_quadlet, generation);
    std::cerr << "   AC3 enable        : 0x" << std::hex << tmp_quadlet
              << std::dec << std::endl;

    stringlist channel_names =
        getRxNameString(deviceInterface, service, i, generation);
    std::cerr << "   Channel names     :" << std::endl;
    for (stringlist::iterator it = channel_names.begin();
         it != channel_names.end(); ++it) {
      std::cerr << "     " << (*it).c_str() << std::endl;
    }
  }
}
void setRXTXfuncs(const E_Direction direction) {
  if (direction == E_Capture) {
    // we are a receive processor
    AUDIO_BASE_REGISTER = DICE_REGISTER_TX_NUMBER_AUDIO_BASE_OFFSET;
    MIDI_BASE_REGISTER = DICE_REGISTER_TX_MIDI_BASE_OFFSET;
    writeFunc = &writeTxReg;
    readFunc = &readTxReg;
    strcpy(dir, "TX");
  } else {
    // we are a transmit processor
    AUDIO_BASE_REGISTER = DICE_REGISTER_RX_NUMBER_AUDIO_BASE_OFFSET;
    MIDI_BASE_REGISTER = DICE_REGISTER_RX_MIDI_BASE_OFFSET;
    writeFunc = &writeRxReg;
    readFunc = &readRxReg;
    strcpy(dir, "RX");
  }
};
bool prepareSP(unsigned int i, const E_Direction direction_requested) {
  UInt32 nb_audio;
  UInt32 nb_midi;
  unsigned int nb_channels = 0;
  E_Direction direction = direction_requested;

  bool snoopMode = false;

  // get the device specific and/or global SP configuration
  Util::Configuration &config = getDeviceManager().getConfiguration();
  // base value is the config.h value
  float recv_sp_dll_bw = STREAMPROCESSOR_DLL_BW_HZ;
  float xmit_sp_dll_bw = STREAMPROCESSOR_DLL_BW_HZ;

  int xmit_max_cycles_early_transmit = AMDTP_MAX_CYCLES_TO_TRANSMIT_EARLY;
  int xmit_transfer_delay = AMDTP_TRANSMIT_TRANSFER_DELAY;
  int xmit_min_cycles_before_presentation =
      AMDTP_MIN_CYCLES_BEFORE_PRESENTATION;

  // we can override that globally
  config.getValueForSetting("streaming.common.recv_sp_dll_bw", recv_sp_dll_bw);
  config.getValueForSetting("streaming.common.xmit_sp_dll_bw", xmit_sp_dll_bw);
  config.getValueForSetting("streaming.amdtp.xmit_max_cycles_early_transmit",
                            xmit_max_cycles_early_transmit);
  config.getValueForSetting("streaming.amdtp.xmit_transfer_delay",
                            xmit_transfer_delay);
  config.getValueForSetting(
      "streaming.amdtp.xmit_min_cycles_before_presentation",
      xmit_min_cycles_before_presentation);

  // or override in the device section
  uint32_t vendorid = getConfigRom().getNodeVendorId();
  uint32_t modelid = getConfigRom().getModelId();
  config.getValueForDeviceSetting(vendorid, modelid, "recv_sp_dll_bw",
                                  recv_sp_dll_bw);
  config.getValueForDeviceSetting(vendorid, modelid, "xmit_sp_dll_bw",
                                  xmit_sp_dll_bw);
  config.getValueForDeviceSetting(vendorid, modelid,
                                  "xmit_max_cycles_early_transmit",
                                  xmit_max_cycles_early_transmit);
  config.getValueForDeviceSetting(vendorid, modelid, "xmit_transfer_delay",
                                  xmit_transfer_delay);
  config.getValueForDeviceSetting(vendorid, modelid,
                                  "xmit_min_cycles_before_presentation",
                                  xmit_min_cycles_before_presentation);

  stringlist names_audio;
  stringlist names_midi;

  Streaming::StreamProcessor *p;
  float dll_bw;

  // set function pointers and base IO for upcoming code
  setRXTXfuncs(direction_requested);

  if (direction == E_Capture) {
    // we are a receive processor
    names_audio = getCptrNameString(i);
  } else {
    names_audio = getPbckNameString(i);
  }

  if (!(*this.*readFunc)(i, audio_base_register, &nb_audio)) {
    debugError(
        "Could not read DICE_REGISTER_%s_NB_AUDIO_BASE register for A%s%u\n",
        dir, dir, i);
    // non-fatal error, simply returning true. Only false is important for
    // prepare();
    return true;
  }

  if (!(*this.*readFunc)(i, midi_base_register, &nb_midi)) {
    debugError("Could not read DICE_REGISTER_%s_MIDI_BASE register for A%s%u\n",
               dir, dir, i);
    // non-fatal error, simply returning true. Only false is important for
    // prepare();
    return true;
  }

  // request the channel names
  if (names_audio.size() != nb_audio) {
    const char *dir_str = (direction == E_Capture) ? "input" : "output";
    debugWarning(
        "The audio channel name vector is incorrect, using default names\n");
    names_audio.clear();

    for (unsigned int j = 0; j < nb_audio; j++) {
      std::ostringstream newname;
      newname << dir_str << i << ":" << (j + 1);
      names_audio.push_back(newname.str());
    }
  }

  nb_channels = nb_audio;
  if (nb_midi)
    nb_channels += 1; // midi-muxed counts as one

  // construct the MIDI names
  for (unsigned int j = 0; j < nb_midi; j++) {
    std::ostringstream newname;
    newname << "midi " << j;
    names_midi.push_back(newname.str());
  }

  // construct the streamprocessor
  if (direction == E_Capture || snoopMode) {
    p = new Streaming::AmdtpReceiveStreamProcessor(*this, nb_channels);
    dll_bw = recv_sp_dll_bw;
    direction = E_Capture;
  } else {
    p = new Streaming::AmdtpTransmitStreamProcessor(*this, nb_channels);
    dll_bw = xmit_sp_dll_bw;

#if AMDTP_ALLOW_PAYLOAD_IN_NODATA_XMIT
    // the DICE-II cannot handle payload in the NO-DATA packets.
    // the other DICE chips don't need payload. Therefore
    // we disable it.
    ((Streaming::AmdtpTransmitStreamProcessor *)p)
        ->sendPayloadForNoDataPackets(false);
#endif

    // transmit control parameters
    ((Streaming::AmdtpTransmitStreamProcessor *)p)
        ->setMaxCyclesToTransmitEarly(xmit_max_cycles_early_transmit);
    ((Streaming::AmdtpTransmitStreamProcessor *)p)
        ->setTransferDelay(xmit_transfer_delay);
    ((Streaming::AmdtpTransmitStreamProcessor *)p)
        ->setMinCyclesBeforePresentation(xmit_min_cycles_before_presentation);
  }

  if (!p->init()) {
    debugFatal("Could not initialize %s processor!\n", dir);
    delete p;
    // non-fatal error, simply returning true. Only false is important for
    // prepare();
    return true;
  }

  // add audio ports to the processor
  for (unsigned int j = 0; j < nb_audio; j++) {
    diceChannelInfo channelInfo;
    channelInfo.name = names_audio.at(j);
    channelInfo.portType = ePT_Analog;
    channelInfo.streamPosition = j;
    channelInfo.streamLocation = 0;

    if (!addChannelToProcessor(&channelInfo, p, direction)) {
      debugError("Could not add channel %s to StreamProcessor\n",
                 channelInfo.name.c_str());
      continue;
    }
  }

  // add midi ports to the processor
  for (unsigned int j = 0; j < nb_midi; j++) {
    diceChannelInfo channelInfo;
    channelInfo.name = names_midi.at(j);
    channelInfo.portType = ePT_MIDI;
    channelInfo.streamPosition = nb_audio;
    channelInfo.streamLocation = j;

    if (!addChannelToProcessor(&channelInfo, p, direction)) {
      debugError("Could not add channel %s to StreamProcessor\n",
                 channelInfo.name.c_str());
      continue;
    }
  }

  if (!p->setDllBandwidth(dll_bw)) {
    debugFatal("Could not set DLL bandwidth\n");
    delete p;
    return false;
  }

  debugOutput(DEBUG_LEVEL_VERBOSE,
              "(%p) %s SP on channel [%d audio, %d midi]\n", this, dir,
              nb_audio, nb_midi);
  // add the SP to the vector
  if (direction_requested == E_Capture) {
    m_receiveProcessors.push_back(p);
  } else {
    // we put this SP into the transmit SP vector,
    // no matter if we are in snoop mode or not
    // this allows us to find out what direction
    // a certain stream should have.
    m_transmitProcessors.push_back(p);
  }

  return true;
}
} // namespace FWA::SCANNER
