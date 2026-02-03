# Rafaelia Baremetal Module CMake Configuration
# Copyright (c) 2026 Rafael Melo Reis
# Licensed under MIT License

cmake_minimum_required(VERSION 3.14)
project(rafaelia-baremetal C)

# Compilation flags for baremetal - no external dependencies
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Option for native CPU optimization (off by default for portability)
option(RAFAELIA_NATIVE_OPTIMIZE "Enable native CPU optimizations (-march=native)" OFF)

# Optimization flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Release flags: portable by default, native optimization if requested
if(RAFAELIA_NATIVE_OPTIMIZE)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -march=native -mtune=native")
    message(STATUS "Rafaelia: Building with native CPU optimizations (non-portable)")
else()
    # Use portable flags that work across different CPUs of the same architecture
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
    message(STATUS "Rafaelia: Building with portable optimizations")
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")

# Option to build the baremetal module
option(RAFAELIA_BAREMETAL "Build Rafaelia Baremetal Module" ON)

if(RAFAELIA_BAREMETAL)
    message(STATUS "Building Rafaelia Baremetal Module")
    
    # Include directories
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/bitraf
        ${CMAKE_CURRENT_SOURCE_DIR}/bitstack
        ${CMAKE_CURRENT_SOURCE_DIR}/zipraf
        ${CMAKE_CURRENT_SOURCE_DIR}/rafstore
        ${CMAKE_CURRENT_SOURCE_DIR}/toroid
        ${CMAKE_CURRENT_SOURCE_DIR}/geometry
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware
        ${CMAKE_CURRENT_SOURCE_DIR}/rafaelia
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    
    # Core library sources
    set(RAFAELIA_CORE_SOURCES
        core/raf_core.c
        core/raf_core.h
        core/raf_platform.c
        core/raf_platform.h
        core/raf_system.c
        core/raf_system.h
    )
    
    # BITRAF library sources
    set(RAFAELIA_BITRAF_SOURCES
        bitraf/raf_bitraf.c
        bitraf/raf_bitraf.h
    )
    
    # BITSTACK library sources (new low-level logic module)
    set(RAFAELIA_BITSTACK_SOURCES
        bitstack/raf_bitstack.c
        bitstack/raf_bitstack.h
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
        rafstore/raf_hd_cache.c
        rafstore/raf_hd_cache.h
    )
    
    # TOROID library sources
    set(RAFAELIA_TOROID_SOURCES
        toroid/raf_toroid.c
        toroid/raf_toroid.h
    )

    # Geometry nucleus sources
    set(RAFAELIA_GEOMETRY_SOURCES
        geometry/raf_geometry.c
        geometry/raf_geometry.h
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
    
    # RAFAELIA module sources (formulas and ND vectors)
    set(RAFAELIA_MODULE_SOURCES
        rafaelia/raf_rafaelia_vector.c
        rafaelia/raf_rafaelia_vector.h
        rafaelia/raf_rafaelia_core.c
        rafaelia/raf_rafaelia_core.h
        rafaelia/raf_rafaelia_math.c
        rafaelia/raf_rafaelia_math.h
    )

    # LLaMA integration bridge sources
    set(RAFAELIA_LLAMA_BRIDGE_SOURCES
        rafaelia/raf_llama_bridge.c
        include/rafaelia.h
    )
    
    # Create static library combining all modules
    add_library(rafaelia-baremetal STATIC
        ${RAFAELIA_CORE_SOURCES}
        ${RAFAELIA_BITRAF_SOURCES}
        ${RAFAELIA_BITSTACK_SOURCES}
        ${RAFAELIA_ZIPRAF_SOURCES}
        ${RAFAELIA_RAFSTORE_SOURCES}
        ${RAFAELIA_TOROID_SOURCES}
        ${RAFAELIA_GEOMETRY_SOURCES}
        ${RAFAELIA_HARDWARE_SOURCES}
        ${RAFAELIA_UTILS_SOURCES}
        ${RAFAELIA_MODULE_SOURCES}
        ${RAFAELIA_LLAMA_BRIDGE_SOURCES}
    )

    target_include_directories(rafaelia-baremetal PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    
    # Link math library if needed
    if(UNIX AND NOT APPLE)
        target_link_libraries(rafaelia-baremetal m)
    endif()
    
    # Install headers
    install(FILES
        core/raf_core.h
        core/raf_platform.h
        core/raf_system.h
        bitraf/raf_bitraf.h
        bitstack/raf_bitstack.h
        zipraf/raf_zipraf.h
        rafstore/raf_rafstore.h
        rafstore/raf_hd_cache.h
        toroid/raf_toroid.h
        geometry/raf_geometry.h
        hardware/raf_hardware.h
        utils/raf_utils.h
        rafaelia/raf_rafaelia_vector.h
        rafaelia/raf_rafaelia_core.h
        rafaelia/raf_rafaelia_math.h
        include/rafaelia.h
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
#include \"core/raf_platform.h\"
#include \"core/raf_system.h\"
#include \"bitraf/raf_bitraf.h\"
#include \"bitstack/raf_bitstack.h\"
#include \"zipraf/raf_zipraf.h\"
#include \"rafstore/raf_rafstore.h\"
#include \"rafstore/raf_hd_cache.h\"
#include \"toroid/raf_toroid.h\"
#include \"geometry/raf_geometry.h\"
#include \"hardware/raf_hardware.h\"
#include \"utils/raf_utils.h\"
#include \"rafaelia/raf_rafaelia_vector.h\"
#include \"rafaelia/raf_rafaelia_core.h\"
#include \"rafaelia/raf_rafaelia_math.h\"
#include \"rafaelia.h\"

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
        
        # Build integration example
        add_executable(rafaelia-integration integration_example.c)
        target_link_libraries(rafaelia-integration rafaelia-baremetal)
        
        # Build RAFAELIA module example
        add_executable(rafaelia-module-example rafaelia/example_rafaelia.c)
        target_link_libraries(rafaelia-module-example rafaelia-baremetal)
        
        # Build HD Cache example
        add_executable(rafaelia-hd-cache-example rafstore/example_hd_cache.c)
        target_link_libraries(rafaelia-hd-cache-example rafaelia-baremetal)
    endif()
    
endif()
