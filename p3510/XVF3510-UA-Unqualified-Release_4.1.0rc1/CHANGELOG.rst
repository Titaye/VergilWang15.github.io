sw_xvf3510 change log
=====================

4.1.0
-----

  * ADDED: Return firmware version and build identifier in VfCtrl help output
  * FIXED: VfCtrl dump of GET_FILTER_COEFF command
  * FIXED: Improve Vfctrl command corrector printouts and parsing
  * FIXED: Error printout in VfCtrl host app when control command is invalid
  * ADDED: License file in release packages
  * FIXED: Content of the main license file
  * FIXED: xvf3510_data_partition_generator.py supports paths with blank spaces
  * FIXED: In Android VfCtrl app skip command corrector to avoid sporadic
    failures due to latency
  * FIXED: In Android Vfctrl app trigger new connection when app is stopped
  * ADDED: SPI slave boot mode check before starting SPI master thread
  * CHANGED: Optimize SPI boot performance by reversing bits in
    send_image_from_rpi.py script more efficiently
  * ADDED: New CLI arguments to tune SPI boot speed in send_image_from_rpi.py
  * ADDED: USB class compliant volume controls

4.0.0
-----

  * FIXED: Potential overflow in IO map difference type outputs
  * ADDED: DFU utility option to just issue a reboot
  * FIXED: DFU utility byte-order portability
  * ADDED: Spi boot direct option to support -UA version
  * FIXED: Issue where maximum serial number length was 24 rather than 25
    characters
  * FIXED: When DFU interrupted during boot image download, wrong run status
    will be reported, empty serial number returned and normal factory functions
    unavailable
  * ADDED: Get hardware build command
  * CHANGED: Include bcdDevice in DFU suffix
  * ADDED: Instructions to build vfctrl_json host app used in the data partition
    generator
  * ADDED: SPI boot script and binary in release package for UA product
  * FIXED: Build DFU host app in release mode to reduce number of dynamically
    linked libraries
  * FIXED: Missing vfctrl_i2c in INT release package
  * ADDED: VfCtrl host app converts run status value into string
  * ADDED: VfCtrl host app checks if data partition is read correctly
  * FIXED: VfCtrl host app doesn't read device version when help info is printed
  * FIXED: VfCtrl host app exits correctly in case of connection error
  * FIXED: VfCtrl host app prints all the parameters when --dump-params is used
  * REMOVED: Invalid commands in Vfctrl I2C host app
  * FIXED: Ensure all strings are null-terminated by the host utilities
  * ADDED: Suggest similar commands in VfCtrl in case of typo
  * ADDED: Unpack all script in release package
  * ADDED: Support for AP driven configure and start in SPI slave boot mode.
  * ADDED: Get GPI pin
  * FIXED: Updated Keras version to ~2.3.1 to match lib_vad
  * REMOVED: Default settings from data partitions
  * FIXED: Use of incorrect flags in UBM timing test

  * Changes to dependencies:

    - audio_test_tools: 4.1.1 -> 4.2.0

      + ADDED: new function for parsing wav data: audio_wav_utils.iter_frames

    - lib_aec: 8.0.3 -> 8.0.4

      + FIXED: Disable debug printouts
      + CHANGED: Switch from pipenv to virtualenv
      + CHANGED: Update Jenkins shared library to 0.14.1
      + ADDED: Locker zero_x_frame calc moved from lib_audio_pipelines
      + ADDED: Locker correlation peak interpolation in Python

    - lib_agc: 7.0.0 -> 7.0.1

      + CHANGED: Use virtualenv instead of pipenv.

    - lib_audio_pipelines: 1.2.0 -> 1.3.1

      + ADDED: State report output to test_wav_ap
      + FIXED: Alt-arch switching failure when using inital delay estimation
      + CHANGED: Set the max size of the strings for USB control commands to 26,
        including the null-terminator char
      + ADDED: Control command to get Hardware Build
      + CHANGED: test_wav_ap now builds variants (similar to sw_xvf3510 build)
      + ADDED: Control command to get GPI PIN
      + ADDED: AEC peak to average ratio and peak location to state_report
      + CHANGED: Disable initial delay estimation in the long pipeline tests
        when the alternate architecture is enabled

    - lib_dfu: 1.0.3 -> 1.0.5

      + FIXED: Suffix generator and verifier byte-order portability
      + FIXED: Build suffix generator in release mode to reduce number of
        dynamically linked libraries
      + CHANGED: Include bcdDevice in suffix

    - lib_dsp: 6.0.0 -> 6.0.1

      + CHANGED: Updated generate_window.py to work with Python 3
      + CHANGED: Updated to use Jenkins shared library 0.14.1

    - lib_flash_data_partition: 2.0.3 -> 2.2.0

      + FIXED: data partition generator parses arguments up to 0xFFFFFFFF on
        Windows
      + ADDED: Boot image searching functions with full image checksum
      + CHANGED: Rename image searching functions to make it clear what level of
        of checksum they do
      + FIXED: Build data partition generator in release mode to reduce number
        of dynamically linked libraries
      + FIXED: Increase serial number maximum from 24 to 25 characters and allow
        strings without null termination in programming function

    - lib_ndp: 1.0.1 -> 2.0.0

      + CHANGED: Updated to SDK version v40
      + FIXED: Device not responding approximately 1 out of 10 tries

    - lib_vad: 1.0.1 -> 1.0.3

      + CHANGED: Restricted the version of keras to be compatible with 2.3
      + CHANGED: Restricted the version of tensorflow to be compatible with 2.1
      + CHANGED: Update Jenkins shared library to 0.14.1
      + CHANGED: Switch from pipenv to virtualenv

