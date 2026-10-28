find_package(Qt6 REQUIRED COMPONENTS Test)

set(TEST_SOURCES
    test_core.cpp
    ../src/core/FaceDetector.h
    ../src/core/FaceDetector.cpp
    ../src/core/YuNetFaceDetector.h
    ../src/core/YuNetFaceDetector.cpp
    ../src/core/HaarFaceDetector.h
    ../src/core/HaarFaceDetector.cpp
    ../src/core/FaceDetectorFactory.h
    ../src/core/FaceDetectorFactory.cpp
    ../src/core/HidingStrategy.h
    ../src/core/GaussianBlurStrategy.h
    ../src/core/GaussianBlurStrategy.cpp
    ../src/core/PixelateStrategy.h
    ../src/core/PixelateStrategy.cpp
    ../src/core/BlackBarStrategy.h
    ../src/core/BlackBarStrategy.cpp
    ../src/core/EmojiOverlayStrategy.h
    ../src/core/EmojiOverlayStrategy.cpp
    ../src/core/FaceHider.h
    ../src/core/FaceHider.cpp
    ../src/core/BatchProcessor.h
    ../src/core/BatchProcessor.cpp
    ../src/core/ProcessSettings.h
    ../src/core/BatchResult.h
    ../src/utils/ImageUtils.h
    ../src/utils/ImageUtils.cpp
)

add_executable(test_core ${TEST_SOURCES})

target_include_directories(test_core PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/utils
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(test_core PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Qt6::Concurrent
    ${OpenCV_LIBS}
)

add_test(NAME test_core COMMAND test_core)
