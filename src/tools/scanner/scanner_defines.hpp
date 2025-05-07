#ifndef FWA_SCANNER_DEFINES_HPP
#define FWA_SCANNER_DEFINES_HPP

#include <cstdint>

namespace FWA {
namespace Scanner {

// Initial base address used by FFADO for finding dynamic pointers
// Value from dice_defines.h:DICE_REGISTER_BASE
constexpr uint64_t FFADO_POINTER_DISCOVERY_BASE = 0x0000FFFFE0000000ULL;

// Offsets relative to FFADO_POINTER_DISCOVERY_BASE to find dynamic base
// pointers Each pointer is 64-bit, split into high and low 32-bit values
constexpr uint32_t FFADO_OFFSET_DICE_GLOBAL_PTR_HI =
    0x0000; // High 32 bits of Global space pointer
constexpr uint32_t FFADO_OFFSET_DICE_GLOBAL_PTR_LO =
    0x0004; // Low 32 bits of Global space pointer
constexpr uint32_t FFADO_OFFSET_DICE_TX_PTR_HI =
    0x0008; // High 32 bits of TX[0] space pointer
constexpr uint32_t FFADO_OFFSET_DICE_TX_PTR_LO =
    0x000C; // Low 32 bits of TX[0] space pointer
constexpr uint32_t FFADO_OFFSET_DICE_RX_PTR_HI =
    0x0018; // High 32 bits of RX[0] space pointer
constexpr uint32_t FFADO_OFFSET_DICE_RX_PTR_LO =
    0x001C; // Low 32 bits of RX[0] space pointer

// Key register offsets relative to the *dynamically discovered* base addresses
// Values from dice_defines.h
constexpr uint32_t DICE_REL_OFFSET_GLOBAL_OWNER =
    0x0000; // DICE_REGISTER_GLOBAL_OWNER
constexpr uint32_t DICE_REL_OFFSET_GLOBAL_NOTIFICATION =
    0x0008; // DICE_REGISTER_GLOBAL_NOTIFICATION
constexpr uint32_t DICE_REL_OFFSET_GLOBAL_CLOCK_SELECT =
    0x004C; // DICE_REGISTER_GLOBAL_CLOCK_SELECT
constexpr uint32_t DICE_REL_OFFSET_GLOBAL_SAMPLE_RATE =
    0x005C; // DICE_REGISTER_GLOBAL_SAMPLE_RATE
constexpr uint32_t DICE_REL_OFFSET_TX_ISOC_BASE =
    0x0008; // DICE_REGISTER_TX_ISOC_BASE
constexpr uint32_t DICE_REL_OFFSET_RX_ISOC_BASE =
    0x0008; // DICE_REGISTER_RX_ISOC_BASE (v1.0.7.0)

// Additional relative offsets for TX/RX stream parameters
constexpr uint32_t DICE_REL_OFFSET_TX_NB_TX = 0x0000; // DICE_REGISTER_TX_NB_TX
constexpr uint32_t DICE_REL_OFFSET_TX_SZ_TX = 0x0004; // DICE_REGISTER_TX_SZ_TX
constexpr uint32_t DICE_REL_OFFSET_RX_NB_RX = 0x0000; // DICE_REGISTER_RX_NB_RX
constexpr uint32_t DICE_REL_OFFSET_RX_SZ_RX = 0x0004; // DICE_REGISTER_RX_SZ_RX

} // namespace Scanner
} // namespace FWA

#endif // FWA_SCANNER_DEFINES_HPP
