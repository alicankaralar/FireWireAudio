// include/FWA/dice/DiceDefines.hpp
#pragma once

#include <cstdint>

#include <set>
#include <string>

namespace FWA {
namespace DICE {

// Structure to identify a device by Vendor and Model ID
struct DeviceIdentifier {
  uint32_t vendorId;
  uint32_t modelId;

  // Comparison operators for use in std::set
  bool operator<(const DeviceIdentifier &other) const {
    if (vendorId != other.vendorId)
      return vendorId < other.vendorId;
    return modelId < other.modelId;
  }
  bool operator==(const DeviceIdentifier &other) const {
    return vendorId == other.vendorId && modelId == other.modelId;
  }
};

// Set of devices known *not* to support EAP
const std::set<DeviceIdentifier> EAP_UNSUPPORTED_DEVICES = {
    {0x10c73f, 0x00000001} // Midas Venice F32 (Based on FFADO log analysis)
                           // Add other devices here as discovered
};

// DICE version definition
#define DICE_VER_1_0_7_0

#define DICE_INVALID_OFFSET 0xFFFFF00000000000ULL

/*
 * This header is based upon the DICE II driver specification
 * version 1.0.7.0 from libffado
 */

// Register addresses & offsets
//  DICE_PRIVATE_SPACE registers
#define DICE_REGISTER_BASE 0x0000FFFFE0000000ULL

#define DICE_REGISTER_TX_BASE_OFFSET 0x400ULL
#define DICE_REGISTER_RX_BASE_OFFSET 0x800ULL

// Default absolute base addresses for TX and RX blocks (used as fallback)
#define DICE_REGISTER_TX_BASE                                                  \
  (DICE_REGISTER_BASE + DICE_REGISTER_TX_BASE_OFFSET)
#define DICE_REGISTER_RX_BASE                                                  \
  (DICE_REGISTER_BASE + DICE_REGISTER_RX_BASE_OFFSET)

#define DICE_REGISTER_GLOBAL_PARAMETER_SPACE_OFFSET 0x0000
#define DICE_REGISTER_GLOBAL_PARAMETER_SPACE_SIZE 0x0004
#define DICE_REGISTER_TX_PARAMETER_SPACE_OFFSET 0x0008
#define DICE_REGISTER_TX_PARAMETER_SPACE_SIZE 0x000C
#define DICE_REGISTER_RX_PARAMETER_SPACE_OFFSET 0x0010
#define DICE_REGISTER_RX_PARAMETER_SPACE_SIZE 0x0014
#define DICE_REGISTER_UNUSED1_SPACE_OFFSET 0x0018
#define DICE_REGISTER_UNUSED1_SPACE_SIZE 0x001C
#define DICE_REGISTER_UNUSED2_SPACE_OFFSET 0x0020
#define DICE_REGISTER_UNUSED2_SPACE_SIZE 0x0024

//  GLOBAL_PAR_SPACE registers
#define DICE_REGISTER_GLOBAL_OWNER_OFFSET                                      \
  0x0000 // Shares address with DICE_REGISTER_GLOBAL_PARAMETER_SPACE_OFFSET
#define DICE_REGISTER_GLOBAL_NOTIFICATION_OFFSET 0x0008
#define DICE_REGISTER_GLOBAL_NICK_NAME_OFFSET 0x000C
#define DICE_REGISTER_GLOBAL_CLOCK_SELECT_OFFSET 0x004C
#define DICE_REGISTER_GLOBAL_ENABLE_OFFSET 0x0050
#define DICE_REGISTER_GLOBAL_STATUS_OFFSET 0x0054
#define DICE_REGISTER_GLOBAL_EXTENDED_STATUS_OFFSET 0x0058
#define DICE_REGISTER_GLOBAL_SAMPLE_RATE_OFFSET 0x005C
#define DICE_REGISTER_GLOBAL_VERSION_OFFSET 0x0060
#define DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES_OFFSET 0x0064
#define DICE_REGISTER_GLOBAL_CLOCKSOURCENAMES_OFFSET 0x0068

//  TX_PAR_SPACE registers
#define DICE_REGISTER_TX_NUMBER_TX_OFFSET 0x0000
#define DICE_REGISTER_TX_SIZE_TX_OFFSET 0x0004

#define DICE_REGISTER_TX_ISOCHRONOUS_BASE_OFFSET 0x0008
#define DICE_REGISTER_TX_NUMBER_AUDIO_BASE_OFFSET 0x000C
#define DICE_REGISTER_TX_MIDI_BASE_OFFSET 0x0010
#define DICE_REGISTER_TX_SPEED_BASE_OFFSET 0x0014
#define DICE_REGISTER_TX_NAMES_BASE_OFFSET 0x0018
#define DICE_REGISTER_TX_AC3_CAPABILITIES_BASE_OFFSET 0x0118
#define DICE_REGISTER_TX_AC3_ENABLE_BASE_OFFSET 0x011C

#define DICE_REGISTER_TX_PARAM(size, i, offset) (((i) * (size)) + (offset))

//  RX_PAR_SPACE registers
#define DICE_REGISTER_RX_NUMBER_RX_OFFSET 0x0000
#define DICE_REGISTER_RX_SIZE_RX_OFFSET 0x0004

#ifdef DICE_VER_1_0_7_0
#define DICE_REGISTER_RX_ISOCHRONOUS_BASE_OFFSET 0x0008
#define DICE_REGISTER_RX_SEQ_START_BASE_OFFSET 0x000C
#define DICE_REGISTER_RX_NUMBER_AUDIO_BASE_OFFSET 0x0010
#define DICE_REGISTER_RX_MIDI_BASE_OFFSET 0x0014
#define DICE_REGISTER_RX_NAMES_BASE_OFFSET 0x0018
#define DICE_REGISTER_RX_AC3_CAPABILITIES_BASE_OFFSET 0x0118
#define DICE_REGISTER_RX_AC3_ENABLE_BASE_OFFSET 0x011C
#endif

#define DICE_REGISTER_RX_PARAM(size, i, offset) (((i) * (size)) + (offset))

//  GPCSR registers (Base: 0xC700_0000)
#define DICE_REGISTER_GPCSR_BASE_OFFSET 0xC7000000ULL
#define DICE_REGISTER_GPCSR_BASE                                               \
  DICE_REGISTER_BASE + DICE_REGISTER_GPCSR_BASE_OFFSET

#define DICE_REGISTER_GPCSR_AUDIO_SELECT_OFFSET 0x0004
#define DICE_REGISTER_GPCSR_CHIP_ID_OFFSET 0x0014

// GPCSR_CHIP_ID bitfields
#define DICE_GPCSR_CHIP_ID_CHIP_ID_MASK 0xFF000000UL
#define DICE_GPCSR_CHIP_ID_CHIP_ID_SHIFT 24
#define DICE_GPCSR_CHIP_ID_CHIP_TYPE_MASK 0x0000000FUL
#define DICE_GPCSR_CHIP_ID_CHIP_TYPE_SHIFT 0

// GPCSR_AUDIO_SELECT bitfields
#define DICE_GPCSR_AUDIO_SELECT_AO1_3_BIT (1UL << 15)
#define DICE_GPCSR_AUDIO_SELECT_AO1_2_BIT (1UL << 14)
#define DICE_GPCSR_AUDIO_SELECT_AO0_3_BIT (1UL << 11)
#define DICE_GPCSR_AUDIO_SELECT_AO0_2_BIT (1UL << 10)
#define DICE_GPCSR_AUDIO_SELECT_AO0_1_BIT (1UL << 9)
#define DICE_GPCSR_AUDIO_SELECT_AO0_0_BIT (1UL << 8)
#define DICE_GPCSR_AUDIO_SELECT_AES3_RX_BIT (1UL << 3)
#define DICE_GPCSR_AUDIO_SELECT_AES2_RX_BIT (1UL << 2)
#define DICE_GPCSR_AUDIO_SELECT_AES1_RX_BIT (1UL << 1)
#define DICE_GPCSR_AUDIO_SELECT_AES0_RX_BIT (1UL << 0)

//  DICE Sub System registers (Base: 0xCE00_0000)
#define DICE_REGISTER_DICE_SUB_SYSTEM_BASE_OFFSET 0xCE000000ULL
#define DICE_REGISTER_DICE_SUB_SYSTEM_BASE                                     \
  DICE_REGISTER_BASE + DICE_REGISTER_DICE_SUB_SYSTEM_BASE_OFFSET

#define DICE_REGISTER_ROUTER_CTRL_OFFSET 0x0000

// ROUTER_CTRL bitfields
#define DICE_ROUTER_CTRL_COUNT_MASK 0x0000FF00UL
#define DICE_ROUTER_CTRL_COUNT_SHIFT 8

//  Clock Controller registers (Base: 0xCE01_0000 within DICE Sub System)
#define DICE_REGISTER_CLOCK_CONTROLLER_BASE_OFFSET 0x010000ULL
#define DICE_REGISTER_CLOCK_CONTROLLER_BASE                                    \
  DICE_REGISTER_DICE_SUB_SYSTEM_BASE +                                         \
      DICE_REGISTER_CLOCK_CONTROLLER_BASE_OFFSET

#define DICE_REGISTER_CLOCK_CONTROLLER_SYNC_CTRL_OFFSET 0x0000
#define DICE_REGISTER_CLOCK_CONTROLLER_DOMAIN_CTRL_OFFSET 0x0004

// SYNC_CTRL bitfields
#define DICE_CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_MASK 0x00000003UL
#define DICE_CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_SHIFT 0

// DOMAIN_CTRL bitfields
#define DICE_CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_MASK 0x00000030UL
#define DICE_CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_SHIFT 4

//  AES Receiver registers (Base: 0xCE02_0000 within DICE Sub System)
#define DICE_REGISTER_AES_RECEIVER_BASE_OFFSET 0x020000ULL
#define DICE_REGISTER_AES_RECEIVER_BASE                                        \
  DICE_REGISTER_DICE_SUB_SYSTEM_BASE + DICE_REGISTER_AES_RECEIVER_BASE_OFFSET

#define DICE_REGISTER_AES_RECEIVER_STAT_ALL_OFFSET 0x0004

// STAT_ALL bitfields
#define DICE_AES_RECEIVER_STAT_ALL_LOCK_BIT (1UL << 0)

//  Audio Mixer registers (Base: 0xCE06_0000 within DICE Sub System)
#define DICE_REGISTER_AUDIO_MIXER_BASE_OFFSET 0x060000ULL
#define DICE_REGISTER_AUDIO_MIXER_BASE                                         \
  DICE_REGISTER_DICE_SUB_SYSTEM_BASE + DICE_REGISTER_AUDIO_MIXER_BASE_OFFSET

#define DICE_REGISTER_AUDIO_MIXER_NUMOFCH_OFFSET 0x0008

//  AVS Sub System registers (Base: 0xCF00_0000)
#define DICE_REGISTER_AVS_SUB_SYSTEM_BASE_OFFSET 0xCF000000ULL
#define DICE_REGISTER_AVS_SUB_SYSTEM_BASE                                      \
  DICE_REGISTER_BASE + DICE_REGISTER_AVS_SUB_SYSTEM_BASE_OFFSET

// AVS Audio Receiver registers (Base: 0xCF00_0000 within AVS Sub System)
#define DICE_REGISTER_AVS_AUDIO_RECEIVER_BASE_OFFSET 0x000000ULL
#define DICE_REGISTER_AVS_AUDIO_RECEIVER_BASE                                  \
  DICE_REGISTER_AVS_SUB_SYSTEM_BASE +                                          \
      DICE_REGISTER_AVS_AUDIO_RECEIVER_BASE_OFFSET

#define DICE_REGISTER_AVS_AUDIO_RECEIVER_CFG0_OFFSET 0x0000
#define DICE_REGISTER_AVS_AUDIO_RECEIVER_CFG1_OFFSET 0x0004

// ARXn_CFG0 bitfields
#define DICE_AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_MASK 0x0000003FUL
#define DICE_AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_SHIFT 0

// ARXn_CFG1 bitfields
#define DICE_AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_MASK 0x01FE0000UL
#define DICE_AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_SHIFT 21

// AVS Audio Transmitter registers (Base: 0xCF00_00C0 within AVS Sub System)
#define DICE_REGISTER_AVS_AUDIO_TRANSMITTER_BASE_OFFSET 0x0000C0ULL
#define DICE_REGISTER_AVS_AUDIO_TRANSMITTER_BASE                               \
  DICE_REGISTER_AVS_SUB_SYSTEM_BASE +                                          \
      DICE_REGISTER_AVS_AUDIO_TRANSMITTER_BASE_OFFSET

#define DICE_REGISTER_AVS_AUDIO_TRANSMITTER_CFG 0x0000

// ATXn_CFG bitfields
#define DICE_AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_MASK 0x000001F0UL
#define DICE_AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_SHIFT 4
#define DICE_AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_MASK 0x00300000UL
#define DICE_AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_SHIFT 20

// Register Bitfields
//  GLOBAL_PAR_SPACE registers
//   OWNER register defines
#define DICE_OWNER_NO_OWNER 0xFFFF000000000000LLU

//   NOTIFICATION register defines
#define DICE_NOTIFY_RX_CFG_CHG_BIT (1UL << 0)
#define DICE_NOTIFY_TX_CFG_CHG_BIT (1UL << 1)
#define DICE_NOTIFY_DUP_ISOCHRONOUS_BIT (1UL << 2)
#define DICE_NOTIFY_BW_ERR_BIT (1UL << 3)
#define DICE_NOTIFY_LOCK_CHG_BIT (1UL << 4)
#define DICE_NOTIFY_CLOCK_ACCEPTED (1UL << 5)
#define DICE_INTERFACE_CHG_BIT (1UL << 6)

// User notifications (Defined in )
#define DICE_NOTIFY_RESERVED1 (1UL << 16)
#define DICE_NOTIFY_RESERVED2 (1UL << 17)
#define DICE_NOTIFY_RESERVED3 (1UL << 18)
#define DICE_NOTIFY_RESERVED4 (1UL << 19)
#define DICE_NOTIFY_MESSAGE (1UL << 20)
#define DICE_NOTIFY_USER1 (1UL << 21)
#define DICE_NOTIFY_USER2 (1UL << 22)
#define DICE_NOTIFY_USER3 (1UL << 23)
#define DICE_NOTIFY_USER4 (1UL << 24)
#define DICE_NOTIFY_USER5 (1UL << 25)
#define DICE_NOTIFY_USER6 (1UL << 26)
#define DICE_NOTIFY_USER7 (1UL << 27)
#define DICE_NOTIFY_USER8 (1UL << 28)
#define DICE_NOTIFY_USER9 (1UL << 29)
#define DICE_NOTIFY_USER10 (1UL << 30)
#define DICE_NOTIFY_USER11 (1UL << 31)

#define DICE_NOTIFY_USER_IS_MESSAGE(x) (((x) & DICE_NOTIFY_MESSAGE) != 0)

#define DICE_NOTIFY_USER_GET_MESSAGE(x) (((x) >> 24) & 0xFF)

//   NICK_NAME register
// NOTE: in bytes
#define DICE_NICK_NAME_SIZE 64 // Defined in

//   CLOCK_SELECT register
// Clock sources supported
#define DICE_CLOCKSOURCE_AES1 0x00
#define DICE_CLOCKSOURCE_AES2 0x01
#define DICE_CLOCKSOURCE_AES3 0x02
#define DICE_CLOCKSOURCE_AES4 0x03
#define DICE_CLOCKSOURCE_AES_ANY 0x04
#define DICE_CLOCKSOURCE_ADAT 0x05
#define DICE_CLOCKSOURCE_TDIF 0x06
#define DICE_CLOCKSOURCE_WC 0x07
#define DICE_CLOCKSOURCE_ARX1 0x08
#define DICE_CLOCKSOURCE_ARX2 0x09
#define DICE_CLOCKSOURCE_ARX3 0x0A
#define DICE_CLOCKSOURCE_ARX4 0x0B
#define DICE_CLOCKSOURCE_INTERNAL 0x0C
#define DICE_CLOCKSOURCE_COUNT (DICE_CLOCKSOURCE_INTERNAL + 1)

#define DICE_CLOCKSOURCE_MASK 0x0000FFFFLU
#define DICE_GET_CLOCKSOURCE(reg) (((reg) & DICE_CLOCKSOURCE_MASK))
#define DICE_SET_CLOCKSOURCE(reg, clk)                                         \
  (((reg) & ~DICE_CLOCKSOURCE_MASK) | ((clk) & DICE_CLOCKSOURCE_MASK))

// Supported rates
#define DICE_RATE_32K 0x00
#define DICE_RATE_44K1 0x01
#define DICE_RATE_48K 0x02
#define DICE_RATE_88K2 0x03
#define DICE_RATE_96K 0x04
#define DICE_RATE_176K4 0x05
#define DICE_RATE_192K 0x06
#define DICE_RATE_ANY_LOW 0x07
#define DICE_RATE_ANY_MID 0x08
#define DICE_RATE_ANY_HIGH 0x09
#define DICE_RATE_NONE 0x0A

#define DICE_RATE_MASK 0x0000FF00LU
#define DICE_GET_RATE(reg) (((reg) & DICE_RATE_MASK) >> 8)
#define DICE_SET_RATE(reg, rate)                                               \
  (((reg) & ~DICE_RATE_MASK) | (((rate) << 8) & DICE_RATE_MASK))

//   ENABLE register
#define DICE_ISOSTREAMING_ENABLE (1UL << 0)
#define DICE_ISOSTREAMING_DISABLE (0)

//   CLOCK_STATUS register
#define DICE_STATUS_SOURCE_LOCKED (1UL << 0)
#define DICE_STATUS_RATE_CONFLICT (1UL << 1)

#define DICE_STATUS_GET_NOMINAL_RATE(x) (((x) >> 8) & 0xFF)

//   EXTENDED_STATUS register
#define DICE_EXT_STATUS_AES0_LOCKED (1UL << 0)
#define DICE_EXT_STATUS_AES1_LOCKED (1UL << 1)
#define DICE_EXT_STATUS_AES2_LOCKED (1UL << 2)
#define DICE_EXT_STATUS_AES3_LOCKED (1UL << 3)
#define DICE_EXT_STATUS_AES_ANY_LOCKED (0x0F)
#define DICE_EXT_STATUS_ADAT_LOCKED (1UL << 4)
#define DICE_EXT_STATUS_TDIF_LOCKED (1UL << 5)
#define DICE_EXT_STATUS_ARX1_LOCKED (1UL << 6)
#define DICE_EXT_STATUS_ARX2_LOCKED (1UL << 7)
#define DICE_EXT_STATUS_ARX3_LOCKED (1UL << 8)
#define DICE_EXT_STATUS_ARX4_LOCKED (1UL << 9)
#define DICE_EXT_STATUS_WC_LOCKED (1UL << 10)

#define DICE_EXT_STATUS_AES0_SLIP (1UL << 16)
#define DICE_EXT_STATUS_AES1_SLIP (1UL << 17)
#define DICE_EXT_STATUS_AES2_SLIP (1UL << 18)
#define DICE_EXT_STATUS_AES3_SLIP (1UL << 19)
#define DICE_EXT_STATUS_ADAT_SLIP (1UL << 20)
#define DICE_EXT_STATUS_TDIF_SLIP (1UL << 21)
#define DICE_EXT_STATUS_ARX1_SLIP (1UL << 22)
#define DICE_EXT_STATUS_ARX2_SLIP (1UL << 23)
#define DICE_EXT_STATUS_ARX3_SLIP (1UL << 24)
#define DICE_EXT_STATUS_ARX4_SLIP (1UL << 25)
#define DICE_EXT_STATUS_WC_SLIP (1UL << 26)

//   VERSION register
#define DICE_DRIVER_SPEC_VERSION_NUMBER_GET(x, y) (((x) >> (y)) & 0xFF)

#define DICE_DRIVER_SPEC_VERSION_NUMBER_GET_A(x)                               \
  DICE_DRIVER_SPEC_VERSION_NUMBER_GET(x, 24)

#define DICE_DRIVER_SPEC_VERSION_NUMBER_GET_B(x)                               \
  DICE_DRIVER_SPEC_VERSION_NUMBER_GET(x, 16)

#define DICE_DRIVER_SPEC_VERSION_NUMBER_GET_C(x)                               \
  DICE_DRIVER_SPEC_VERSION_NUMBER_GET(x, 8)

#define DICE_DRIVER_SPEC_VERSION_NUMBER_GET_D(x)                               \
  DICE_DRIVER_SPEC_VERSION_NUMBER_GET(x, 0)

//   CLOCKCAPABILITIES register ()
#define DICE_CLOCKCAP_RATE_32K (1UL << 0)
#define DICE_CLOCKCAP_RATE_44K1 (1UL << 1)
#define DICE_CLOCKCAP_RATE_48K (1UL << 2)
#define DICE_CLOCKCAP_RATE_88K2 (1UL << 3)
#define DICE_CLOCKCAP_RATE_96K (1UL << 4)
#define DICE_CLOCKCAP_RATE_176K4 (1UL << 5)
#define DICE_CLOCKCAP_RATE_192K (1UL << 6)
#define DICE_CLOCKCAP_SOURCE_AES1 (1UL << 16)
#define DICE_CLOCKCAP_SOURCE_AES2 (1UL << 17)
#define DICE_CLOCKCAP_SOURCE_AES3 (1UL << 18)
#define DICE_CLOCKCAP_SOURCE_AES4 (1UL << 19)
#define DICE_CLOCKCAP_SOURCE_AES_ANY (1UL << 20)
#define DICE_CLOCKCAP_SOURCE_ADAT (1UL << 21)
#define DICE_CLOCKCAP_SOURCE_TDIF (1UL << 22)
#define DICE_CLOCKCAP_SOURCE_WORDCLOCK (1UL << 23)
#define DICE_CLOCKCAP_SOURCE_ARX1 (1UL << 24)
#define DICE_CLOCKCAP_SOURCE_ARX2 (1UL << 25)
#define DICE_CLOCKCAP_SOURCE_ARX3 (1UL << 26)
#define DICE_CLOCKCAP_SOURCE_ARX4 (1UL << 27)
#define DICE_CLOCKCAP_SOURCE_INTERNAL (1UL << 28)

//   CLOCKSOURCENAMES
// note: in bytes
#define DICE_CLOCKSOURCENAMES_SIZE 256

//  TX_PAR_SPACE registers
// note: in bytes
#define DICE_TX_NAMES_SIZE 256

//  RX_PAR_SPACE registers
// note: in bytes
#define DICE_RX_NAMES_SIZE 256

// EAP (Extended Application Protocol) Definitions
// EAP (Extended Application Protocol) Base and Transformation
#define DICE_EAP_BASE 0x0000000000200000ULL
#define DICE_EAP_MAX_SIZE 0x0000000000F00000ULL
#define DICE_EAP_TRANSFORM_BASE 0xFFFFE0200000ULL

// EAP Section Definitions
#define DICE_EAP_CAPABILITY_SPACE_OFFSET 0x0000
#define DICE_EAP_CAPABILITY_SPACE_SIZE 0x0004
#define DICE_EAP_CMD_SPACE_OFFSET 0x0008
#define DICE_EAP_CMD_SPACE_SIZE 0x000C
#define DICE_EAP_MIXER_SPACE_OFFSET 0x0010
#define DICE_EAP_MIXER_SPACE_SIZE 0x0014
#define DICE_EAP_PEAK_SPACE_OFFSET 0x0018
#define DICE_EAP_PEAK_SPACE_SIZE 0x001C
#define DICE_EAP_NEW_ROUTING_SPACE_OFFSET 0x0020
#define DICE_EAP_NEW_ROUTING_SPACE_SIZE 0x0024
#define DICE_EAP_NEW_STREAM_CFG_SPACE_OFFSET 0x0028
#define DICE_EAP_NEW_STREAM_CFG_SPACE_SIZE 0x002C
#define DICE_EAP_CURR_CFG_SPACE_OFFSET 0x0030
#define DICE_EAP_CURR_CFG_SPACE_SIZE 0x0034
#define DICE_EAP_STAND_ALONE_CFG_SPACE_OFFSET 0x0038
#define DICE_EAP_STAND_ALONE_CFG_SPACE_SIZE 0x003C
#define DICE_EAP_APP_SPACE_OFFSET 0x0040
#define DICE_EAP_APP_SPACE_SIZE 0x0044
#define DICE_EAP_ZERO_MARKER_1 0x0048

// EAP Section Identifiers
enum class EAPSection {
  Capability = 0,
  Command = 1,
  Mixer = 2,
  Peak = 3,
  NewRouting = 4,
  NewStreamConfig = 5,
  CurrentConfig = 6,
  StandaloneConfig = 7,
  Application = 8,
  Unknown = 0xFF
};

// EAP Section Info Structure
struct EAPSectionInfo {
  uint32_t offset;
  uint32_t size;
  const char *name;
  bool isReadOnly;
};

// EAP Section Mapping
static const EAPSectionInfo EAP_SECTION_MAP[] = {
    {DICE_EAP_CAPABILITY_SPACE_OFFSET, DICE_EAP_CAPABILITY_SPACE_SIZE,
     "Capability", true},
    {DICE_EAP_CMD_SPACE_OFFSET, DICE_EAP_CMD_SPACE_SIZE, "Command", false},
    {DICE_EAP_MIXER_SPACE_OFFSET, DICE_EAP_MIXER_SPACE_SIZE, "Mixer", false},
    {DICE_EAP_PEAK_SPACE_OFFSET, DICE_EAP_PEAK_SPACE_SIZE, "Peak", true},
    {DICE_EAP_NEW_ROUTING_SPACE_OFFSET, DICE_EAP_NEW_ROUTING_SPACE_SIZE,
     "NewRouting", false},
    {DICE_EAP_NEW_STREAM_CFG_SPACE_OFFSET, DICE_EAP_NEW_STREAM_CFG_SPACE_SIZE,
     "NewStreamConfig", false},
    {DICE_EAP_CURR_CFG_SPACE_OFFSET, DICE_EAP_CURR_CFG_SPACE_SIZE,
     "CurrentConfig", true},
    {DICE_EAP_STAND_ALONE_CFG_SPACE_OFFSET, DICE_EAP_STAND_ALONE_CFG_SPACE_SIZE,
     "StandaloneConfig", false},
    {DICE_EAP_APP_SPACE_OFFSET, DICE_EAP_APP_SPACE_SIZE, "Application", false}};

// CAPABILITY registers
#define DICE_EAP_CAPABILITY_ROUTER_OFFSET 0x0000
#define DICE_EAP_CAPABILITY_MIXER_OFFSET 0x0004
#define DICE_EAP_CAPABILITY_GENERAL_OFFSET 0x0008
#define DICE_EAP_CAPABILITY_RESERVED_OFFSET 0x000C

// CAPABILITY bit definitions
#define DICE_EAP_CAP_ROUTER_EXPOSED 0
#define DICE_EAP_CAP_ROUTER_READONLY 1
#define DICE_EAP_CAP_ROUTER_FLASHSTORED 2
#define DICE_EAP_CAP_ROUTER_MAXROUTES 16

#define DICE_EAP_CAP_MIXER_EXPOSED 0
#define DICE_EAP_CAP_MIXER_READONLY 1
#define DICE_EAP_CAP_MIXER_FLASHSTORED 2
#define DICE_EAP_CAP_MIXER_IN_DEV 4
#define DICE_EAP_CAP_MIXER_OUT_DEV 8
#define DICE_EAP_CAP_MIXER_INPUTS 16
#define DICE_EAP_CAP_MIXER_OUTPUTS 24

#define DICE_EAP_CAP_GENERAL_STRM_CFG_EN 0
#define DICE_EAP_CAP_GENERAL_FLASH_EN 1
#define DICE_EAP_CAP_GENERAL_PEAK_EN 2
#define DICE_EAP_CAP_GENERAL_MAX_TX_STREAM 4
#define DICE_EAP_CAP_GENERAL_MAX_RX_STREAM 8
#define DICE_EAP_CAP_GENERAL_STRM_CFG_FLS 12
#define DICE_EAP_CAP_GENERAL_CHIP 16

#define DICE_EAP_CAP_GENERAL_CHIP_DICEII 0
#define DICE_EAP_CAP_GENERAL_CHIP_DICEMINI 1
#define DICE_EAP_CAP_GENERAL_CHIP_DICEJR 2

// COMMAND registers
#define DICE_EAP_COMMAND_OPCODE_OFFSET 0x0000
#define DICE_EAP_COMMAND_RETVAL_OFFSET 0x0004

// opcodes
#define DICE_EAP_CMD_OPCODE_NO_OP 0x0000
#define DICE_EAP_CMD_OPCODE_LD_ROUTER 0x0001
#define DICE_EAP_CMD_OPCODE_LD_STRM_CFG 0x0002
#define DICE_EAP_CMD_OPCODE_LD_RTR_STRM_CFG 0x0003
#define DICE_EAP_CMD_OPCODE_LD_FLASH_CFG 0x0004
#define DICE_EAP_CMD_OPCODE_ST_FLASH_CFG 0x0005

#define DICE_EAP_CMD_OPCODE_FLAG_LD_LOW (1U << 16)
#define DICE_EAP_CMD_OPCODE_FLAG_LD_MID (1U << 17)
#define DICE_EAP_CMD_OPCODE_FLAG_LD_HIGH (1U << 18)
#define DICE_EAP_CMD_OPCODE_FLAG_LD_EXECUTE (1U << 31)

// CURRENT CFG registers
#define DICE_EAP_CURRCFG_LOW_ROUTER_OFFSET 0x0000
#define DICE_EAP_CURRCFG_LOW_STREAM_OFFSET 0x1000
#define DICE_EAP_CURRCFG_MID_ROUTER_OFFSET 0x2000
#define DICE_EAP_CURRCFG_MID_STREAM_OFFSET 0x3000
#define DICE_EAP_CURRCFG_HIGH_ROUTER_OFFSET 0x4000
#define DICE_EAP_CURRCFG_HIGH_STREAM_OFFSET 0x5000

#define DICE_EAP_CHANNEL_CONFIG_NAMESTR_LEN_QUADS (64)
#define DICE_EAP_CHANNEL_CONFIG_NAMESTR_LEN_BYTES                              \
  (4 * DICE_EAP_CHANNEL_CONFIG_NAMESTR_LEN_QUADS)

// DICE Notifier
#define DICE_NOTIFIER_BASE_ADDRESS 0xFFFFE00000000000ULL
#define DICE_NOTIFIER_BLOCK_LENGTH 4

// Clock sources for SYNC_CTRL

enum class ClockSourceEnum {
  AES0 = 0,
  AES1 = 1,
  AES2 = 2,
  AES3 = 3,
  SlaveInputs = 4,
  HPLL = 5,
  Internal = 6,
  Unknown = 0xFF
};

enum eClockSourceType {
  eCT_Invalid,   ///> invalid entry (e.g. on error)
  eCT_Auto,      ///> automatically select clock source
  eCT_Internal,  ///> internal sync (unspecified)
  eCT_1394Bus,   ///> Sync on the 1394 bus clock (e.g. CSP)
  eCT_SytMatch,  ///> SYT match on incoming audio stream
  eCT_SytStream, ///> SYT match on incoming sync stream
  eCT_WordClock, ///> SYT on WordClock input
  eCT_SPDIF,     ///> SYT on SPDIF input
  eCT_ADAT,      ///> SYT on ADAT input
  eCT_TDIF,      ///> SYT on TDIF input
  eCT_AES,       ///> SYT on AES input
  eCT_SMPTE,     ///> SMPTE clock
};

class ClockSource {
public:
  ClockSource()
      : type(eCT_Invalid), id(0), valid(false), active(false), locked(true),
        slipping(false), description("") {};
  /// indicates the type of the clock source (e.g. eCT_ADAT)
  enum eClockSourceType type;
  /// indicated the id of the clock source (e.g. id=1 => clocksource is ADAT_1)
  unsigned int id;
  /// is the clock source valid (i.e. can be selected) at this moment?
  bool valid;
  /// is the clock source active at this moment?
  bool active;
  /// is the clock source locked?
  bool locked;
  /// is the clock source slipping?
  bool slipping;
  /// description of the clock struct (optional)
  std::string description;

