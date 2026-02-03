set(TARGET llama-cli)
add_executable(${TARGET} main.cpp)
target_link_libraries(${TARGET} PRIVATE common llama ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(${TARGET} PRIVATE cxx_std_17)

if (RAFAELIA_BAREMETAL)
    target_link_libraries(${TARGET} PRIVATE rafaelia-baremetal)
endif()

if(LLAMA_TOOLS_INSTALL)
    install(TARGETS ${TARGET} RUNTIME)
endif()