3.0.5
-----

  * ADDED: Two new channels for the ref input filter
  * ADDED: VfCtrl host app prints the app version in the help information
  * FIXED: During data partition generation compatibility version is retrieved
    from VfCtrl host app
  * FIXED: HID report mapping for a working Raspberry Pi demo
  * ADDED: Optional argument to configure compatibility version in
    xvf3510_data_partition_generator.py
  * ADDED: compatibility version is appended to file name of data partition
    image
  * CHANGED: Rename all _config.json files as .json
  * ADDED: Test harness support
  * CHANGED: change packing resolution for PACKED_ALL based on
    device_to_usb_bit_res
  * ADDED: VfCtrl host app dedicated to convert control commands for data
    partitions
  * FIXED: Windows compile warnings
  * FIXED: Remove superfluous config.xscope
  * FIXED: Runtime exception when DFU initiated without valid data partition
  * CHANGED: Update unpacker_packed_all.py to be robust to audio dropouts
  * REMOVED: Out-of-date documentation for user guides and datasheet
  * CHANGED: Length of View name string for host apps increased from 30 to 50
  * CHANGED: Minimum lib_audio_pipelines version increased to 1.1.0
  * ADDED: SPI boot script and binary in release package for INT product
  * ADDED: State report mechanism for getting time-synced Locker state
  * CHANGED: Jenkinsfile pinned to Jenkins shared library 0.13.0
  * ADDED: Control commands for Locker delay set-point
  * ADDED: Command to override flash specification for DFU
  * CHANGED: Remove checking of flash manufacturer ID
  * FIXED: Undefined I2S BCLK polarity for i2s_slave
  * ADDED: Programmable serial number

  * Changes to dependencies:

    - audio_test_tools: 4.1.0 -> 4.1.1

      + CHANGED: minimum version of lib_dsp required moved to 6.0.0
      + CHANGED: use v0.12.1 of Jenkins shared library

    - lib_aec: 8.0.1 -> 8.0.3

      + CHANGED: Reduce memory reserved for locker detection from 4K to 3.5K
      + FIXED: Division by zero error in Locker
      + ADDED: Sub-frame if-zero check to Locker XC
      + CHANGED: Reserve memory and cycles for locker detection function

    - lib_agc: 6.0.1 -> 7.0.0

      + CHANGED: Loss control requires an AEC correlation value.
      + CHANGED: Removed unnecessary internal state in agc_ch_state_t.
      + CHANGED: Switch from pipenv to virtualenv
      + CHANGED: Update Jenkins shared library to 0.14.1
      + CHANGED: Further updates to loss control parameters.

    - lib_audio_pipelines: 1.1.1 -> 1.2.0

      + ADDED: Regression to test the ref filter.
      + ADDED: New filter for ref input in Stage A.
      + ADDED: Mic, ref and output filter in the python simulation.
      + CHANGED: Calculate AEC correlation value and forward in metadata.
      + ADDED: Get and set metadata functions for AEC correlation value.
      + CHANGED: Move python dependencies from Pipfile into
        requirements[-dev].txt
      + CHANGED: Locker now robust to reference dropouts while AEC is converging
      + FIXED: Fixed IC behaviour for alt-arch mode.
      + FIXED: test_wav_ap can now be controlled in simulation while alt-arch is
        enabled
      + ADDED: Sub-frame if-zero check to locker.py
      + CHANGED: Lower threshold for locker search success, increase locker
        search window
      + FIXED: Remove 2 thread mic array code from stage a
      + ADDED: Option to use delay setpoint in Locker (alternative to ADEC
        trigger)
      + FIXED: Workaround for locker correlation bug to avoid setting incorrect
        delay
      + CHANGED: Length of View name string increased from 30 to 50
      + CHANGED: Minimum lib_aec version increased to 8.0.0
      + ADDED: State report mechanism for getting time-synced Locker state
      + CHANGED: Enable the Alternative Architecture by default
      + ADDED: Control commands to set USB Serial Number string

    - lib_dfu: 1.0.1 -> 1.0.3

      + ADDED: Switch to newly added single-spec flash connect function
      + FIXED: Windows compile warnings

    - lib_flash_data_partition: 2.0.1 -> 2.0.3

      + ADDED: Flash connect function where manufacturer ID checking is optional
      + ADDED: Serial number section in data partition header with programming
      + FIXED: Windows compile warnings

    - lib_voice_toolbox: 7.1.1 -> 8.0.0

      + CHANGED: vtb_md_t data size is configurable at the application level.

    - lib_xua: 1.0.0 -> 1.1.0

      + ADDED:     Ability to read or modify serial number string
      + FIXED:     Wrong size of vendor and product strings

3.0.4
-----

  * CHANGED: Create separate release packages for UA and INT products
  * FIXED: Bug in data partition generation for Windows platforms
  * ADDED: Test to time User Buffer Management

  * Changes to dependencies:

    - lib_audio_pipelines: 1.1.0 -> 1.1.1

      + FIXED: Increase Stage C fixed ticks to 14 ms

