cmake_minimum_required(VERSION 3.16)
project(Credis LANGUAGES CXX)

# ---- Language standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Default to an optimized build ----
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- Optional sanitizers: cmake -B build -DSANITIZE=address  (or thread) ----
set(SANITIZE "" CACHE STRING "Sanitizer to enable: address or thread")

find_package(Threads REQUIRED)

# ---- Core library: everything except the two main() functions ----
add_library(credis_core STATIC
    src/response_seralizer/resp.cc
    src/executor/executor.cc
    src/net_connection/credis_init.cc
    src/net_connection/credis_run.cc
    src/persistence/aof.cc
)
target_include_directories(credis_core PUBLIC src/includes)
target_compile_options(credis_core PUBLIC -Wall -Wextra -Wshadow)
target_link_libraries(credis_core PUBLIC Threads::Threads)

if(SANITIZE)
    target_compile_options(credis_core PUBLIC -fsanitize=${SANITIZE} -fno-omit-frame-pointer)
    target_link_options(credis_core PUBLIC -fsanitize=${SANITIZE})
endif()

# ---- The server ----
add_executable(credis src/main.cc)
target_link_libraries(credis PRIVATE credis_core)

# ---- The tests ----
enable_testing()
add_executable(credis_tests
    src/tests/test_main.cc
    src/tests/tester.cc
)
target_link_libraries(credis_tests PRIVATE credis_core)
# Tests use assert(), so keep it active even in Release builds
target_compile_options(credis_tests PRIVATE -UNDEBUG)
add_test(NAME unit_tests COMMAND credis_tests)