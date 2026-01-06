# Rafaelia Baremetal Module CMake Configuration
# Copyright (c) 2026 Rafael Melo Reis
# Licensed under MIT License

cmake_minimum_required(VERSION 3.14)
project(rafaelia-baremetal C)

# Compilation flags for baremetal - no external dependencies
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Optimization flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -march=native")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")

# Option to build the baremetal module
option(RAFAELIA_BAREMETAL "Build Rafaelia Baremetal Module" ON)

if(RAFAELIA_BAREMETAL)
    message(STATUS "Building Rafaelia Baremetal Module")
    
    # Include directories
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/bitraf
        ${CMAKE_CURRENT_SOURCE_DIR}/zipraf
        ${CMAKE_CURRENT_SOURCE_DIR}/rafstore
        ${CMAKE_CURRENT_SOURCE_DIR}/toroid
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware
    )
    
    # Core library sources
    set(RAFAELIA_CORE_SOURCES
        core/raf_core.c
        core/raf_core.h
    )
    
    # BITRAF library sources
    set(RAFAELIA_BITRAF_SOURCES
        bitraf/raf_bitraf.c
        bitraf/raf_bitraf.h
    )
    
    # ZIPRAF library sources
    set(RAFAELIA_ZIPRAF_SOURCES
        zipraf/raf_zipraf.c
        zipraf/raf_zipraf.h
    )
    
    # RAFSTORE library sources
    set(RAFAELIA_RAFSTORE_SOURCES
        rafstore/raf_rafstore.c
        rafstore/raf_rafstore.h
    )
    
    # TOROID library sources
    set(RAFAELIA_TOROID_SOURCES
        toroid/raf_toroid.c
        toroid/raf_toroid.h
    )
    
    # Hardware detection sources
    set(RAFAELIA_HARDWARE_SOURCES
        hardware/raf_hardware.c
        hardware/raf_hardware.h
    )
    
    # Utility tools sources (42 tools)
    set(RAFAELIA_UTILS_SOURCES
        utils/raf_utils.c
        utils/raf_utils.h
    )
    
    # Create static library combining all modules
    add_library(rafaelia-baremetal STATIC
        ${RAFAELIA_CORE_SOURCES}
        ${RAFAELIA_BITRAF_SOURCES}
        ${RAFAELIA_ZIPRAF_SOURCES}
        ${RAFAELIA_RAFSTORE_SOURCES}
        ${RAFAELIA_TOROID_SOURCES}
        ${RAFAELIA_HARDWARE_SOURCES}
        ${RAFAELIA_UTILS_SOURCES}
    )
    
    # Link math library if needed
    if(UNIX AND NOT APPLE)
        target_link_libraries(rafaelia-baremetal m)
    endif()
    
    # Install headers
    install(FILES
        core/raf_core.h
        bitraf/raf_bitraf.h
        zipraf/raf_zipraf.h
        rafstore/raf_rafstore.h
        toroid/raf_toroid.h
        hardware/raf_hardware.h
        utils/raf_utils.h
        DESTINATION include/rafaelia-baremetal
    )
    
    # Install library
    install(TARGETS rafaelia-baremetal
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )
    
    # Create a master header for convenience
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/rafaelia_baremetal.h
"/* Rafaelia Baremetal - Master Header */
#ifndef RAFAELIA_BAREMETAL_H
#define RAFAELIA_BAREMETAL_H

#include \"core/raf_core.h\"
#include \"bitraf/raf_bitraf.h\"
#include \"zipraf/raf_zipraf.h\"
#include \"rafstore/raf_rafstore.h\"
#include \"toroid/raf_toroid.h\"
#include \"hardware/raf_hardware.h\"
#include \"utils/raf_utils.h\"

#endif /* RAFAELIA_BAREMETAL_H */
"
    )
    
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rafaelia_baremetal.h
        DESTINATION include
    )
    
    # Build test executable
    option(RAFAELIA_BUILD_TESTS "Build Rafaelia Baremetal tests" ON)
    if(RAFAELIA_BUILD_TESTS)
        add_executable(rafaelia-test test.c)
        target_link_libraries(rafaelia-test rafaelia-baremetal)
        
        # Also build example
        add_executable(rafaelia-example example.c)
        target_link_libraries(rafaelia-example rafaelia-baremetal)
    endif()
    
endif()
