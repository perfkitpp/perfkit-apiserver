cmake_minimum_required(VERSION 3.16)
project(perfkit_apiserver)

# ============== IMPORT THIRD PARTY LIBRARIES
find_package(Threads)
find_package(ZLIB)
find_package(OpenSSL)
find_package(spdlog CONFIG REQUIRED)

set(perfkit_BUILD_NET_CLIENT ON)
set(perfkit_BUILD_CLI ON)
set(perfkit_BUILD_EXAMPLES ON)
add_subdirectory(third/perfkit EXCLUDE_FROM_ALL)

add_library(cppcrow INTERFACE)
target_include_directories(cppcrow INTERFACE third/crow/include/)
target_compile_features(cppcrow INTERFACE cxx_std_17)

# ============== CONFIGURE BUILD TYPE
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

# ============== CONFIGURE OPENSSL
if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif ()

# ============== CONFIGURE BOOST
if (MSVC)
    set(Boost_USE_STATIC_LIBS "On")
    find_package(Boost 1.70 COMPONENTS system thread regex REQUIRED)
else ()
    find_package(Boost 1.70 COMPONENTS system thread REQUIRED)
endif ()

# ============== ADD TARGETS
add_subdirectory(src)
