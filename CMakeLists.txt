cmake_minimum_required(VERSION 3.20)

project(cporechop VERSION 1.0.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# C++ Standard: seqan3 requires __cplusplus >= 202100 (C++23)
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Compiler flags
# ---------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
endif()

# ---------------------------------------------------------------------------
# Dependencies via find_package (system-installed via micromamba)
# ---------------------------------------------------------------------------
find_package(seqan3 3.4 REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)
find_package(ZLIB REQUIRED)

find_package(CLI11 QUIET)
if(NOT CLI11_FOUND)
    FetchContent_Declare(CLI11 GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git GIT_TAG v2.4.2)
    FetchContent_MakeAvailable(CLI11)
endif()

# ---------------------------------------------------------------------------
# cporechop — shared library
# ---------------------------------------------------------------------------
add_library(cporechop_lib SHARED
    src/adapter_file.cpp
    src/align.cpp
    src/barcode.cpp
    src/cli.cpp
    src/detect.cpp
    src/full_barcode.cpp
    src/identity.cpp
    src/io.cpp
    src/middle.cpp
    src/output.cpp
    src/pipeline.cpp
    src/scoring.cpp
    src/threads.cpp
    src/trim.cpp
)

target_include_directories(cporechop_lib
    PUBLIC  include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(cporechop_lib
    PUBLIC  seqan3::seqan3
    PUBLIC  CLI11::CLI11
    PUBLIC  Boost::thread
    PUBLIC  ZLIB::ZLIB
)

set_target_properties(cporechop_lib PROPERTIES
    OUTPUT_NAME "cporechop"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# ---------------------------------------------------------------------------
# cporechop — CLI executable
# ---------------------------------------------------------------------------
add_executable(cporechop
    src/main.cpp
)

target_link_libraries(cporechop
    PRIVATE cporechop_lib
)

set_target_properties(cporechop PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    BUILD_RPATH "$ORIGIN/../lib"
)

# ---------------------------------------------------------------------------
# Portability: bundle shared libraries for redistribution
# ---------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # Statically link the C++ runtime so libstdc++/libgcc don't need bundling
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_link_options(cporechop PRIVATE -static-libgcc -static-libstdc++)
    endif()

    # RPATH: look for bundled .so files relative to the executable
    set_target_properties(cporechop PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
    )
endif()

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
install(TARGETS cporechop
    RUNTIME DESTINATION bin
)

install(TARGETS cporechop_lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
)

install(DIRECTORY include/porechop
    DESTINATION include
)

# Bundle shared library dependencies (Boost, ZLIB) for portable packages
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    install(CODE [[
        file(GET_RUNTIME_DEPENDENCIES
            EXECUTABLES "${CMAKE_INSTALL_PREFIX}/bin/cporechop"
            RESOLVED_DEPENDENCIES_VAR _resolved
            UNRESOLVED_DEPENDENCIES_VAR _unresolved
            PRE_EXCLUDE_REGEXES "ld-linux"
            POST_EXCLUDE_REGEXES "^/lib" "^/usr/lib"
        )
        foreach(_file IN LISTS _resolved)
            file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
                TYPE FILE FOLLOW_SYMLINK_CHAIN
                FILES "${_file}"
            )
        endforeach()
        if(_unresolved)
            message(WARNING "Could not resolve runtime dependencies: ${_unresolved}")
        endif()
    ]])
endif()

# ---------------------------------------------------------------------------
# CPack packaging
# ---------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME            "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR          "Yueqi Jin")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "C++20 reimplementation of Porechop — adapter trimming and demultiplexing for Oxford Nanopore reads")
set(CPACK_PACKAGE_VERSION         "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR   "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR   "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH   "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_HOMEPAGE_URL    "https://github.com/YueqiJin/cporechop")
set(CPACK_RESOURCE_FILE_LICENSE   "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README    "${CMAKE_SOURCE_DIR}/README.md")

# Generators: .tar.gz on Linux/macOS; .zip on Windows
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CPACK_GENERATOR "TGZ")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ")
elseif(WIN32)
    set(CPACK_GENERATOR "ZIP")
endif()

# Source package
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    "/build/"
    "/\\\\.cpm-cache/"
    "/\\\\.git/"
    "/\\\\.sisyphus/"
    "\\\\.DS_Store$"
    "\\\\.o$"
)

include(CPack)
