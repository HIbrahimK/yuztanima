# Kaynak dosyaları topla
set(CORE_SOURCES
    core/FaceDetector.h
    core/FaceDetector.cpp
    core/YuNetFaceDetector.h
    core/YuNetFaceDetector.cpp
    core/HaarFaceDetector.h
    core/HaarFaceDetector.cpp
    core/FaceDetectorFactory.h
    core/FaceDetectorFactory.cpp
    core/HidingStrategy.h
    core/GaussianBlurStrategy.h
    core/GaussianBlurStrategy.cpp
    core/PixelateStrategy.h
    core/PixelateStrategy.cpp
    core/BlackBarStrategy.h
    core/BlackBarStrategy.cpp
    core/EmojiOverlayStrategy.h
    core/EmojiOverlayStrategy.cpp
    core/FaceHider.h
    core/FaceHider.cpp
    core/BatchProcessor.h
    core/BatchProcessor.cpp
    core/ProcessSettings.h
    core/BatchResult.h
)

set(GUI_SOURCES
    gui/MainWindow.h
    gui/MainWindow.cpp
    gui/SettingsDialog.h
    gui/SettingsDialog.cpp
    gui/ImagePreviewWidget.h
    gui/ImagePreviewWidget.cpp
    gui/FileListWidget.h
    gui/FileListWidget.cpp
    gui/BatchResultDialog.h
    gui/BatchResultDialog.cpp
    gui/AboutDialog.h
    gui/AboutDialog.cpp
    gui/CustomSizeDialog.h
    gui/CustomSizeDialog.cpp
)

set(UTILS_SOURCES
    utils/ImageUtils.h
    utils/ImageUtils.cpp
    utils/SettingsManager.h
    utils/SettingsManager.cpp
)

# Çeviri dosyaları
set(TS_FILES
    translations/yuzgizle_tr.ts
    translations/yuzgizle_en.ts
)

# Kaynak dosyası
set(RESOURCE_FILES
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
)

# Ana çalıştırılabilir
add_executable(YuzGizle
    main.cpp
    ${CORE_SOURCES}
    ${GUI_SOURCES}
    ${UTILS_SOURCES}
    ${RESOURCE_FILES}
)

# Qt6 çeviri dosyalarını derle
qt6_add_translations(YuzGizle
    TS_FILES ${TS_FILES}
)

# Include dizinleri
target_include_directories(YuzGizle PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${OpenCV_INCLUDE_DIRS}
)

# Kütüphaneleri bağla
target_link_libraries(YuzGizle PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::Concurrent
    ${OpenCV_LIBS}
)

# spdlog varsa bağla
if(spdlog_FOUND)
    target_link_libraries(YuzGizle PRIVATE spdlog::spdlog)
    target_compile_definitions(YuzGizle PRIVATE HAS_SPDLOG)
endif()

# Platform-spesifik ayarlar
if(WIN32)
    # Windows'ta konsol penceresini gizle
    set_target_properties(YuzGizle PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