3.0.3
-----

  * ADDED: DFU host utility source code in release package
  * FIXED: printout of GET_BLD_MODIFIED command
  * FIXED: XMOS_ROOT path in xvf3510_data_partition_generator.py
  * FIXED: DFU oversize image handling (#691)
  * REMOVED: USB_DEVICE_ATTACHED and USB_DEVICE_DETACHED actions in VfCtrl
    Android App
  * ADDED:  Support for configurable Product and Vendors ID in VfCtrl Android
    App

  * Changes to dependencies:

    - lib_aec: 8.0.0 -> 8.0.1

      + CHANGED: Jenkinsfile pinned to Jenkins shared library 0.10.0

    - lib_agc: 6.0.0 -> 6.0.1

      + CHANGED: Updated loss control parameters for communications channel.

    - lib_audio_pipelines: 1.0.1 -> 1.1.0

      + FIXED: Size and type of ap_control_cmd_get_bld_modified
      + REMOVED: XVF3100 example applications
      + ADDED: Added high-pass filter to Comms channel in Stage C.
      + CHANGED: Configured Noise Suppression to apply to both channels.

    - lib_dfu: 1.0.0 -> 1.0.1

      + ADDED: Handling of oversize images (protect factory programming in
        flash)

    - lib_flash_data_partition: 2.0.0 -> 2.0.1

      + FIXED: Handle oversize images

    - lib_mic_array: 4.2.0 -> 4.2.1

      + CHANGED: Jenkinsfile pinned to Jenkins shared library 0.10.0
      + CHANGED: Updated the minimum version of libraries this library depends
        upon.

    - lib_ndp: 1.0.0 -> 1.0.1

      + CHANGED: Update to use improved safety QSPI flash reading

    - lib_noise_suppression: 2.0.0 -> 2.1.0

      + REMOVED: Statistical Noise Suppression (SNS) support has been removed.

    - lib_vad: 1.0.0 -> 1.0.1

      + CHANGED: Jenkinsfile pinned to Jenkins shared library 0.10.0
      + CHANGED: Updated the minimum version of libraries this library depends
        upon.

3.0.2
-----

  * CHANGED: Support for configurable Product and Vendors ID in VfCtrl Host App
  * FIXED: Handling of optional arguments in VfCtrl host app

  * Changes to dependencies:

    - lib_aec: 7.0.0 -> 8.0.0

      + FIXED: Locker - Correlator peak position calc only returning positive
        values
      + CHANGED: Locker - Added peak-average ratio calculation
      + CHANGED: Locker - Correlation peak position now interpolated (Q16
        format)

    - lib_audio_pipelines: 0.17.0 -> 1.0.1

      + CHANGED: Locker - Use a "fuzzy mode" of interpolated peak position
      + CHANGED: Locker - Use peak to average ratio threshold to avoid setting
        the wrong delay
      + CHANGED: Version for first major release.

    - lib_interference_canceller: 5.1.0 -> 5.1.1

      + CHANGED: Minimum version for libraries upon which this one depends

3.0.1
-----

  * CHANGED: Version number for beta testing (3.0.0 was used for alpha)

3.0.0
-----

  * CHANGED: Programmable audio master clock input and output
  * ADDED: Persistent control parameters programmed in flash memory
  * ADDED: Configure USB bcdDevice at startup through a control command
  * CHANGED: DFU capable of upgrading boot and data images
  * FIXED: plot_aec.py now works with alt-arch mode
  * ADDED: Control commands for enabled loss control
  * ADDED: Control command to check run status
  * ADDED: Control commands to set USB Vendor and Product strings
  * CHANGED: renamed script json_data_partition_generator.py to
    xvf3510_data_partition_images_generator.py and improved CLI arguments
  * ADDED: Vfctrl Android host app
  * ADDED: Support for handling small delay changes (project Locker)
  * ADDED: Support for packing mic, ref and processed outputs over 2, 48kHz
    outputs
  * ADDED: Handle active high or active low setting on GPO and GPI pins

  * Changes to dependencies:

    - audio_test_tools: 4.0.0 -> 4.1.0

      + FIXED: audio_generation.py get_noise for non-integer durations

    - lib_aec: 6.1.0 -> 7.0.0

      + ADDED: Alt-arch mode supporting 15 phase, single channel AEC
      + FIXED: Support for y channels = 1 & x channels = 2
      + DEPRECATED: make_aec method on aec python: use `aec(**aec_conf)` instead
      + CHANGED: AEC outputs 1 channel pair instead of 2.
      + CHANGED: Updated test_wav_aec.py for consistency with other
        test_wav_*.py files
      + ADDED: Support for handling small delay changes (project Locker)

    - lib_agc: 5.1.0 -> 6.0.0

      + ADDED: support for loss control.
      + UPDATED: API updated to accept reference audio frame power.
      + CHANGED: Update dependency on lib_dsp to v6.0.0

    - lib_ai: 0.0.1 -> 1.0.0

      + CHANGED: Version for first major release

    - lib_device_control: 4.0.0 -> 4.0.1

      + CHANGED: Increase USB host timeout to 500ms

    - lib_dfu: Added dependency 1.0.0

      + First release

    - lib_dsp: 5.0.0 -> 6.0.0

      + ADDED: vector add, sub and mul functions for int8, int16 and int32 in
        both fixed and block floating point.
      + CHANGED: Functions re-implemented in C, requires the use of unsafe
        pointers if called from XC

    - lib_flash_data_partition: 0.1.0 -> 2.0.0

      + CHANGED: Refactor using a clone of the toolchain built-in quadflash
        library
      + CHANGED: Remove need of toolchain in flash_specification_serialiser.py
      + ADDED: Support for reading flash specification from hardware build
        section

    - lib_ndp: 0.0.1 -> 1.0.0

      + Initial release
      + ADDED: Support for flash data partition

    - lib_vad: 0.5.0 -> 1.0.0

      + CHANGED: Version for first major release.

    - lib_voice_toolbox: 7.1.0 -> 7.1.1

      + ADDED: floating point element in vtb_md_t
      + ADDED: vtb_is_frame_active function
      + ADDED: get_dict_subtree_from_key python utility function.
      + ADDED: VTBFrame, VTBInfo, and DelayLine classes in Python
      + ADDED: voice_toolbox.h can now be included from C

    - lib_xua: 0.3.0 -> 1.0.0

      + ADDED:     UAC1 HID support with simulated Voice Command detection
        reported every 10 seconds
      + ADDED:     Support for USB HID Set Idle request
      + ADDED:     Pre-processor symbols to enable single-threaded, dual-PDM
        microphone operation
      + FIXED:     Descriptors for XUA_ADAPTIVE incorrectly defined for IN
        endpoint
      + ADDED:     Guards to user_hid.h and xua_hid.h
      + ADDED:     UAC1 HID support for AC Stop (End Call), Volume Increment and
        Volume Decrement
      + CHANGE:    UAC1 HID to report function keys f21 through f24 as specified
        by customer
      + CHANGE:    HID interface for user to set and clear events from global
        variable to function
      + CHANGE     HID report descriptor to use generic events instead of GPI
        events, to report Key-phrase detection as AC Search, and to report
        end-call detection as AC Stop
      + ADDED:     Ability to read or modify vendor and product IDs and strings
      + ADDED:     Ability to read or modify bcdDevice
      + ADDED:     Override USB descriptor with sampling frequency and
        bit-resolution set at boot time.
      + ADDED:     Global pointer to allow external access to masterClockFreq

    - lib_xud: 0.2.0 -> 1.0.0

      + CHANGE:     Version for first major release.

2.2.1
-----

  * ADDED: Enable mute button interrupt in data partition
  * ADDED: Red LED is flashing after boot is completed
  * ADDED: Support write and read multi-data by I2C
  * ADDED: Data partition generator source code in release package
  * FIXED: Filter index mapping now matches output_filter_map_t
  * FIXED: Host conversion for uint32
  * ADDED: Add the default spec for GD25Q16CTIGR
  * ADDED: Control commands for setting USB rate and bit resolution
  * ADDED: Variable USB Sampling Frequency and Bit Resolution. USB descriptor is
    limited to one freq/resolution configuration only.
  * FIXED: Print of data partition commands for AGC module
  * ADDED: Assert in case data partition commands are invalid
  * FIXED: Build Win32 host apps in release mode
  * FIXED: Do not process data partition control messages if queue is full
  * ADDED: Script to generate JSON data partition files
  * ADDED: Data partition generator host app binaries in release package

2.2.0
-----

  * ADDED: Commands to set I2S rate and to start I2S in data partition.
  * ADDED: Configure I2S rate at startup through a control command and then
    start the I2S thread.
  * ADDED: Configure USB Vendor and Prodcut ID at startup through a control
    command and then start the USB threads.
  * ADDED: Programmable biquad filters at mic input, usb output and i2s output
  * ADDED: Ability to send packed 16kHz Mic, Reference and Pipeline output
    signals on a 48kHz output
  * REMOVED: usb_no_processing_adaptive build
  * ADDED: Unpacker script to the release
  * REMOVED: timing assert in audiohub and replaced it with a debug print

  * Changes to dependencies:

    - lib_audio_pipelines: 0.16.1 -> 0.17.0

      + ADDED: Support for programmable biquad filters at front and back of
        pipeline
      + ADDED: Control commands to set I2S rate and start I2S
      + ADDED: Control commands to set USB Product/Vendor IDs and strings
      + ADDED: Control command to start USB
      + ADDED: Control commands to set USB rate and bit resolution
      + ADDED: Control command to set USB bcdDevice version
      + ADDED: Audio master clock control commands
      + CHANGED: Adjust pass criteria for 7 tests
      + ADDED: Control command to check boot status
      + CHANGED: Custom get_next_frame saves 3.8kB on Tile[0]
      + FIXED: Case where ADEC gets stuck in DE mode after white noise
      + ADDED: Addtional ADEC watchdog case where pk:ave is too low
      + ADDED: Support for Loss Control
      + CHANGED: Support for reference power field in metadata
      + CHANGED: No reference channels passthrough from stage a and b to stage c
      + ADDED: Support for handling small delay changes (project Locker)
      + ADDED: Control commands to set GPO and GPI pin active level

    - lib_flash_data_partition: 0.0.1 -> 0.1.0

    - lib_mic_array: 4.1.0 -> 4.2.0

      + ADDED support for global define to set single/dual output buffer for
        mic_dual

2.1.0
-----

  * CHANGED: Adaptive USB PID set to target fill level on audio stream start
  * CHANGED: Enforce synchronisation of adaptive USB IN/OUT FIFOs
  * CHANGED: USB rate control performed at SOF rather than at USB data OUT
  * CHANGED: AudioHub now calls XUA_Transfer_samples at a regular rate
  * ADDED: Support for enabling beamforming or passthrough on pipeline output
    channel 1
  * ADDED:   Alt-arch dynamic switching of AEC and IC modes
  * CHANGED: Use new HID interface to set and clear events
  * CHANGED: HID report descriptor to use generic events instead of GPI events
  * CHANGED: Enable sharing of a common clock port for SPI and QSPI transactions
  * ADDED: Support HID events on any GPI pin that is configured to have
    interrupt enabled
  * CHANGED: KWD data loaded using lib_flash_data_partition
  * ADDED: Support for reading control messages from data partition
  * ADDED: Printing of control message for data partition JSON file in host app
  * ADDED: Infrastructure for arbitrary hardware tests
  * CHANGED: vfctrl version check only prints when version is incorrect
  * CHANGED: Use i2s_frame_slave and AudioHub instead of XUA_AudioHub for i2s
    slave build
  * REMOVED: i2s_slave_usb_monitor, usb, usb_out_i2s_in and usb_no_processing
    builds
  * ADDED: Script to generate release archives
  * CHANGED: AP control API updates
  * ADDED: Automate host app builds for RaspberryPi

  * Changes to dependencies:

    - lib_aec: 6.0.0 -> 6.1.0

    - lib_agc: 5.0.0 -> 5.1.0

      + CHANGED: Update .json config for test_wav_agc due to lib_vtb v7.1.0

    - lib_audio_pipelines: 0.15.0 -> 0.16.1

      + CHANGED: Delay estimator estimation stability and speed improvements
      + CHANGED: Alt-Arch AEC convergence improvement
      + ADDED: In the python implementation, cascade Verbose option to the
        Interference Canceller functions
      + ADDED: Optional alt-arch mode where stage A dynamically switches between
        a 15 phase, single mic AEC and IC only
      + ADDED: Support for reading AP control commands from data partition
      + ADDED: Support to test stage B only with a .wav file
      + CHANGED: Write AP control commands with same content are not discarded
      + FIXED: Preserve order of arrival of AP control messages

    - lib_i2s: 4.0.0 -> 4.1.0

      + ADDED: Frame based I2S master that needs the bit clock to be set up
        externally.
      + REMOVED: I2S_BCLOCK_FROM_XCORE and I2S_XCORE_BLOCK_DIV optional #ifdefs

    - lib_interference_canceller: 5.0.0 -> 5.1.0

      + ADDED: Option to either beamform or do passthrough on the second channel
      + ADDED: In python IC, print interesting variables on overflow during
        noise minimisation and invalid operand during filter adaptation
      + FIXED: In python IC, copy a slice of the Y data rather than creating a
        reference to it when calculating the minimum noise error to avoid
        corrupting the data buffer by subsequent noise minimisation processing
      + FIXED: Normalize the scaling factor for the minimum noise error
        calculation before applying it to avoid exponent overflow
      + FIXED: In python IC, re-calculate the freq-domain of the error after
        scaling the time-domain version of it
      + CHANGED: Instances of IC_DEBUG to IC_DEBUG_MODE
      + FIXED: Reference time-domain struct elements as ch_a and ch_b rather
        than re and im to match their definition
      + FIXED: Use the minimised time-domain error signal instead of the
        un-minimised one for the first 240 (IC_FRAME_ADVANCE) samples when
        forming the output frame
      + CHANGED: Delta parameter default value to 0.00007

    - lib_spi: 3.0.4 -> 3.1.0

      + Set up clock port in synchronous master every time (that way clock port
        can be shared with another task)
      + Add shutdown function to asynchronous master

    - lib_voice_toolbox: 7.0.1 -> 7.1.0

2.0.0
-----

  * ADDED:   GPIO server accesed via device_control supporting I2C master on
    tile[1] and GPI/GPO + SPI on tile [0]
  * ADDED:   HID report with value set by a GPIO pulse
  * ADDED:   Application-level configuration file: sw_xvf3510_conf.h
  * CHANGED: I2S for USB adaptive is now 16KHz (1.024MHz BCLK)
  * CHANGED: I2S out for USB adaptive has ASR on channel 0 and far end reference
    on channel 1
  * CHANGED: 16bit resolution in/out in USB adaptive configuration
  * CHANGED: Use single-thread mic array
  * CHANGED: Revert unnecessary increase in PLL modulation range
  * ADDED: support for custom spispec file in DFU for firmware and host app
  * ADDED: External keyword detector with initialisation and control from device
    code
  * ADDED: Support for control command to find out KWD boot status
  * CHANGED: Move DFU task to GPIO server thread
  * CHANGED: DFU host code to first confirm KWD boot has completed before
    starting the DFU process
  * ADDED: Template for new pull-requests on GitHub
  * REMOVED: Simulation of interrupts to trigger HID reporting
  * CHANGED: Audio control channel not passed to XUA Buffer Lite in order to
    save a channel end
  * REMOVED: Unused ports
  * CHANGED: Minimum acceptable version of lib_mic_array and lib_xua
  * CHANGED: Application configuration constant and header file name to match
    convention
  * ADDED: Template for new bug reports on GitHub
  * FIXED: Fixed-point values are printed with the correct number of decimal
    places in control host app
  * ADDED: Support for AC Stop (End Call), Volume Increment and Volume Decrement
  * ADDED: Support for changing input to output routing based on control
    commands
  * CHANGED: Build host control app as a C library using CMake
  * ADDED: Control command to get maximum number of cycles taken by one user
    buffer management function call

  * Changes to dependencies:

    - lib_ndp: Added dependency 0.0.1

      + Initial version

    - lib_spi: Added dependency 3.0.4

      + Make use of Wavedrom in documentation generation offline (fixes
        automated build due to a known Wavevedrom issue where it would generate
        zero size PNG)

1.1.1
-----

  * FIXED: crash of vfctrl_usb when the usb is not plugged in
  * CHANGED: #include "ap_conf.h" -> #include "ap_conf_full.h"

  * Changes to dependencies:

    - lib_audio_pipelines: 0.14.0 -> 0.15.0

      + REMOVED: audio_pipelines_common.h
      + ADDED: ap_conf_full.h and ap_conf_default.h
      + ADDED: compile-time check: AP_PROC_FRAME_LENGTH > AP_FRAME_ADVANCE
      + CHANGED: Use single-thread mic array
      + ADDED: Control command to check KWD device boot status
      + ADDED: Control commands to set IO mapping
      + ADDED: Control command to get max user buffer management function cycles
      + CHANGED: Pipeline tests reference results updated for new IC delta value

    - lib_mic_array: 4.0.0 -> 4.1.0

      + Added mic_dual, an optimised single core, 16kHz, two channel version
        (not compatible with async interface)

    - lib_xua: 0.2.1 -> 0.3.0

1.1.0
-----

  * ADDED: Hardware test for Adaptive USB - checks PID is at target > 85% of the
    time and that PID level is <= 148 over a 30s period
  * FIXED: macOS host utils zip missing libusb dynamic libs
  * ADDED: Build info is autogenerated and compiled into the app - see
    build_info_check.py --help
  * CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.
  * ADDED: Support for Xscope audio capture.
  * ADDED: Version check host control app
  * CHANGED: package_host.py no longer needs to be inside a sandbox
  * FIXED: Clock now runs at >500MHz in adaptive USB build

  * Changes to dependencies:

    - audio_test_tools: 3.0.0 -> 4.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_aec: 5.1.0 -> 6.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_agc: 4.1.0 -> 5.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_audio_pipelines: 0.13.1 -> 0.14.0

      + ADDED: Added commands for getting build info - BLD_HOST, BLD_MSG,
        BLD_XGIT_VIEW, BLD_XGIT_HASH, and BLD_MODIFIED
      + CHANGED: Reduced ADEC the far end threshold energy by 30dB to cope with
        very loud systems.
      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.
      + FIXED: Set async_interface_enabled to 0 in decimator_config to
        ap_stage_a.
      + FIXED: Spacing of x axis in regression test plots

    - lib_device_control: 3.2.4 -> 4.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_dsp: 4.2.0 -> 5.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_i2c: 5.0.1 -> 6.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_i2s: 3.0.0 -> 4.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.
      + CHANGE: At initialisation, configure LR clock of frame-based I2S slave
        for input.
      + CHANGE: Renamed example application directories to have standard "app"
        prefix.
      + ADDED: I2S_BCLOCK_FROM_XCORE and I2S_XCORE_BLOCK_DIV optional #ifdefs

    - lib_interference_canceller: 4.1.0 -> 5.0.0

      + CHANGED: Use Pipenv to setup python environment
      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.
      + CHANGED: Get wavfile processing related functions from audio_wav_utils
        in audio_test_tools

    - lib_logging: 2.1.1 -> 3.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_mic_array: 3.2.0 -> 4.0.0

      + ADDED: Support for arbitrary frame sizes
      + ADDED: #defines for mic muting
      + ADDED: Non-blocking interface to decimators for 2 mic setup
      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_noise_suppression: 1.7.0 -> 2.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_spdif: 3.1.0 -> 4.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_src: 1.1.2 -> 2.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_vad: 0.4.2 -> 0.5.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_voice_toolbox: 6.1.0 -> 7.0.1

      + HOTFIX: Fix bug in vtb_compute_Error_asm for phases > 32
      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.
      + ADDED: support for storing adaptive filter coefficients as 16-bit
        integers

    - lib_xassert: 3.0.1 -> 4.0.0

      + CHANGED: Build files updated to support new "xcommon" behaviour in xwaf.

    - lib_xua: 0.2.0 -> 0.2.1

      + HOTFIX: Fix descriptors for XUA_ADAPTIVE

    - lib_xud: 0.1.1 -> 0.2.0

      + CHANGE:     Build files updated to support new "xcommon" behaviour in
        xwaf.

1.0.0
-----

  * FIXED: Dependency change information now automatically included in this
    changelog. This release rolls up all dependency updates made since the 0.1.0
    release.
  * REMOVED: unused timer
  * REMOVED: extra zero in initialisation of a list
  * ADDED: new AGC parameters in control host apps
  * ADDED: initialization of the decoupler buffer
  * ADDED: control commands to get and set AGC max gain, increment and decrement
    gain stepsizes.
  * CHANGED: DFU Linux makefile targets now match DSP control makefile
  * ADDED: package_host.py script to package host code
  * FIXED: example_host Makefiles now detect relative location of
    lib_device_control
  * FIXED: Q0.32 format conversions in host app
  * CHANGED: XUA Lite now depends on latest xmos/lib_xua

  * Changes to dependencies:

    - audio_test_tools: 2.0.0 -> 3.0.0

      + CHANGED: Have a separate file contain wav file processing related python
        functions.
      + ADDED: Use pipenv to set up python environment

    - lib_aec: 5.0.0 -> 5.1.0

      + CHANGED: Use of pipenv to setup Python environment
      + ADDED: Support for JSON file for Python scripts
      + FIXED: optimized speed of aec_get_filter_coefficients() ( < 1000 cycles)
      + CHANGED: Get wavfile processing related functions from audio_wav_utils
        in audio_test_tools

    - lib_agc: 3.1.1 -> 4.1.0

      + CHANGED: Use pipenv to set up python environment.
      + CHANGED: Get wavfile processing related functions from audio_wav_utils
        in audio_test_tools
      + ADDED: support for JSON config file
      + UPDATED: Removed VAD threshold. API updated to accept VAD flag instead.

    - lib_audio_pipelines: 0.13.0 -> 0.13.1

      + FIXED: Reference input on comms channel when user issues the
        SET_REF_OUT_CH1 command.
      + CHANGED: Use old sensory model licensed for TrulyHandsfree for running
        pipeline tests
      + FIXED: ap_control taking >14000 ticks to process one command.
      + CHANGED: Declare ap_control() as combinable, no functional change
      + CHANGED: Python stage b bypass doesn't bypass the VAD, only the audio
        path
      + CHANGED: Add support for JSON config file for AGC
      + CHANGED: Get wavfile processing related functions from audio_wav_utils
        in audio_test_tools
      + ADDED: Calculation of vad flag value based on vad output and agc vad
        threshold

    - lib_i2c: 5.0.0 -> 5.0.1

      + CHANGE: Renamed example application directories to have standard "app"
        prefix.

    - lib_interference_canceller: 4.0.0 -> 4.1.0

    - lib_mic_array: 3.1.1 -> 3.2.0

      + Added xwaf build system support
      + Cleaned up some of the code in the FIR designer.
      + Removed fixed gain in examples
      + Update VU meter example
      + Fix port types in examples
      + Set and inherit XCC_FLAGS rather than XCC_XC_FLAGS when building library

    - lib_noise_suppression: 1.6.2 -> 1.7.0

      + ADDED: Use of pipenv to set up python environment
      + CHANGED: Get wavfile processing related functions from audio_wav_utils
        in audio_test_tools

    - lib_src: 1.1.1 -> 1.1.2

      + CHANGED: initialisation lists to avoid warnings when building

    - lib_vad: 0.3.2 -> 0.4.2

      + CHANGED: Refactored VAD class allows for different implementations
      + CHANGED: Feature extractor implemented as a class
      + ADDED: Option to replace current model with webrtcvad
      + ADDED: Option to output only the VAD features instead of the prediction
      + CHANGED: Get wavfile processing related functions from audio_wav_utils
        in audio_test_tools
      + ADDED: Regression test added for python implementation
      + CHANGED: test_wav_vad renamed
      + HOTFIX: Removed model coefficient override in python (saved keras model
        includes correct weights)
      + CHANGED: Custom model can be passed to python wav test

    - lib_voice_toolbox: 5.2.0 -> 6.1.0

      + CHANGED: Use of pipenv to setup Python environment
      + ADDED: floating point implementations of block floating point xc for
        rx/tx functions
      + ADDED: abs_channel function
      + ADDED: frequency domain implementation of hanning window.
      + ADDED: floating point implementations of block floating point xc
      + ADDED: transfer_status attribute to rx_state_t and tx_state_t
      + CHANGED: vtb_tx_notification and vtb_rx_notification now require a tx/rx
        state
      + CHANGED: added #pragma select handler to vtb_rx_notification (Issue
        #121)
      + CHANGED: vtb_rx is now a 'select' function
      + CHANGED: Improve usability of tx/rx when sending only frame_advance
      + ADDED: support for JSON config files

0.12.0
------

  * ADDED: Automatic delay estimator controller (ADEC)
  * ADDED: delay estimation always on
  * ADDED: delay update can be triggered manually for all builds
  * ADDED: USB build trigger delay update at startup
  * FIXED: Send DFU_CMD_INIT as a write command in dfu host code

  * Changes to dependencies:

    - lib_aec: 4.3.0 -> 5.0.0

      + ADDED: Support for ADEC
      + ADDED: Delay estimation during normal AEC mode

    - lib_audio_pipelines: 0.12.3 -> 0.13.0

      + ADDED: Automatic delay estimator controller (ADEC)
      + ADDED: Delay estimation during normal AEC mode
      + ADDED: Update delay setting via manual trigger only

0.11.1
------

  * CHANGED: Audio Pipelines to v0.12.2
  * CHANGED: AGC to v3.1.1
  * CHANGED: NS to v1.6.2
  * CHANGED: Rename plot_aec_coeffs.py -> plot_aec.py
  * CHANGED: Rename configure_delay.py -> estimate_delay.py
  * REMOVED: show_h_hat.py
  * CHANGED: Updated CLI for python host scripts: plot_aec.py, plot_ic.py,
    estimate_delay.py
  * ADDED: 10-xvf3510.local.rules
  * CHANGED: All documentation updated
  * FiXED: Fix SPI boot in usb adaptive by removing debug port output from
    xua_buffer_lite
  * CHANGED: Host scripts fail if executed with Python 2
  * ADDED: Argument to package_host.py to create a host_utils_vX.Y.Z.zip
  * CHANGED: Increased robustness of plot_aec.py and plot_ic.py by removing
    coefficients files before running
  * CHANGED: Fix SPI scripts to run with Python 3
  * ADDED: --half option to plot_ic.py to plot from -Pi/2 to Pi/2

  * Changes to dependencies:

    - audio_test_tools: 1.0.0 -> 2.0.0

      + REMOVED: moved dsp_complex_fp to lib_dsp
      + FIXED: Fixed scaling of floating point fft
      + ADDED: common_utils.py for loading/saving json/ini configs (with '//'
        comments)
      + FIXED: Enabled more tests on Jenkins
      + REMOVED: Function to load ini files
      + ADDED: Pipfile + setup.py for pipenv support
      + ADDED: Function to parse and convert ini files
      + ADDED: python_setup.bat
      + CHANGED: Read and write files as binary in process_wav.xc
      + UPDATED: Python code be python 3 compatible
      + CHANGED: Updated lib_voice_toolbox dependency to v5.0.0
      + CHANGED: att_process_wav output wav is now optional
      + ADDED: seed parameter to audio_generation.get_noise function
      + FIXED: audio_utils.convert_to_32_bit not checking if data is already
        int32
      + Added function to limit the number of bits represented by a complex
        array
      + Added 16 bit functions
      + ADDED: get_erle() function, moved from lib_audio_pipelines
      + ADDED: generation of delayed echo function
      + Updated version information

    - lib_agc: 3.0.0 -> 3.1.1

      + CHANGED: VAD threshold increased to 80%.
      + CHANGED: upper and lower threshold parameters in python from dB to
        non-dB.

    - lib_ai: Added dependency 0.0.1

      + Initial version

    - lib_audio_pipelines: 0.12.0 -> 0.12.3

      + Updated: Use lib_noise_suppression v1.6.2
      + CHANGED: Noise suppressor re-enabled
      + UPDATED: Use AGC v3.1.1.
      + UPDATED: Bypass noise suppressor in Stage C.
      + ADDED: AGC control commands to set increment, decrement stepsize

    - lib_dsp: 4.1.0 -> 4.2.0

      + Added dsp_ch_pair_t struct to represent channel pairs
      + Added floating point biquads
      + Changed implementation of biquads to be faster and smaller
      + Added use of pipenv to set up python environment

    - lib_noise_suppression: 1.6.0 -> 1.6.2

      + FIXED: Suppression performance issues
      + REMOVED: Unused variables to avoid warnings when building
      + FIXED: Control type of noise_mcra_noise_floor

0.11.0
------

  * ADDED: Support for tools release 15.0.0
  * REMOVED: unused .spispec file
  * FIXED: Clarify I2S port definitions
  * ADDED: warning for I2C control in guides
  * ADDED: usb_hardware_in_loop configuration
  * CHANGED: AGC to version v3.0.0
  * REMOVED: range constrainer

  * Changes to dependencies:

    - lib_agc: 2.3.0 -> 3.0.0

      + ADDED: Range constrainer like functionality within AGC.
      + ADDED: Parameters for upper and lower desired voice level thresholds.

    - lib_audio_pipelines: 0.11.0 -> 0.12.0

      + CHANGED: Default mic input shift and saturate values
      + CHANGED: Use agc instead of range constrainer
      + ADDED: Support for comms channel
      + ADDED: Support for running boinc python jobs

    - lib_interference_canceller: 3.1.1 -> 4.0.0

      + ADDED: Support for comms channel
      + ADDED: Added more parameters to __init__ method of python IC

    - lib_vad: 0.3.1 -> 0.3.2

      + ADDED: Changes in python to make it work when run as a pyinstaller
        compiled executable

0.10.1
------

  * REMOVED: debug logs from the decoupler

  * Changes to dependencies:

    - lib_aec: 4.1.0 -> 4.3.0

      + ADDED: support for config ini files for Python scripts
      + ADDED: Pipfile + setup.py for pipenv support
      + UPDATED: Python code be python 3 compatible
      + ADDED: Test for phase changes created in non cancelled audio output

    - lib_agc: 2.2.0 -> 2.3.0

      + ADDED: Pipfile + setup.py for pipenv support
      + ADDED: Python 3 support

    - lib_audio_pipelines: 0.10.0 -> 0.11.0

      + ADDED: Option to pass mic_array data frame to allow stage A input
        monitoring
      + ADDED: mic shift support in Python
      + ADDED: More state to python ap_pipeline class
      + ADDED: bypass attribute to all python stages
      + ADDED: Option to test single stages in Python
      + ADDED: Pipfile + setup.py for pipenv support

    - lib_interference_canceller: 3.0.0 -> 3.1.1

      + FIXED: Cython compile bug
      + ADDED: Pipfile + setup.py for pipenv support
      + ADDED: Python 3 support

    - lib_noise_suppression: 1.4.0 -> 1.6.0

      + ADDED: Pipfile + setup.py for pipenv support
      + ADDED: Support for python 3
      + FIXED: Parameters adjusted for consistency between xc and python

    - lib_vad: 0.1.0 -> 0.3.1

      + HOTFIX: Added missing python dependencies to Pipfile + setup.py
      + ADDED: Pipfile + setup.py for pipenv support
      + ADDED: Support for Python 3

    - lib_voice_toolbox: 5.0.0 -> 5.2.0

      + ADDED: Pipfile + setup.py for pipenv support
      + CHANGED: VTB tx/rx chunk size = 32
      + CHANGED: Lower overhead per-call of vtb_tx_chunked and vtb_rx_chunked
      + CHANGED: vtb_form_rx_state accepts null for prev_frame parameter

0.10.0
------

  * ADDED: Realtime Python pipeline
  * ADDED: Pipfile for use with pipenv
  * ADDED: usb_no_processing_adaptive build
  * ADDED: Debug build for i2s slave that allows monitoring of stage A input
    over USB
  * FIXED: Bug in XUA lite buffering
  * FIXED: Path to run Python scripts on Windows
  * CHANGED: Set control interface as positional parameter in Python scripts
  * CHANGED: Replicate samples instead of upsampling in USB no processing mode
  * FIXED: Remove use of old VTB API structs
  * FIXED: correct timing for the decoupler() at the end of the pipeline

0.9.0
-----

  * CHANGED: Updated all dependencies to use VTB v5.0.0
  * CHANGED: Make scripts compatible with Python3
  * FIXED: conversion and settings of AGC set/get commands
  * ADDED: mic shift and headroom constrainer control commands
  * ADDED: check if example_host files are correct
  * FIXED: rename set_delay.py to configure_delay.py
  * ADDED: plot_ic.py script to visualize IC filter
  * FIXED: use exec instead of reload in plot_aec_coeffs.py
  * FIXED: check module_build_info version
  * REMOVED: internal control flag pipeline_started_ubm

  * Changes to dependencies:

    - lib_aec: 4.0.0 -> 4.1.0

      + UPDATED: use API of lib_vtb v5.0.0
      + ADDED: test for input and output phases

    - lib_agc: 2.1.0 -> 2.2.0

      + CHANGED: Updated lib_voice_toolbox dependency to v5.0.0

    - lib_audio_pipelines: 0.9.0 -> 0.10.0

      + UPDATED: Use VTB API v5.0.0
      + UPDATED: Make scripts compatible with Python3
      + ADDED: Mic shift (default 4) at the beginning of stage A
      + ADDED: Range constrainer (default 3) at the end of stage C
      + UPDATED: Noise floor and noise reset period values for noise suppressor
      + UPDATED: Enable noise minimisation for interface canceller
      + UPDATED: Use Sensory server in RPi for pipeline tests

    - lib_interference_canceller: 1.5.2 -> 3.0.0

      + ADDED: Control commands to get IC coefficients
      + FIXED: use_noise_minimisation added to control map + tests
      + FIXED: Set/get x_energy_delta commands
      + CHANGED: Updated lib_voice_toolbox dependency to v5.0.0

    - lib_noise_suppression: 1.3.0 -> 1.4.0

      + CHANGED: Updated lib_voice_toolbox dependency to v5.0.0

    - lib_vad: 0.0.1 -> 0.1.0

      + CHANGED: Updated lib_voice_toolbox dependency to v5.0.0

    - lib_voice_toolbox: 4.0.0 -> 5.0.0

      + ADDED: vtb_ch_pair_t type for audio data in the time-domain.
      + ADDED: New API for VTB TX/RX
      + CHANGED: Major update to function names, header files and prototypes.
      + CHANGED: Documentation updates.
      + FIXED: Renamed enum types in control API
      + FIXED: Removed obsolete control funtions

0.8.1
-----

  * FIXED: handle paths with spaces in plot_aec_coeffs.py
  * FIXED: update example_host files with latest version from dsp_control

0.8.0
-----

  * FIXED: Skip assert in first iteration of UserBufferManagement
  * ADDED: Script to fetch and plot AEC filter coefficients
  * ADDED: Commands to enable the delay estimator and get the estimated delay
  * ADDED: Script set_delay.py to demo the delay estimator
  * FIXED: Remove stage information in control parameter names

  * Changes to dependencies:

    - lib_aec: 3.0.0 -> 4.0.0

      + ADDED: change config at runtime
      + ADDED: delay estimator
      + FIXED: improve convergence time

    - lib_audio_pipelines: 0.8.2 -> 0.9.0

      + CHANGED: enable noise suppression by default
      + ADDED: Delay estimator to stage A
      + ADDED: tests for diffused noise
      + CHANGED: Reduce convergence time in tests

    - lib_noise_suppression: 1.2.0 -> 1.3.0

      + CHANGED: Commented out echo suppression
      + ADDED: Option to apply noise suppression on only asr channel

0.7.0
-----

  * ADDED: script to update DSP control utility
  * ADDED: XUA Lite: usb_adaptive build config
  * CHANGED: DSP control host prints help when device is disconnected
  * CHANGED: DSP control host has a single Makefile for Linux
  * ADDED: DSP control host dumps the values of all the GET_ commands
  * FIXED: renamed TYPE_FLOAT_ as TYPE_FIXED_ in dsp_control host
  * FIXED: removed delay at startup
  * FIXED: types and value in dsp control app
  * FIXED: get aec coefficients control command on i2c
  * ADDED: new AGC commands in dsp control app
  * CHANGED: Reduce FLASH_MAX_UPGRADE_SIZE to 256KB

  * Changes to dependencies:

    - lib_aec: 1.0.3 -> 3.0.0

      + UPDATED: energy calculation to improve stability
      + CHANGED: x energy delta control parameter
      + REMOVED: aec_cmd_reset_coefficient_index
      + ADDED: aec_cmd_set_coefficient_index and aec_cmd_get_coefficient_index
      + ADDED: Improved algorithm performance
      + ADDED: Command to get filter coefficients
      + CHANGED: Reset command payload length to 1 byte
      + FIXED: Alignment bug
      + CHANGED: Compiler flags to reduce memory usage
      + CHANGED: Controls now use VTB helper functions
      + HOTFIX: Fix bug where setting bypass affected adaption config
      + ADDED: Additional control functions
      + CHANGED: aec_dump_paramters function name to aec_dump_parameters
      + REMOVED: aec_control_set_adaption function
      + REMOVED: aec_control_set_x_energy_delta function
      + REMOVED: aec_control_set_x_energy_gamma_log2 function
      + CHANGED: Command protocol to set MSB in all get commands
      + ADDED: test updates
      + FIXED: Division by zero bug

    - lib_agc: 1.0.0 -> 2.1.0

      + CHANGE: Fixed channel index bug.
      + CHANGE: Extended unit tests.
      + CHANGED: AGC adaptive algorithm.
      + CHANGED: Processing a frame requires VAD.
      + CHANGED: Renamed AGC_CHANNELS to AGC_INPUT_CHANNELS.
      + ADDED: Parameter get and set functions.
      + ADDED: Initial AGC config structure.

    - lib_audio_pipelines: 0.1.0 -> 0.8.2

      + UPDATED: sort test results in alphabetical order
      + FIXED: test_aec_performance now tests using only stage A
      + CHANGED: Reduced unsafe region in ap_stage_a
      + UPDATED: hotfix on lib_interference_canceller
      + ADDED: Communications channel on Stage C output
      + UPDATED: Using AGC v2.1.0
      + CHANGED: control function API in lib_vtb (v4.0.0)
      + ADDED: Control commands for reference audio on right output channel
      + ADDED: no processing audio pipelines stage
      + FIXED: Incorrect payload length for reset commands
      + UPDATED: Jenkins tests triggers
      + ADDED: tests for configurable delay
      + ADDED: instability tests
      + FIXED: number format errors
      + FIXED: command queue fragmentation
      + ADDED: more timers to facilitate pipeline optimisation
      + REMOVED: Obsolete build options
      + ADDED: Bypass, characterization, robustness and delay tests
      + FIXED: AEC delta control parameter
      + ADDED: return code checking to all stage command handlers
      + ADDED: support for Windows
      + ADDED: AEC, interference canceller and noise suppressor control
        robustness tests
      + ADDED: pre-AEC delay block
      + ADDED: Additional tests
      + ADDED: Configurable delay for references and mics
      + UPDATED: copyright notices
      + UPDATED: license file dates
      + FIXED: repetition in control code
      + FIXED: issue #196
      + FIXED: sensory setup test script
      + FIXED: stage b python vs. xc inconsistency
      + CHANGED: Control code to make use of VTB helper functions
      + CHANGED: Interference canceller parameters
      + CHANGED: Status register clears when read
      + CHANGED: Gain control uses AGC core (NOT automatic)
      + CHANGED: Interference canceller operation
      + ADDED: Mic array interface changes
      + ADDED: I2C control
      + ADDED: USB control
      + ADDED: Fixed gain control
      + ADDED: USB host code
      + ADDED: Support for unity
      + ADDED: Sensory license check
      + ADDED: Ability to configure DAC from xcore
      + ADDED: License file
      + CHANGED: Sensory operating point
      + REMOVED: I2C support for clock stretching

    - lib_interference_canceller: 1.4.2 -> 1.5.2

      + CHANGED: Noise minimisation off by default
      + FIXED: release error
      + ADDED: Noise minimisation get/set commands
      + CHANGED: Complex scaling to use VTB functions
      + FIXED: Silence condition performance problems

    - lib_voice_toolbox: 3.0.0 -> 4.0.0

      + CHANGED: add new control functions
      + CHANGED: added new complex functions

0.6.0
-----

  * ADDED: cron job in Jenkins
  * FIXED: audio pipeline instabilities
  * REMOVED: Undocumented commands from example host
  * REMOVED: old RPi hat app files
  * ADDED: usb_no_processing build

  * Changes to dependencies:

    - lib_agc: 0.0.2 -> 1.0.0

      + ADDED: Multiple channel support
      + ADDED: Gain and adaption control
      + ADDED: Unit tests
      + ADDED: Python and XC implementations
      + ADDED: Jenkinsfile

    - lib_i2s: Added dependency 3.0.0

      + REMOVED: Combined I2S and TDM master

    - lib_interference_canceller: 1.0.0 -> 1.4.2

      + FIXED: Reset IC filter control parameter
      + FIXED: Parameter correction in python and XC implementation
      + ADDED: ic_set_leakage_alpha control command
      + FIXED: memory initialization during bypass
      + FIXED: Python model behaviour
      + ADDED: Energy computations
      + ADDED: Memory optimisations
      + ADDED: Control commands
      + CHANGED: Control code now uses voice toolbox helper functions
      + CHANGED: Implementation of Algorithm
      + ADDED: Control interface
      + ADDED: Extra tests with directional performance measurement
      + ADDED: Support for xmake

    - lib_noise_suppression: 1.0.0 -> 1.2.0

      + CHANGED: Control code to use VTB helper functions
      + ADDED: control for MCRA noise floor
      + ADDED: Python test and comparison code
      + ADDED: Control interface and tests

    - lib_spdif: 3.0.0 -> 3.1.0

      + Add library wscript to enable applications built using xwaf

    - lib_vad: Added dependency 0.0.1

      + Initial version

    - lib_voice_toolbox: 1.0.2 -> 3.0.0

      + CHANGED: energy calculation
      + ADDED: headroom get function
      + FIXED: divide by 0 assert in vtb_div_u32_u32()
      + ADDED: Q format code
      + ADDED: Metadata functions
      + REMOVED: Duplicate code
      + ADDED: More control functions
      + FIXED: Reset commands now have payload length > 0
      + FIXED: Floating point sqrt
      + REMOVED: Duplicate code
      + ADDED: Support for more compact control code
      + ADDED: Energy functions and tests
      + FIXED: Memory alignment issues
      + ADDED: Steering function
      + CHANGED: vtb_get_fd_frame_power function return value
      + CHANGED: vtb_get_td_frame_power function return value
      + ADDED: Additional functions for local floating point implementation
      + ADDED: Frame formation function
      + ADDED: Tests for floating point functions

0.5.3
-----

  * ADDED: lib_device_control based DFU for all platforms
  * ADDED: dfu_control host app
  * REMOVED: lib_xua DFU implementation

0.5.2
-----

  * ADDED: Support for PI 24.576MHz MCLK

0.5.1
-----

  * ADDED: Configurable delay
  * ADDED: Support for AEC coefficient reading

0.5.0
-----

  * ADDED: Memory optimisations
  * ADDED: Control commands

0.4.1
-----

  * ADDED: Booting from flash for XK-XVF3510-L71 board
  * ADDED: I2S slave build for XK-XVF3510-L71 board
  * ADDED: hybrid USB for XK-XVF3510-L71 board

0.4.0
-----

  * ADDED: Controllable delay path

0.3.1
-----

  * FIXED: version number inconsistencies

0.3.0
-----

  * ADD: app for XK-XVF3510-L71 board

0.2.0
-----

  * UPDATED: Documentation

0.1.0
-----

  * ADDED: full pipeline examples for i2s_master/slave and USB configurations

  * Changes to dependencies:

    - audio_test_tools: Added dependency 1.0.0

    - lib_aec: Added dependency 1.0.3

      + ADDED: Support for xmake
      + Fixed two renamed debug functions and set the AEC start frequency to 0Hz
      + Added audio_test_tools to the dependencies

    - lib_agc: Added dependency 0.0.2

      + ADDED: Support for xmake
      + Copyrights, licences and dependencies

    - lib_audio_pipelines: Added dependency 0.1.0

      + ADDED: support for DFU via USB in XVF3510 example
      + ADDED: unit tests
      + ADDED: full pipeline examples for i2s_master/slave configurations on
        XVF3100 and XVF3510
      + ADDED: infrastructure for control interface and relative tests
      + ADDED: pipeline tests and plots for Jenkins
      + ADDED: benchmark and .wav tests for the different stages
      + ADDED: audio pipeline stages A, B and C
      + ADDED: Jenkins file

    - lib_device_control: Added dependency 3.2.4

      + Change to use lib_i2c 5.0.0

    - lib_dsp: Added dependency 4.1.0

      + Added post-FFT Hanning windowing
      + Added function to combine real and imaginary arrays into complex array
      + Added function to split complex array into real and imaginary arrays
      + Added 48-point DCT
      + Added dsp filter FIR add sample
      + Added softplus
      + Added integer sqrt
      + Documentation updates

    - lib_i2c: Added dependency 5.0.0

      + CHANGE: i2c_master_single_port no longer supported on XS1.
      + CHANGE: Removed the start_read_request() and start_write_request()
        functions from the i2c_slave_callback_if.
      + CHANGE: Removed the start_master_read() and start_master_write()
        functions from the i2c_slave_callback_if.
      + RESOLVED: Fixed timing of i2c master (both single port and multi-port).
      + RESOLVED: Fixed bug with the master not coping with clock stretching on
        start bits.

    - lib_interference_canceller: Added dependency 1.0.0

      + Initial version

    - lib_logging: Added dependency 2.1.1

      + CHANGE:   Test runner script now terminates correctly on Windows

    - lib_mic_array: Added dependency 3.1.1

      + Updated lib_dsp dependancy from 3.0.0 to 4.0.0

    - lib_noise_suppression: Added dependency 1.0.0

      + ADDED: Support for xmake
      + Update changelog and module_build_info

    - lib_spdif: Added dependency 3.0.0

      + spdif_tx() no longer configures port. Additional function
        spdif_tx_port_config() provided. Allows sharing of clockblock with other
        tasks

    - lib_src: Added dependency 1.1.1

      + RESOLVED: correct compensation factor for voice upsampling
      + ADDED: test of voice unity gain

    - lib_voice_toolbox: Added dependency 1.0.2

      + ADDED: Support for xmake
      + Removed old examples
      + Removed old code
      + Updated dependencies
      + Updated copyrights

    - lib_xassert: Added dependency 3.0.1

      + CHANGE: Correct dates in LICENSE.txt files

    - lib_xua: Added dependency 0.2.0

      + ADDED:     Initial library documentation
      + ADDED:     Application note AN00247: Using lib_xua with lib_spdif
        (transmit)
      + ADDED:     Separate callbacks for input/output audio stream start/stop
      + CHANGE:    I2S hardware resources no longer used globally and must be
        passed to XUA_AudioHub()
      + CHANGE:    XUA_AudioHub() no longer pars S/PDIF transmitter task
      + CHANGE:    Moved to lib_spdif (from module_spdif_tx & module_spdif_rx)
      + CHANGE:    Define NUM_PDM_MICS renamed to XUA_NUM_PDM_MICS
      + CHANGE:    Define NO_USB renamed to XUA_USB_EN
      + CHANGE:    Build files updated to support new "xcommon" behaviour in
        xwaf.
      + RESOLVED:  wChannelConfig in UAC1 descriptor set according to output
        channel count
      + RESOLVED:  Indexing of ADAT channel strings (#18059)
      + RESOLVED:  Rebooting device fails when PLL config "not reset" bit is set

    - lib_xud: Added dependency 0.1.1

      + RESOLVED:   Transmit timing fixes for U-series devices (introduced in
        sc_xud 2.3.0)
      + RESOLVED:   Continuous suspend/resume notifications when host
        disconnected (introduced in sc_xud 2.4.2, #11813)
      + RESOLVED:   Exception raised in GET_STATUS request when null pointer
        passed for high-speed configuration descriptor

0.0.1
-----

  * Initial version

