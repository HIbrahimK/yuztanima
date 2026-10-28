cmake_minimum_required(VERSION 3.22)
project(YuzGizle VERSION 1.0.0 LANGUAGES CXX)

# C++17 zorunlu (OpenCV 5.0 gereksinimi)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt6 MOC/UIC/RCC otomasyonu
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Debug/Release çıktı dizinleri
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Qt6 modüllerini bul
find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Core
    Gui
    LinguistTools
    Concurrent
)

# OpenCV modüllerini bul
find_package(OpenCV REQUIRED COMPONENTS
    core
    imgproc
    imgcodecs
    objdetect
    dnn
)
find_package(OpenCV OPTIONAL_COMPONENTS xobjdetect)

# spdlog (opsiyonel loglama)
find_package(spdlog QUIET)

message(STATUS "Qt6 version: ${Qt6_VERSION}")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Alt dizinler
add_subdirectory(src)

# Testler (opsiyonel)
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
