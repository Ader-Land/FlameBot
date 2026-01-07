cmake_minimum_required(VERSION 3.21)
project(TerminalChess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(TerminalChess
  src/main.cpp
  src/OpeningBook/OpeningBook.cpp
  src/Time/Time.cpp
  src/Chess/Chess.cpp
  src/FlameBoth/FlameBoth.cpp
  src/BoardHash/BoardHash.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS ">>> Target System: Windows (Static Build)")
    target_compile_options(TerminalChess PRIVATE -O3 -funroll-loops -Wall)
    target_link_options(TerminalChess PRIVATE -static -static-libgcc -static-libstdc++)

else()
    message(STATUS ">>> Target System: Linux/Unix (Native Optimization)")
    target_compile_options(TerminalChess PRIVATE -O3 -march=native -funroll-loops -Wall)
endif()