# Summary of FFADO Mailing List Technical Discussions

## 1. ALSA DICE Driver Enhancements
- Support for up to 192.0 kHz sampling rate
- PCM capture and playback capabilities
- MIDI playback/capture support
- Full-duplex streams with synchronization (using snd-firewire-lib)
- Limitations: No synchronization between multiple devices on same bus, no internal DSP control

## 2. DICE Notification Mechanism
- Driver reserves 4-byte region for notifications on OHCI 1394 host controller
- Driver registers address with device
- Device sends 4-byte messages for status changes (e.g., clock changes)
- Notification mechanism used for model-specific operations
- Critical timing for notification registration after bus reset

## 3. Device Support and Compatibility
- Extensive list of supported vendors including TC Electronic, Focusrite, M-Audio, Presonus, etc.
- Device-specific quirks handling (e.g., disabling double PCM frames for some M-Audio models)
- Vendor/Model ID detection crucial for proper device support
- GUID inconsistency handling for specific models (e.g., Focusrite Saffire Pro 40)

## 4. Register Access Implementation
- FFADO uses dynamic base address discovery for register spaces
- Different approach from absolute offsets (0xFFFFF0000000 + offset)
- Global, Tx, and Rx register spaces located dynamically
- Relative offsets used within discovered spaces
- Important discrepancy between specification's absolute offsets and FFADO's dynamic approach

## 5. Driver Architecture Insights
- Separation between kernel driver and userspace functionality
- ALSA HwDep interface for synchronous operations
- Event-based notification system for asynchronous updates
- Careful handling of device ownership between kernel and userspace drivers

## 6. Channel Configuration
- Support for complex channel configurations (e.g., Venice F with 24 mono + 4 stereo pairs)
- Hardcoded connections in some devices (1:1 mapping)
- ADAT/FireWire routing affects channel accessibility
- Need for dynamic channel discovery and validation

This information provides crucial insights for implementing DICE support, particularly regarding register access strategies, notification handling, and device-specific considerations.

## Venice F32 FFADO Log Analysis

### Device Identification
- Vendor: Midas
- Model: Venice F32
- GUID: 0x10C73F04004029BC
- Vendor ID: 0x10c73f
- Model ID: 0x00000001

### DICE Parameter Space Layout
- Global: offset=0x0028, size=0360
- TX: offset=0x0190, size=0568 (2 streams, size=0280 each)
- RX: offset=0x03C8, size=1128 (2 streams, size=0280 each)
- UNUSED1: offset=0x0830, size=0016
- UNUSED2: offset=0x0000, size=0000

### Channel Configuration
- 24 mono output channels (OUTPUT CH1-CH24)
- 24 mono input channels (INPUT CH1-CH24)
- 4 stereo output pairs (OUTPUT ST CH1L/R through CH4L/R)
- 4 stereo input pairs (INPUT ST CH1L/R through CH4L/R)
- Total: 64 I/O channels (32 in, 32 out)

### Clock Information
- Version: 0x01000400 (1.0.4.0)
- Clock capabilities: 0x13000006
- Internal clock source available (48000 Hz default)
- External clock sources not detected in log

### Notable Issues
- EAP mixer initialization failed: "Could not read from DICE EAP base address" (lines 199-202)
- Error accessing high memory addresses (0xFFFFE0200000)
- Streaming start failure with timeout waiting for synchronization
- ISO channel allocation issues for streams 2 and 3

### Register Access Strategy
- Uses ReadBlock for high addresses (>= 0xFFFFF0000000)
- Confirms FFADO's approach of using dynamic base address discovery
- ARM notification mechanism observed at 0x0000FFFFE0000000

[2025-05-06 14:37:12] - Venice F32 FFADO log analysis added