  bool operator==(const ClockSource &x) {
    return (type == x.type) && (id == x.id);
  }
};

// AVS System Modes for ATXn_CFG
enum class AvsSystemMode {
  Low = 0,  // 32k-48k
  Mid = 1,  // 88.2k-96k
  High = 2, // 176.4k-192k
  Unknown = 0xFF
};

// Router Frame Sync modes for DOMAIN_CTRL
enum class RouterFrameSyncMode {
  BaseRate = 0,
  DoubleRate = 1,
  QuadRate = 2,
  Unknown = 0xFF
};

// DICE chip types
enum class DiceChipType {
  DiceII = 0,
  DiceMini = 1,
  DiceJr = 2,
  Unknown = 0xFF
};

// DICE configuration types
enum class DiceConfig {
  Unknown,
  Low, // 32k-48k
  Mid, // 88.2k-96k
  High // 176.4k-192k
};

// Route Sources (Inputs to the router matrix)
enum class RouteSource : uint8_t {
  AES = 0,
  ADAT = 1,
  // 2, 3 reserved?
  Mixer = 4,
  // 5 reserved?
  InS0 = 6,  // Input Stream 0 (from device physical inputs)
  InS1 = 7,  // Input Stream 1 (from device physical inputs - DiceJr?)
  ARM = 8,   // ARM processor audio
  ARX0 = 9,  // Audio Receiver 0 (FireWire input)
  ARX1 = 10, // Audio Receiver 1 (FireWire input)
  // 11-14 reserved?
  Muted = 15,
  Invalid = 0xFF
};

// Route Destinations (Outputs from the router matrix)
enum class RouteDestination : uint8_t {
  AES = 0,
  ADAT = 1,
  // 2 reserved?
  Mixer0 = 3, // Mixer Input Bank 0
  Mixer1 = 4, // Mixer Input Bank 1
  InS0 = 5,   // Input Stream 0 (to device physical outputs)
  InS1 = 6,   // Input Stream 1 (to device physical outputs - DiceJr?)
  ARM = 7,    // ARM processor audio
  ATX0 = 8,   // Audio Transmitter 0 (FireWire output)
  ATX1 = 9,   // Audio Transmitter 1 (FireWire output)
  // 10-14 reserved?
  Muted = 15,
  Invalid = 0xFF
};

} // namespace DICE
} // namespace FWA
