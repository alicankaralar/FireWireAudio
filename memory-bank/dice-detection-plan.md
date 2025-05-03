# Plan: Fixing DICE Device Detection (2025-05-03)

## Goal

Modify the FireWire audio device discovery and initialization process to correctly detect and handle DICE devices, particularly those that present as generic `IOFireWireDevice` services rather than `IOFireWireAVCUnit` services.

## Problem Analysis

1.  **Discovery (`IOKitFireWireDeviceDiscovery.cpp`):** Uses `IOServiceMatching("IOFireWireDevice")` and correctly filters using `IsDiceDevice` helper function. This part seems functional in *identifying* potential DICE devices.
2.  **Initialization (`AudioDevice.cpp::init`)**: This method currently assumes the `io_service_t` passed to the `AudioDevice` constructor (`avcUnit_`) is an `IOFireWireAVCUnit`. It then traverses *up* the I/O Registry tree using `IORegistryEntryGetParentEntry` to find the parent `fwUnit_` and grandparent `fwDevice_`.
3.  **Failure Point:** If a DICE device is represented only by an `IOFireWireDevice` service without the expected AVC Unit parentage in the registry, the initial `IORegistryEntryGetParentEntry(avcUnit_, ...)` call within `AudioDevice::init()` fails. This prevents the necessary `fwDevice_` service from being obtained, which is required later in `init()` to create the essential `IOFireWireLibDeviceRef` (`deviceInterface`).

## Approved Solution

Modify both the discovery logic and the `AudioDevice` class to work directly with the `IOFireWireDevice` service.

1.  **Modify `src/FWA/IOKitFireWireDeviceDiscovery.cpp` (`deviceAdded` function):**
    *   After identifying a DICE device using `IsDiceDevice(deviceInterface, guid)`, confirm that `device_service` (the `io_object_t` from the iterator) represents the correct `IOFireWireDevice` service needed. (It's likely `device_service` itself is the correct object).
    *   Extract GUID, Vendor Name, Device Name directly from the `device_service` properties (similar to the logic currently within `createAudioDevice`).
    *   **Crucially:** Instead of calling `createAudioDevice(device_service)`, call the *modified* `AudioDevice` constructor directly, passing the confirmed `IOFireWireDevice` service (`device_service`) as the primary service object.
    *   The `createAudioDevice` helper function might become redundant or need significant refactoring if its primary purpose was the registry traversal. Consider moving the property extraction logic directly into `deviceAdded` before constructing the `AudioDevice`.

2.  **Modify `AudioDevice.h`:**
    *   Change the constructor signature to accept the FireWire Device service directly:
        ```cpp
        // From:
        // AudioDevice(..., io_service_t avcUnit, ...);
        // To:
        AudioDevice(..., io_service_t fwDevice, ...);
        ```
    *   Update the corresponding member variable holding this service (e.g., rename `avcUnit_` to `fwDevice_` or use `fwDevice_` which already exists but is currently populated via parent lookup). Ensure consistency.

3.  **Modify `src/FWA/AudioDevice.cpp` (`init` method):**
    *   Remove the initial block of code that uses `IORegistryEntryGetParentEntry` to find `fwUnit_` and `fwDevice_` by traversing *up* from the service passed to the constructor (approx. lines 85-100).
    *   Use the `fwDevice_` member (now passed directly via the constructor) to:
        *   Get the `busController_` using `IORegistryEntryGetParentEntry(fwDevice_, kIOServicePlane, &busController_);` (approx. line 103).
        *   Call `createFWDeviceInterface()` (approx. line 127), ensuring it correctly uses the `fwDevice_` member.
    *   **Review AVC Interface:** Carefully examine how `avcInterface_` (`IOFireWireAVCLibUnitInterface**`) is created and used. Since the device might not be a standard AVC unit, its creation might fail or be unnecessary. Consider:
        *   Conditionally creating/using the `avcInterface_` based on the class of `fwDevice_`.
        *   Finding alternative methods for sending necessary commands if the standard AVC interface isn't available but equivalent functionality exists via direct FireWire transactions.

## Next Steps (Post-Planning)

1.  Switch to Code mode.
2.  Implement the changes in `IOKitFireWireDeviceDiscovery.cpp`, `AudioDevice.h`, and `AudioDevice.cpp`.
3.  Compile and test DICE device detection.
4.  Debug any issues arising from the changes, particularly around command interface initialization and usage.