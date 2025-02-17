# Example CMake command line to create project build files:
#
# *** Windows ***
# cmake -G "Visual Studio 17 2022" -A Win32 -B Build -S .
#
# *** Linux ***
# cmake -G "Unix Makefiles" -B Build -S .

# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language (C or C++)
project(AsyncStateMachine VERSION 1.0 LANGUAGES CXX)

# Set build options
set (DMQ_ALLOCATOR "OFF")
set (DMQ_UTIL "ON")
set (DMQ_THREAD "DMQ_THREAD_STDLIB")
set (DMQ_SERIALIZE "DMQ_SERIALIZE_NONE")
set (DMQ_TRANSPORT "DMQ_TRANSPORT_NONE")
include("${CMAKE_SOURCE_DIR}/DelegateMQ/DelegateMQ.cmake")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Collect all .cpp and *.h source files in the current directory
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.h")

# Collect DelegateMQ predef source files
list(APPEND SOURCES ${DMQ_PREDEF_SOURCES})

# Organize delegate source files within IDE (Visual Studio)
source_group("Delegate Files" FILES ${DMQ_LIB_SOURCES})

# Add subdirectories to include path
include_directories( 
    SelfTest
    StateMachine
    Port
    ${DMQ_ROOT_DIR}
)

# Add an executable target
add_executable(AsyncStateMachineApp ${SOURCES} ${DMQ_LIB_SOURCES})

# Add subdirectories to build
add_subdirectory(SelfTest)
add_subdirectory(StateMachine)
add_subdirectory(Port)

target_link_libraries(AsyncStateMachineApp PRIVATE 
    SelfTestLib
    StateMachineLib
    PortLib
)

