cmake_minimum_required(VERSION 3.7)

# Force release mode so that all the DLLs are statically linked
set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Only Release mode is allowed" FORCE)

option(I2C "I2C" OFF)

set (DEFINES _GNU_SOURCE HOST_APP)

set (INCLUDE_DIRS 
        "../../../../lib_device_control/lib_device_control/api"
        "../../../../lib_device_control/lib_device_control/src"
        "../../../../lib_device_control/lib_device_control/host"
        "../../../../lib_dfu/lib_dfu/api"
        "../../../../lib_dfu/host/libsuffix_verifier"
        "../../../app_xk_xvf3510_l71/src"
        "api"
        "src"
)

set (SOURCE_FILES
        src/argument_parser.c
        src/hal.c
        src/input_reader.c
        src/operations.c
        src/sleep.c
        src/labels.c
        ../../../../lib_dfu/host/libsuffix_verifier/crc.c
        ../../../../lib_dfu/host/libsuffix_verifier/suffix_verifier.c
        ../../../../lib_device_control/lib_device_control/host/util.c
)

set (LINK_LIBS)

if (NOT I2C)
    # Assuming USB
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        link_directories("libusb/OSX64")
        set(libusb-1.0_INCLUDE_DIRS "libusb/OSX64")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        find_package(PkgConfig)
        pkg_check_modules(libusb-1.0 REQUIRED libusb-1.0)
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        link_directories("libusb/Win32")
        set(libusb-1.0_INCLUDE_DIRS "libusb/Win32")
    endif()

    set (DFUCTRL_LIB dfuctrl_usb_1.0)
    set (DFUCTRL_APP dfu_usb)
    set (DEFINES ${DEFINES} USE_USB)
    set (INCLUDE_DIRS ${INCLUDE_DIRS} ${libusb-1.0_INCLUDE_DIRS})
    set (SOURCE_FILES ${SOURCE_FILES} ../../../../lib_device_control/lib_device_control/host/device_access_usb.c)

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(LINK_LIBS ${LINK_LIBS} usb-1.0.0)
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(LINK_LIBS ${LINK_LIBS} usb-1.0)
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set (DEFINES ${DEFINES} nologo W4 WX- O2 EHa _CRT_SECURE_NO_WARNINGS)
        set(LINK_LIBS ${LINK_LIBS} libusb)
    endif()
else()
    set (DFUCTRL_LIB dfuctrl_i2c_1.0)
    set (DFUCTRL_APP dfu_i2c)
    set (DEFINES ${DEFINES} USE_I2C RPI)
    set (SOURCE_FILES ${SOURCE_FILES} ../../../../lib_device_control/lib_device_control/host/device_access_i2c_rpi.c)
endif()


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_library(${DFUCTRL_LIB} STATIC ${SOURCE_FILES})
target_compile_definitions(${DFUCTRL_LIB} PUBLIC ${DEFINES})
target_include_directories(${DFUCTRL_LIB} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${DFUCTRL_LIB} ${LINK_LIBS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(${DFUCTRL_APP} src/main.c)
target_include_directories(${DFUCTRL_APP} PUBLIC "api")
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(${DFUCTRL_APP} ${DFUCTRL_LIB})
else()
    target_link_libraries(${DFUCTRL_APP} ${DFUCTRL_LIB} m)
endif()

if (NOT I2C)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_custom_command(TARGET ${DFUCTRL_APP}
        POST_BUILD COMMAND 
        ${CMAKE_INSTALL_NAME_TOOL} -change "/usr/local/lib/libusb-1.0.0.dylib" "@executable_path/libusb-1.0.0.dylib" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${DFUCTRL_APP}
        )
add_custom_command(
    TARGET ${DFUCTRL_APP} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/libusb/OSX64/libusb-1.0.0.dylib
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                )
endif()
endif()


