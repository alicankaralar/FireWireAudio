// DiceAbsoluteAddresses.hpp
// Absolute register addresses and offsets for DICE chip (Reorganized)
#pragma once

#include "DiceDefines.hpp"
#include <cstdint>

namespace FWA {
namespace DICE {
// --- Base Addresses ---
// Using macros from DiceDefines.hpp directly
// These will be used within the DICE namespace

// --- Pointer Block Offsets ---
// Offsets for reading pointers to actual register spaces
#define DICE_OFFSET_GLOBAL_SPACE_PTR_HI                                        \
  0x00 // High 32 bits of Global register space pointer
#define DICE_OFFSET_GLOBAL_SPACE_PTR_LO                                        \
  0x04 // Low 32 bits of Global register space pointer
#define DICE_OFFSET_TX_SPACE_PTR_HI(idx)                                       \
  (0x08 + (idx) * 0x08) // High 32 bits of TX[idx] register space pointer
#define DICE_OFFSET_TX_SPACE_PTR_LO(idx)                                       \
  (0x0C + (idx) * 0x08) // Low 32 bits of TX[idx] register space pointer
#define DICE_OFFSET_RX_SPACE_PTR_HI(idx)                                       \
  (0x18 + (idx) * 0x08) // High 32 bits of RX[idx] register space pointer
#define DICE_OFFSET_RX_SPACE_PTR_LO(idx)                                       \
  (0x1C + (idx) * 0x08) // Low 32 bits of RX[idx] register space pointer

// --- Register Offsets & Related Bitfields/Masks/Shifts ---
// Using DICE_INVALID_OFFSET from DiceDefines.hpp

// GPCSR Offsets (relative to DICE_REGISTER_GPCSR_BASE)
#define GPCSR_AUDIO_SELECT_OFFSET DICE_REGISTER_GPCSR_AUDIO_SELECT
#define GPCSR_CHIP_ID_OFFSET DICE_REGISTER_GPCSR_CHIP_ID
// GPCSR_CHIP_ID bitfields
#define GPCSR_CHIP_ID_CHIP_ID_MASK DICE_GPCSR_CHIP_ID_CHIP_ID_MASK
#define GPCSR_CHIP_ID_CHIP_ID_SHIFT DICE_GPCSR_CHIP_ID_CHIP_ID_SHIFT
#define GPCSR_CHIP_ID_CHIP_TYPE_MASK DICE_GPCSR_CHIP_ID_CHIP_TYPE_MASK
#define GPCSR_CHIP_ID_CHIP_TYPE_SHIFT DICE_GPCSR_CHIP_ID_CHIP_TYPE_SHIFT

// Clock Controller Offsets (relative to DICE_REGISTER_CLOCK_CONTROLLER_BASE)
#define CLOCK_CONTROLLER_SYNC_CTRL_OFFSET                                      \
  DICE_REGISTER_CLOCK_CONTROLLER_SYNC_CTRL
#define CLOCK_CONTROLLER_DOMAIN_CTRL_OFFSET                                    \
  DICE_REGISTER_CLOCK_CONTROLLER_DOMAIN_CTRL
// CLOCK_CONTROLLER_SYNC_CTRL bitfields
#define CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_MASK                               \
  DICE_CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_MASK
#define CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_SHIFT                              \
  DICE_CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_SHIFT
// CLOCK_CONTROLLER_DOMAIN_CTRL bitfields
#define CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_MASK                               \
  DICE_CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_MASK
#define CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_SHIFT                              \
  DICE_CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_SHIFT

// AES Receiver Offsets (relative to DICE_REGISTER_AES_RECEIVER_BASE)
#define AES_RECEIVER_STAT_ALL_OFFSET DICE_REGISTER_AES_RECEIVER_STAT_ALL
#define AES_RECEIVER_STAT_ALL_LOCK_BIT DICE_AES_RECEIVER_STAT_ALL_LOCK_BIT

// Audio Mixer Offsets (relative to DICE_REGISTER_AUDIO_MIXER_BASE)
#define AUDIO_MIXER_NUMOFCH_OFFSET DICE_REGISTER_AUDIO_MIXER_NUMOFCH

// AVS Audio Receiver Offsets (relative to
// DICE_REGISTER_AVS_AUDIO_RECEIVER_BASE)
#define AVS_AUDIO_RECEIVER_CFG0_OFFSET DICE_REGISTER_AVS_AUDIO_RECEIVER_CFG0
#define AVS_AUDIO_RECEIVER_CFG1_OFFSET DICE_REGISTER_AVS_AUDIO_RECEIVER_CFG1
// ARXn_CFG0 bitfields
#define AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_MASK                                \
  DICE_AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_MASK
#define AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_SHIFT                               \
  DICE_AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_SHIFT
// ARXn_CFG1 bitfields
#define AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_MASK                             \
  DICE_AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_MASK
#define AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_SHIFT                            \
  DICE_AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_SHIFT

// AVS Audio Transmitter Offsets (relative to
// DICE_REGISTER_AVS_AUDIO_TRANSMITTER_BASE)
#define AVS_AUDIO_TRANSMITTER_CFG_OFFSET DICE_REGISTER_AVS_AUDIO_TRANSMITTER_CFG
// ATXn_CFG bitfields
#define AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_MASK                         \
  DICE_AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_MASK
#define AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_SHIFT                        \
  DICE_AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_SHIFT
#define AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_MASK                                \
  DICE_AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_MASK
#define AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_SHIFT                               \
  DICE_AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_SHIFT

// TX Parameter Space Offsets (relative to DICE_REGISTER_TX_BASE)
#define REGISTER_TX_PAR_SPACE_OFF DICE_REGISTER_TX_ISOC_BASE

// RX Parameter Space Offsets (relative to DICE_REGISTER_RX_BASE)
#define REGISTER_RX_PAR_SPACE_OFF DICE_REGISTER_RX_NB_AUDIO_BASE

// --- Absolute Addresses ---

// GLOBAL_PAR_SPACE absolute addresses (relative to DICE_REGISTER_BASE)
#define GLOBAL_OWNER_ADDR (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_OWNER)
#define GLOBAL_NOTIFICATION_ADDR                                               \
  (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_NOTIFICATION)
#define GLOBAL_NICK_NAME_ADDR                                                  \
  (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_NICK_NAME)
#define GLOBAL_CLOCK_SELECT_ADDR                                               \
  (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_CLOCK_SELECT)
#define GLOBAL_ENABLE_ADDR (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_ENABLE)
#define GLOBAL_STATUS_ADDR (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_STATUS)
#define GLOBAL_EXTENDED_STATUS_ADDR                                            \
  (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_EXTENDED_STATUS)
#define GLOBAL_SAMPLE_RATE_ADDR                                                \
  (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_SAMPLE_RATE)
#define GLOBAL_VERSION_ADDR (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_VERSION)
#define GLOBAL_CLOCKCAPABILITIES_ADDR                                          \
  (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES)
#define GLOBAL_CLOCKSOURCENAMES_ADDR                                           \
  (DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_CLOCKSOURCENAMES)

// TX_PAR_SPACE absolute addresses (relative to DICE_REGISTER_TX_BASE)
#define TX_NB_TX_ADDR (DICE_REGISTER_TX_BASE + DICE_REGISTER_TX_NB_TX)
#define TX_SZ_TX_ADDR (DICE_REGISTER_TX_BASE + DICE_REGISTER_TX_SZ_TX)
#define TX_ISOC_BASE_ADDR (DICE_REGISTER_TX_BASE + DICE_REGISTER_TX_ISOC_BASE)
#define TX_NB_AUDIO_BASE_ADDR                                                  \
  (DICE_REGISTER_TX_BASE + DICE_REGISTER_TX_NB_AUDIO_BASE)
#define TX_MIDI_BASE_ADDR (DICE_REGISTER_TX_BASE + DICE_REGISTER_TX_MIDI_BASE)
#define TX_SPEED_BASE_ADDR (DICE_REGISTER_TX_BASE + DICE_REGISTER_TX_SPEED_BASE)
#define TX_NAMES_BASE_ADDR (DICE_REGISTER_TX_BASE + DICE_REGISTER_TX_NAMES_BASE)
#define TX_AC3_CAPABILITIES_BASE_ADDR                                          \
  (DICE_REGISTER_TX_BASE + DICE_REGISTER_TX_AC3_CAPABILITIES_BASE)
#define TX_AC3_ENABLE_BASE_ADDR                                                \
  (DICE_REGISTER_TX_BASE + DICE_REGISTER_TX_AC3_ENABLE_BASE)

// RX_PAR_SPACE absolute addresses (relative to DICE_REGISTER_RX_BASE)
#define RX_NB_RX_ADDR (DICE_REGISTER_RX_BASE + DICE_REGISTER_RX_NB_RX)
#define RX_SZ_RX_ADDR (DICE_REGISTER_RX_BASE + DICE_REGISTER_RX_SZ_RX)
#define RX_ISOC_BASE_ADDR (DICE_REGISTER_RX_BASE + DICE_REGISTER_RX_ISOC_BASE)
#define RX_SEQ_START_BASE_ADDR                                                 \
  (DICE_REGISTER_RX_BASE + DICE_REGISTER_RX_SEQ_START_BASE)
#define RX_NB_AUDIO_BASE_ADDR                                                  \
  (DICE_REGISTER_RX_BASE + DICE_REGISTER_RX_NB_AUDIO_BASE)
#define RX_MIDI_BASE_ADDR (DICE_REGISTER_RX_BASE + DICE_REGISTER_RX_MIDI_BASE)
#define RX_NAMES_BASE_ADDR (DICE_REGISTER_RX_BASE + DICE_REGISTER_RX_NAMES_BASE)
#define RX_AC3_CAPABILITIES_BASE_ADDR                                          \
  (DICE_REGISTER_RX_BASE + DICE_REGISTER_RX_AC3_CAPABILITIES_BASE)
#define RX_AC3_ENABLE_BASE_ADDR                                                \
  (DICE_REGISTER_RX_BASE + DICE_REGISTER_RX_AC3_ENABLE_BASE)

// EAP (Extended Application Protocol) absolute addresses (relative to
// DICE_EAP_BASE)
#define DICE_EAP_BASE_ADDR (DICE_REGISTER_BASE + DICE_EAP_BASE)
#define DICE_EAP_CAPABILITY_SPACE_ADDR                                         \
  (DICE_EAP_BASE_ADDR + DICE_EAP_CAPABILITY_SPACE_OFF)
#define DICE_EAP_CMD_SPACE_ADDR (DICE_EAP_BASE_ADDR + DICE_EAP_CMD_SPACE_OFF)
#define DICE_EAP_MIXER_SPACE_ADDR                                              \
  (DICE_EAP_BASE_ADDR + DICE_EAP_MIXER_SPACE_OFF)
#define DICE_EAP_PEAK_SPACE_ADDR (DICE_EAP_BASE_ADDR + DICE_EAP_PEAK_SPACE_OFF)
#define DICE_EAP_NEW_ROUTING_SPACE_ADDR                                        \
  (DICE_EAP_BASE_ADDR + DICE_EAP_NEW_ROUTING_SPACE_OFF)
#define DICE_EAP_NEW_STREAM_CFG_SPACE_ADDR                                     \
  (DICE_EAP_BASE_ADDR + DICE_EAP_NEW_STREAM_CFG_SPACE_OFF)
#define DICE_EAP_CURR_CFG_SPACE_ADDR                                           \
  (DICE_EAP_BASE_ADDR + DICE_EAP_CURR_CFG_SPACE_OFF)
#define DICE_EAP_STAND_ALONE_CFG_SPACE_ADDR                                    \
  (DICE_EAP_BASE_ADDR + DICE_EAP_STAND_ALONE_CFG_SPACE_OFF)
#define DICE_EAP_APP_SPACE_ADDR (DICE_EAP_BASE_ADDR + DICE_EAP_APP_SPACE_OFF)
#define DICE_EAP_ZERO_MARKER_1_ADDR                                            \
  (DICE_EAP_BASE_ADDR + DICE_EAP_ZERO_MARKER_1)

// GPCSR absolute addresses
#define GPCSR_AUDIO_SELECT_ADDR                                                \
  (DICE_REGISTER_GPCSR_BASE + GPCSR_AUDIO_SELECT_OFFSET)
#define GPCSR_CHIP_ID_ADDR (DICE_REGISTER_GPCSR_BASE + GPCSR_CHIP_ID_OFFSET)

// Clock Controller absolute addresses
#define CLOCK_CONTROLLER_SYNC_CTRL_ADDR                                        \
  (DICE_REGISTER_CLOCK_CONTROLLER_BASE + CLOCK_CONTROLLER_SYNC_CTRL_OFFSET)
#define CLOCK_CONTROLLER_DOMAIN_CTRL_ADDR                                      \
  (DICE_REGISTER_CLOCK_CONTROLLER_BASE + CLOCK_CONTROLLER_DOMAIN_CTRL_OFFSET)

// AES Receiver absolute addresses
#define AES_RECEIVER_STAT_ALL_ADDR                                             \
  (DICE_REGISTER_AES_RECEIVER_BASE + AES_RECEIVER_STAT_ALL_OFFSET)

// Audio Mixer absolute addresses
#define AUDIO_MIXER_NUMOFCH_ADDR                                               \
  (DICE_REGISTER_AUDIO_MIXER_BASE + AUDIO_MIXER_NUMOFCH_OFFSET)

// AVS Audio Receiver absolute addresses
#define AVS_AUDIO_RECEIVER_CFG0_ADDR                                           \
  (DICE_REGISTER_AVS_AUDIO_RECEIVER_BASE + AVS_AUDIO_RECEIVER_CFG0_OFFSET)
#define AVS_AUDIO_RECEIVER_CFG1_ADDR                                           \
  (DICE_REGISTER_AVS_AUDIO_RECEIVER_BASE + AVS_AUDIO_RECEIVER_CFG1_OFFSET)

// AVS Audio Transmitter absolute addresses
#define AVS_AUDIO_TRANSMITTER_CFG_ADDR                                         \
  (DICE_REGISTER_AVS_AUDIO_TRANSMITTER_BASE + AVS_AUDIO_TRANSMITTER_CFG_OFFSET)

// --- Size Definitions ---
// Using size definitions from DiceDefines.hpp directly

// --- Bitfield Masks and Constants (from DiceDefines.hpp) ---
// EAP Capabilities General Chip Types
// Using EAP capability definitions from DiceDefines.hpp directly

// --- Helper Functions ---

// Per-stream/channel address calculation helpers
#define TX_PARAM_ADDR(stream_size, i, offset)                                  \
  (DICE_REGISTER_TX_BASE + DICE_REGISTER_TX_PARAM(stream_size, i, offset))
#define RX_PARAM_ADDR(stream_size, i, offset)                                  \
  (DICE_REGISTER_RX_BASE + DICE_REGISTER_RX_PARAM(stream_size, i, offset))

// --- Field Extraction Helpers (Macros) ---
// Using macros directly from DiceDefines.hpp
// DICE_GET_CLOCKSOURCE(reg)
// DICE_SET_CLOCKSOURCE(reg, clk)
// DICE_GET_RATE(reg)
// DICE_SET_RATE(reg, rate)
// DICE_STATUS_GET_NOMINAL_RATE(x)
// DICE_NOTIFY_USER_IS_MESSAGE(x)
// DICE_NOTIFY_USER_GET_MESSAGE(x)

// --- Enums ---
// Using enum classes from DiceDefines.hpp with using declarations
// to maintain backward compatibility with existing code
} // namespace DICE

// Global using declarations for backward compatibility
using DICE::AvsSystemMode;
using DICE::ClockSource;
using DICE::DiceChipType;
using DICE::DiceConfig;
using DICE::RouteDestination;
using DICE::RouterFrameSyncMode;
using DICE::RouteSource;

// Global base address constants for backward compatibility
// These are already defined as macros in DiceDefines.hpp, so we can use them
// directly

// Additional base addresses needed by scanner

// Known channel name addresses for scanner
#define DICE_CHANNEL_NAMES_ADDR_1 0xffffe00001a8ULL
#define DICE_CHANNEL_NAMES_ADDR_2 0xffffe0000090ULL
#define DICE_CHANNEL_NAMES_ADDR_3 0xffffe0000100ULL
#define DICE_CHANNEL_NAMES_ADDR_4 0xffffe0000200ULL

// Device name address for scanner
#define DICE_DEVICE_NAME_ADDR 0xffffe0000034ULL

// High address threshold for ReadBlock vs ReadQuadlet decision
#define DICE_HIGH_ADDRESS_THRESHOLD 0xFFFFF0000000ULL

// Additional EAP capability constants needed by scanner
#define DICE_EAP_CAP_GENERAL_CHIP 16
#define GPCSR_BASE DICE_REGISTER_GPCSR_BASE
#define REGISTER_DICE_SUB_SYSTEM_BASE DICE_REGISTER_DICE_SUB_SYSTEM_BASE
#define REGISTER_CLOCK_CONTROLLER_BASE DICE_REGISTER_CLOCK_CONTROLLER_BASE
#define REGISTER_AES_RECEIVER_BASE DICE_REGISTER_AES_RECEIVER_BASE
#define REGISTER_AUDIO_MIXER_BASE DICE_REGISTER_AUDIO_MIXER_BASE
#define REGISTER_AVS_SUB_SYSTEM_BASE DICE_REGISTER_AVS_SUB_SYSTEM_BASE
#define REGISTER_AVS_AUDIO_RECEIVER_BASE DICE_REGISTER_AVS_AUDIO_RECEIVER_BASE
#define REGISTER_AVS_AUDIO_TRANSMITTER_BASE                                    \
  DICE_REGISTER_AVS_AUDIO_TRANSMITTER_BASE

namespace DICE {} // namespace DICE
} // namespace FWA
