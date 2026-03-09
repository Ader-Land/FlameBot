cmake_minimum_required(VERSION 3.21)
project(FlameBot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ──────────────────────────────────────────────
#  Ortak kaynak dosyaları (EXE ve DLL paylaşır)
# ──────────────────────────────────────────────
set(ENGINE_SOURCES
  src/Chess/Chess.cpp
  src/FlameBoth/FlameBoth.cpp
  src/BoardHash/BoardHash.cpp
  src/OpeningBook/OpeningBook.cpp
  src/Time/Time.cpp
)

# ──────────────────────────────────────────────
#  1) Standalone EXE (UCI modu)
# ──────────────────────────────────────────────
add_executable(FlameBot
  src/main.cpp
  ${ENGINE_SOURCES}
)
target_include_directories(FlameBot PRIVATE src)

# ──────────────────────────────────────────────
#  2) Shared Library / DLL (Unreal Engine modu)
# ──────────────────────────────────────────────
add_library(FlameBotDLL SHARED
  src/FlameBot_API.cpp
  ${ENGINE_SOURCES}
)
target_include_directories(FlameBotDLL PRIVATE src)
target_compile_definitions(FlameBotDLL PRIVATE FLAMEBOT_EXPORTS)

# DLL çıktı ismini "FlameBot" yap (FlameBotDLL.dll yerine FlameBot.dll)
set_target_properties(FlameBotDLL PROPERTIES OUTPUT_NAME "FlameBot")

# ──────────────────────────────────────────────
#  Derleyici ayarları (her iki hedef için)
# ──────────────────────────────────────────────
if(MSVC)
    message(STATUS ">>> Target System: Windows (MSVC)")
    foreach(tgt FlameBot FlameBotDLL)
        target_compile_options(${tgt} PRIVATE /O2 /W3 /utf-8)
    endforeach()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS ">>> Target System: Windows (MinGW)")
    foreach(tgt FlameBot FlameBotDLL)
        target_compile_options(${tgt} PRIVATE
            -O3 -funroll-loops -Wall
            -finput-charset=UTF-8 -fexec-charset=UTF-8
        )
    endforeach()
else()
    message(STATUS ">>> Target System: Linux/Unix")
    foreach(tgt FlameBot FlameBotDLL)
        target_compile_options(${tgt} PRIVATE -O3 -march=native -funroll-loops -Wall)
    endforeach()
endif()