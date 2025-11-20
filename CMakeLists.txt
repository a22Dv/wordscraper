# NOTE (environment-specific):
# Due to issues regarding the cmake.useVsDeveloperEnvironment setting,
# it is required to run cmake --preset <configuration-preset> manually inside
# the VS Developer pwsh/cmd for any clean run to initialize the build directory,
# before relying on the cached variables from that run for subsequent 
# configurations using the VSCode CMake extension.

# KNOWN ISSUES (maybe environment-specific):
# clangd has issues parsing cl.exe's -external:I flag, use clang-cl to output -imsvc.
# clangd can do header inference for .hpp files which can break it if it grabs the wrong source file compile command to reference on.
# target_precompile_headers() masks this issue by explicitly citing a source file to compile the header with. 
# This does not guarantee that this issue won't pop up in the future.

# Configuration.
cmake_minimum_required(VERSION 4.0) # Not the absolute minimum required version. But bumped up for simplicity.

# Reconfigure if source files get added to rerun GLOB_RECURSE.
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS 
    "${CMAKE_SOURCE_DIR}/src/*.cpp" 
)
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS 
    "${CMAKE_SOURCE_DIR}/tests/*.cpp" 
)

# Top-level details.
set(PROJECT_NAME wordscraper)
set(PROJECT_LANGUAGE CXX)
set(PROJECT_STANDARD 20)
if (PROJECT_LANGUAGE STREQUAL "CXX")
    set(CMAKE_CXX_STANDARD ${PROJECT_STANDARD})
elseif (PROJECT_LANGUAGE STREQUAL "C")
    set(CMAKE_C_STANDARD ${PROJECT_STANDARD})
endif()
project(${PROJECT_NAME} ${PROJECT_LANGUAGE})

add_library(${PROJECT_NAME}_LIB STATIC)

# Packages
include(FetchContent)
FetchContent_Declare(
    Tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy
    GIT_TAG v0.13.0
)
FetchContent_MakeAvailable(Tracy)
find_package(OpenCV CONFIG REQUIRED)
set(PACKAGE_INCLUDES ${OpenCV_INCLUDE_DIRS})
set(PACKAGE_LIBS ${OpenCV_LIBS} Tracy::TracyClient)

# Files/Dependencies
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
target_sources(${PROJECT_NAME}_LIB PRIVATE ${SOURCES})
target_include_directories(${PROJECT_NAME}_LIB PUBLIC "${CMAKE_SOURCE_DIR}/include" ${PACKAGE_INCLUDES})
target_precompile_headers(${PROJECT_NAME}_LIB PUBLIC "${CMAKE_SOURCE_DIR}/include/core/pch.hpp")
target_link_libraries(${PROJECT_NAME}_LIB PUBLIC ${PACKAGE_LIBS}) # No-op by default.

# Compile definitions/options
set(BASE_COMPILE_OPTIONS /WX /W4 /permissive-) 

if (PROJECT_LANGUAGE STREQUAL "CXX")
    list(APPEND BASE_COMPILE_OPTIONS /EHsc) # Exception support.
endif()

target_compile_options(${PROJECT_NAME}_LIB PUBLIC ${BASE_COMPILE_OPTIONS})
target_compile_definitions(${PROJECT_NAME}_LIB PUBLIC) # No-op by default.

add_executable(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/app/main.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_LIB)

# Commands
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data"
    "${CMAKE_BINARY_DIR}/data"
)

# Tests 
set(TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/tests")
file(GLOB_RECURSE TESTS "${CMAKE_SOURCE_DIR}/tests/*.cpp")
foreach(TEST_SOURCE ${TESTS})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE ${PROJECT_NAME}_LIB)

    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TEST_OUTPUT_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TEST_OUTPUT_DIR}
    )
endforeach()








