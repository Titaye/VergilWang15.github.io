cmake_minimum_required(VERSION 3.7)

# Force release mode so that all the DLLs are statically linked
set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Only Release mode is allowed" FORCE)

option(I2C "I2C" OFF)
option(JSON "JSON" OFF)

set (DEFINES _GNU_SOURCE HOST_APP)

if (JSON)
    set (INCLUDE_DIRS 
        "../../../../lib_device_control/lib_device_control/api"
        "api"
    )
else()
    set (INCLUDE_DIRS 
        "../../../../lib_device_control/lib_device_control/api"
        "../../../../lib_device_control/lib_device_control/src"
        "../../../../lib_device_control/lib_device_control/host"
        "api"
    )
endif()
set (SOURCE_FILES
        src/host.c
)

set (LINK_LIBS)
if (JSON)
    set (VFCTRL_LIB vfctrl_json_1.0)
    set (VFCTRL_APP vfctrl_json)
    set (DEFINES ${DEFINES} JSON_ONLY)
    set (SOURCE_FILES ${SOURCE_FILES} src/json_only.c)
elseif(I2C)
    set (VFCTRL_LIB vfctrl_i2c_1.0)
    set (VFCTRL_APP vfctrl_i2c)
    set (DEFINES ${DEFINES} USE_I2C RPI)
    set (SOURCE_FILES ${SOURCE_FILES} ../../../../lib_device_control/lib_device_control/host/device_access_i2c_rpi.c)
else() 
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

    set (VFCTRL_LIB vfctrl_usb_1.0)
    set (VFCTRL_APP vfctrl_usb)
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
endif()


add_library(${VFCTRL_LIB} STATIC ${SOURCE_FILES})
target_compile_definitions(${VFCTRL_LIB} PUBLIC ${DEFINES})
target_include_directories(${VFCTRL_LIB} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${VFCTRL_LIB} ${LINK_LIBS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(${VFCTRL_APP} src/main.c)
target_include_directories(${VFCTRL_APP} PUBLIC "api")
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(${VFCTRL_APP} ${VFCTRL_LIB})
else()
    target_link_libraries(${VFCTRL_APP} ${VFCTRL_LIB} m)
endif()

if (NOT I2C AND NOT JSON)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_custom_command(TARGET ${VFCTRL_APP}
        POST_BUILD COMMAND 
        ${CMAKE_INSTALL_NAME_TOOL} -change "/usr/local/lib/libusb-1.0.0.dylib" "@executable_path/libusb-1.0.0.dylib" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${VFCTRL_APP}
        )
add_custom_command(
    TARGET ${VFCTRL_APP} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/libusb/OSX64/libusb-1.0.0.dylib
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                )
endif()
endif()

