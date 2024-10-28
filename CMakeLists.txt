# Example CMake command line to create project build files:
#
# *** Windows ***
# cmake -G "Visual Studio 17 2022" -A Win32 -B ../AsyncStateMachineBuild -S .
#
# *** Linux ***
# cmake -G "Unix Makefiles" -B ../AsyncStateMachineBuild -S .

# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language (C or C++)
project(AsyncStateMachine VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Collect all .cpp and *.h source files in the current directory
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.h")

# Add subdirectories to include path
include_directories( 
    ${CMAKE_SOURCE_DIR}/Delegate
    ${CMAKE_SOURCE_DIR}/SelfTest
    ${CMAKE_SOURCE_DIR}/StateMachine
    ${CMAKE_SOURCE_DIR}/Port
)

# Add an executable target
add_executable(AsyncStateMachineApp ${SOURCES})

# Define the DELEGATE_UNIT_TEST macro for the AsyncStateMachineApp target
if (ENABLE_UNIT_TESTS)
    add_compile_definitions(DELEGATE_UNIT_TESTS)
endif()

# Add subdirectories to build
add_subdirectory(Delegate)
add_subdirectory(SelfTest)
add_subdirectory(StateMachine)
add_subdirectory(Port)

target_link_libraries(AsyncStateMachineApp PRIVATE 
    DelegateLib
    SelfTestLib
    StateMachineLib
    PortLib
)

