How to build host apps
======================

**Note**: To build the XVF3510 host apps on Windows you need to use the **x86 Native Tools Command Prompt for VS 2017** or a newer VS version, but it must be an x86 Command Prompt.
**Note**: To build the XVF3510 host apps on Linux you need to install the following packages::

    sudo apt-get install -y build-essential
    sudo apt-get install -y pkg-config
    sudo apt-get install -y libusb-1.0-0-dev

VfCtrl USB host app
-------------------

  1. Open a terminal and go to folder ``host/src/vfctrl/sw_xvf3510/app_xk_xvf3510_l71/host/dsp_control`` located inside the release package:

  2. Delete the file CMakeCache.txt if present

  3. From the terminal type:

     * on WINDOWS::

        cmake -G "NMake Makefiles" -S . -Wno-dev
        nmake``

     * on Linux, MAC and RaspberryPi::

        cmake .
        make

VfCtrl I2C host app (RaspberryPi only)
--------------------------------------

  1. Open a terminal and go to folder ``host/src/vfctrl/sw_xvf3510/app_xk_xvf3510_l71/host/dsp_control`` located inside the release package:

  2. Delete the file CMakeCache.txt if present

  3. From the terminal type::

    cmake . -DI2C=ON
    make

VfCtrl JSON host app
--------------------

This host app is used only during the generation of the data partition images.

  1. Open a terminal and go to folder ``host/src/vfctrl/sw_xvf3510/app_xk_xvf3510_l71/host/dsp_control`` located inside the release package:

  2. Delete the file CMakeCache.txt if present

  3. From the terminal type:

     * on WINDOWS::

        cmake -G "NMake Makefiles" -S . -Wno-dev -DJSON=ON
        nmake

     * on Linux, MAC and RaspberryPi::

        cmake . -DJSON=ON
        make

DFU USB host app
----------------

  1. Open a terminal and go to folder ``host/src/vfctrl/sw_xvf3510/app_xk_xvf3510_l71/host/dfu_control`` located inside the release package:

  2. Delete the file CMakeCache.txt if present

  3. From the terminal type:

     * on WINDOWS::

        cmake -G "NMake Makefiles" -S . -Wno-dev
        nmake

     * on Linux, MAC and RaspberryPi::

        cmake .
        make

DFU I2C host app (RaspberryPi only)
-----------------------------------

  1. Open a terminal and go to folder ``host/src/vfctrl/sw_xvf3510/app_xk_xvf3510_l71/host/dfu_control`` located inside the release package:

  2. Delete the file CMakeCache.txt if present

  3. From the terminal type::

    cmake . -DI2C=ON
    make

Data partition generator
------------------------

This host app is used only during the generation of the data partition images.

  1. Open a terminal and go to folder ``host/src/dpgen/ls lib_flash_data_partition/host/data_partition_generator`` located inside the release package:

  2. Delete the file CMakeCache.txt if present

  3. From the terminal type:

     * on WINDOWS::

        cmake -G "NMake Makefiles" -S . -Wno-dev
        nmake

     * on Linux, MAC and RaspberryPi::

        cmake .
        make

DFU USB Android app
-------------------

   1. Open on Android Studio the project below from the release package::

        src/vfctrl/sw_xvf3510/app_xk_xvf3510_l71/host/android/VfctrlApp

   2. Build and run the project from Android Studio
