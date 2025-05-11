# Summary for src/Isoch/core/AudioClockPLL.cpp

This C++ file implements the `FWA::Isoch::AudioClockPLL` class. This class is designed to function as a Phase-Locked Loop (PLL) for audio clock synchronization. Its primary purpose is to synchronize a local audio clock (e.g., the computer's clock used for audio processing or playback) with an external reference clock, typically derived from SYT (System Time) timestamps embedded in an incoming AMDT-P stream from a FireWire audio device.

**Key Functionalities:**

-   **Constructor (`AudioClockPLL::AudioClockPLL`):**
    -   Takes parameters such as:
        -   `nominalSampleRate`: The target sample rate (e.g., 44100.0, 48000.0 Hz).
        -   PLL loop filter coefficients (e.g., proportional gain `Kp`, integral gain `Ki`). These determine the PLL's response characteristics (how quickly it locks, how much jitter it filters).
        -   An `spdlog::logger`.
    -   Initializes internal state variables:
        -   `_nominalSamplePeriod`: Calculated from `nominalSampleRate`.
        -   Phase error accumulator, integral term for the PI controller.
        -   Previous timestamps (local and device).
        -   Lock status indicators.

-   **Timestamp Update (`update` or `processTimestamps`):**
    -   `void update(uint64_t deviceSytTimestamp, uint64_t localMachTimestamp)`:
        -   This method is called periodically with new timestamp pairs:
            -   `deviceSytTimestamp`: An SYT timestamp received from the FireWire device (e.g., from an AMDT-P packet header). This represents the device's clock.
            -   `localMachTimestamp`: The local system time (e.g., from `mach_absolute_time()`) corresponding to when the `deviceSytTimestamp` was valid or received. This represents the local clock.
        -   **Phase Error Calculation:**
            -   Calculates the difference in elapsed time between successive calls for both the device clock and the local clock.
            -   Compares these elapsed times to determine the phase error (i.e., how much the local clock is leading or lagging the device clock). This often involves converting time differences to sample counts or fractions of a sample period.
            -   Handles timestamp rollovers.
        -   **Loop Filter (PI Controller):**
            -   The calculated phase error is fed into a Proportional-Integral (PI) loop filter:
                -   Proportional term: `Kp * phaseError`.
                -   Integral term: `Ki * accumulatedPhaseError`.
            -   The sum of these terms produces a correction signal.
        -   Updates the internal state of the PI filter (e.g., the integral accumulator).

-   **Clock Correction Output (`getCorrectionFactor` or `getAdjustedSamplePeriod`):**
    -   `double getCorrectionFactor() const`:
        -   Returns a correction factor (e.g., a value slightly greater or less than 1.0) based on the output of the loop filter.
        -   This factor can be used by the local audio engine to slightly adjust its playback or recording rate to stay synchronized with the device clock. For example, if the local clock is running fast, the correction factor would be < 1.0 to slow it down.
    -   Alternatively, it might provide an adjusted sample period.

-   **Lock Status (`isLocked`):**
    -   `bool isLocked() const`:
        -   Provides a status indicating whether the PLL has successfully locked onto the device clock (i.e., the phase error is consistently within a small tolerance).
        -   This might involve checking the magnitude of the phase error or the stability of the correction factor over a period.

-   **Reset (`reset`):**
    -   `void reset()`: Resets the internal state of the PLL (phase error, integral term, previous timestamps) to its initial conditions. This is useful when starting a new stream or if a major clock discontinuity occurs.

**Overall Role:**
The `AudioClockPLL` is crucial for maintaining high-quality, synchronized audio playback and recording with external FireWire devices, especially when the device is the master clock source. By continuously comparing device timestamps (from SYT) with local timestamps and applying a control loop, it generates a correction signal that allows the computer's audio system to precisely track the FireWire device's sample clock. This prevents audio artifacts like clicks, pops, and drift caused by clock mismatches, ensuring that audio samples are processed and rendered at the correct rate relative to the external device. It's a fundamental component for robust isochronous audio streaming.
