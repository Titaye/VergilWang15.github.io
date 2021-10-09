cmake_minimum_required(VERSION 3.7)

# Force release mode so that all the DLLs are statically linked
set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Only Release mode is allowed" FORCE)

set (DEFINES _GNU_SOURCE HOST_APP)

set (INCLUDE_DIRS 
        "../../lib_flash_data_partition/api"
        "../../lib_flash_data_partition/src"
)

set (SOURCE_FILES
        src/checksum.c
        src/crc.c
        src/input_reader.c
        src/output_writer.c
        src/parser.c
        src/renderer.c
        src/tokeniser.c
        src/data_partition_generator.c
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (DP_GEN data_partition_generator)
add_executable(${DP_GEN} ${SOURCE_FILES})
target_link_libraries(${DP_GEN} )
target_include_directories(${DP_GEN} PRIVATE ${INCLUDE_DIRS})

