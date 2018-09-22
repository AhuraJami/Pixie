cmake_minimum_required(VERSION 3.8.0)
#==========================================================================================
# Read and import modules in cmake directory
#==========================================================================================
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
include(Message)

#=========================================================================================
# Define the target and add all source and header files to it
#=========================================================================================
add_library(Pixie SHARED Source/Core/Engine.cpp)

# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(Pixie::Pixie ALIAS Pixie)
#=========================================================================================
# Set the include directories for building and installation
#=========================================================================================
set(PIXIE_INCLUDE_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/Include/Pixie)
set(PIXIE_SOURCE_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_include_directories(Pixie
        PUBLIC
            $<INSTALL_INTERFACE:include/pixie>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include/>
        PRIVATE
            ${PIXIE_SOURCE_DIR})

#=========================================================================================
# Add Source and header files
#=========================================================================================
target_sources(Pixie
    PUBLIC
        ${PIXIE_INCLUDE_DIR}/Concepts/Virtual/Begin.h
        ${PIXIE_INCLUDE_DIR}/Concepts/Virtual/Tick.h
        ${PIXIE_INCLUDE_DIR}/Concepts/Virtual/End.h

        ${PIXIE_INCLUDE_DIR}/Concepts/Object.h
        ${PIXIE_INCLUDE_DIR}/Concepts/Tickable.h

        ${PIXIE_INCLUDE_DIR}/Core/Core.h
        ${PIXIE_INCLUDE_DIR}/Core/ObjectInitializer.h
        ${PIXIE_INCLUDE_DIR}/Core/Engine/Clock.h
        ${PIXIE_INCLUDE_DIR}/Core/Engine/Engine.h
        ${PIXIE_INCLUDE_DIR}/Core/Scene/Scene.h
        ${PIXIE_INCLUDE_DIR}/Core/Scene/Forest.h

        ${PIXIE_INCLUDE_DIR}/Misc/Placeholders.h
        ${PIXIE_INCLUDE_DIR}/Misc/PixieExports.h

        ${PIXIE_INCLUDE_DIR}/Utility/TypeTraits.h
        ${PIXIE_INCLUDE_DIR}/Utility/Chrono.h
    PRIVATE
        ${PIXIE_SOURCE_DIR}/Core/Core.cpp
        ${PIXIE_SOURCE_DIR}/Core/Scene.cpp
)

#=========================================================================================
# Set up Pixie compile options
#=========================================================================================\
# define EXPORT preprocessors
target_compile_definitions(Pixie PRIVATE PIXIE_DLL_EXPORTS=1)

# Set language standard
target_compile_features(Pixie
   PRIVATE
       cxx_std_17)

# Add compile flags
target_compile_options(Pixie
    PUBLIC
        # GNU (g++)
        $<$<CXX_COMPILER_ID:GNU>:$<BUILD_INTERFACE:-Wall; -Wextra; -Wpedantic>>
        # Clang
        $<$<CXX_COMPILER_ID:Clang>:$<BUILD_INTERFACE:-Wall; -Wextra; -Wpedantic>>
        # MSVC
        $<$<CXX_COMPILER_ID:MSVC>:$<BUILD_INTERFACE:/W4; /permissive->>)

# Set output build and output directories so that test executables end up next to dll's
set_target_properties(Pixie PROPERTIES
    FOLDER                    Pixie
    ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
        
#=========================================================================================
# Add thirdparty libraries
#=========================================================================================
# It seems like all of std::variant's features are not available until
# AppleClang version 10.1 which unfortunately Travis CI doesn't support
# yet. So for now, we're going to use mpark::variable instead of the
# standard one for AppleClang platform
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    include(MParkVariant)
    target_compile_definitions(Pixie PUBLIC __APPLE_CLANG__)
endif()

#=========================================================================================
# Enable unit tests
#=========================================================================================
if(PIXIE_BUILD_UNIT_TESTS)
    include(GoogleTest)
    enable_google_test(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/googletest)

    add_subdirectory(Test)
endif()