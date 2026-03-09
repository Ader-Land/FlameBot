cmake_minimum_required(VERSION 3.21)
project(FlameBot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(FlameBot
  src/main.cpp
  src/Chess/Chess.cpp
  src/FlameBoth/FlameBoth.cpp
  src/BoardHash/BoardHash.cpp
  src/OpeningBook/OpeningBook.cpp
  src/Time/Time.cpp
)

target_include_directories(FlameBot PRIVATE src)

if(MSVC)
    message(STATUS ">>> Target System: Windows (MSVC)")
    target_compile_options(FlameBot PRIVATE 
        /O2 
        /W3 
        /utf-8
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS ">>> Target System: Windows (MinGW)")
    target_compile_options(FlameBot PRIVATE 
        -O3 
        -funroll-loops 
        -Wall 
        -finput-charset=UTF-8 
        -fexec-charset=UTF-8
    )
else()
    message(STATUS ">>> Target System: Linux/Unix")
    target_compile_options(FlameBot PRIVATE -O3 -march=native -funroll-loops -Wall)
endif()