# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set (LIB_DEVICE_CONTROL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../lib_device_control/lib_device_control")
set (DSP_HOST_APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../dsp_control")

set (INCLUDE_DIRS
    .
    )

set (VFCTRL_LIB vfctrl_usb_1.0)

set (VFCTRL_INCLUDE_FILES
        "${DSP_HOST_APP_DIR}/api"
        "${LIB_DEVICE_CONTROL_DIR}/api"
        "${LIB_DEVICE_CONTROL_DIR}/src"
        "${LIB_DEVICE_CONTROL_DIR}/host"
        "libusb/Android"
    )

set (XMOS_DEVICE_CONTROL_INCLUDE
        "standalone_app"
        "standalone_app/include"
    )

set (INCLUDE_DIRS ${INCLUDE_DIRS} ${VFCTRL_INCLUDE_FILES} ${SYNTIANT_IIF_INCLUDE_FILES} ${SYNTIANT_INCLUDE_FILES} ${XMOS_DEVICE_CONTROL_INCLUDE})


set (VFCTRL_SRC_FILES
        ${DSP_HOST_APP_DIR}/src/host.c
        ${LIB_DEVICE_CONTROL_DIR}/host/util.c
        #TODO: update device_access_usb in lib_device_control with the changes in the local file if we want to use the unmodified file
        lib_device_control/lib_device_control/host/device_access_usb.c
    )

set (XMOS_DEVICE_CONTROL_SOURCE_FILES
        standalone_app/xmos_device_manager.c
)


set(VFCTRL_LINK_LIBS ${CMAKE_SOURCE_DIR}/libusb/Android/${CMAKE_ANDROID_ARCH_ABI}/libusb1.0.so)

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp

        ${VFCTRL_SRC_FILES}
        ${XMOS_DEVICE_CONTROL_SOURCE_FILES}
        )
            


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

          target_include_directories(native-lib PUBLIC ${INCLUDE_DIRS})
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
        )

target_compile_definitions(native-lib PUBLIC USE_USB __arm__)
target_include_directories(native-lib PUBLIC ${INCLUDE_DIRS})
target_link_libraries(native-lib ${VFCTRL_LINK_LIBS})
