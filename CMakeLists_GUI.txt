cmake_minimum_required(VERSION 3.21)
project(TerminalChess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SLINT_CPP_STATIC OFF CACHE BOOL "Force static Slint" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Use shared libs" FORCE)

include(FetchContent)
FetchContent_Declare(
  Slint
  GIT_REPOSITORY https://github.com/slint-ui/slint.git
  GIT_TAG release/1.8 
  SOURCE_SUBDIR api/cpp
)
FetchContent_MakeAvailable(Slint)

add_executable(TerminalChess
  src/gui.cpp
  src/ChessApp/ChessApp.cpp
  src/Time/Time.cpp
  src/Chess/Chess.cpp
  src/FlameBoth/FlameBoth.cpp
  src/BoardHash/BoardHash.cpp
)

slint_target_sources(TerminalChess ui/appGui.slint)

target_include_directories(TerminalChess PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(TerminalChess PRIVATE Slint::Slint)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS ">>> Target System: Windows")
    
    target_compile_options(TerminalChess PRIVATE 
        -O3 
        -funroll-loops 
        -Wall 
        -finput-charset=UTF-8 
        -fexec-charset=UTF-8
    )
    
else()
    message(STATUS ">>> Target System: Linux/Unix")
    target_compile_options(TerminalChess PRIVATE -O3 -march=native -funroll-loops -Wall)
endif()