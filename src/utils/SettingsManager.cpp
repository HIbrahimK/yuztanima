#include "SettingsManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

SettingsManager& SettingsManager::instance()
{
    static SettingsManager s_instance;
    return s_instance;
}

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , m_settings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MEB"), QStringLiteral("YuzGizle"))
{
}

void SettingsManager::saveSettings(const ProcessSettings& settings)
{
    m_settings.beginGroup(QStringLiteral("Detection"));
    m_settings.setValue(QStringLiteral("detectorType"), static_cast<int>(settings.detectorType));
    m_settings.setValue(QStringLiteral("confidenceThreshold"), static_cast<double>(settings.confidenceThreshold));
    m_settings.setValue(QStringLiteral("minFaceSize"), settings.minFaceSize);
    m_settings.endGroup();

    m_settings.beginGroup(QStringLiteral("Hiding"));
    m_settings.setValue(QStringLiteral("hidingMethod"), static_cast<int>(settings.hidingMethod));
    m_settings.setValue(QStringLiteral("blurIntensity"), settings.blurIntensity);
    m_settings.setValue(QStringLiteral("pixelSize"), settings.pixelSize);
    m_settings.setValue(QStringLiteral("emojiPath"), settings.emojiPath);
    m_settings.endGroup();

    m_settings.beginGroup(QStringLiteral("Output"));
    m_settings.setValue(QStringLiteral("outputDirectory"), settings.outputDirectory);
    m_settings.setValue(QStringLiteral("outputFormat"), static_cast<int>(settings.outputFormat));
    m_settings.setValue(QStringLiteral("jpegQuality"), settings.jpegQuality);
    m_settings.setValue(QStringLiteral("filenameSuffix"), settings.filenameSuffix);
    m_settings.setValue(QStringLiteral("skipNoFace"), settings.skipNoFace);
    m_settings.setValue(QStringLiteral("maxThreads"), settings.maxThreads);
    m_settings.setValue(QStringLiteral("enableMaxFileSize"), settings.enableMaxFileSize);
    m_settings.setValue(QStringLiteral("maxFileSizeKb"), settings.maxFileSizeKb);
    m_settings.setValue(QStringLiteral("maxFileSizeAction"), static_cast<int>(settings.maxFileSizeAction));
    m_settings.endGroup();

    m_settings.beginGroup(QStringLiteral("MEB"));
    m_settings.setValue(QStringLiteral("resizeMode"), static_cast<int>(settings.resizeMode));
    m_settings.setValue(QStringLiteral("customWidth"), settings.customWidth);
    m_settings.setValue(QStringLiteral("customHeight"), settings.customHeight);
    m_settings.setValue(QStringLiteral("maintainAspectRatio"), settings.maintainAspectRatio);
    m_settings.endGroup();

    m_settings.beginGroup(QStringLiteral("Watermark"));
    m_settings.setValue(QStringLiteral("type"), static_cast<int>(settings.watermarkType));
    m_settings.setValue(QStringLiteral("imagePath"), settings.watermarkImagePath);
    m_settings.setValue(QStringLiteral("text"), settings.watermarkText);
    m_settings.setValue(QStringLiteral("position"), static_cast<int>(settings.watermarkPosition));
    m_settings.setValue(QStringLiteral("opacity"), settings.watermarkOpacity);
    m_settings.setValue(QStringLiteral("scalePercent"), settings.watermarkScalePercent);
    m_settings.setValue(QStringLiteral("textBackground"), settings.watermarkTextBackground);
    m_settings.endGroup();

    m_settings.sync();
    emit settingsChanged(settings);
}

ProcessSettings SettingsManager::loadSettings() const
{
    ProcessSettings settings;

    m_settings.beginGroup(QStringLiteral("Detection"));
    settings.detectorType = static_cast<DetectorType>(
        m_settings.value(QStringLiteral("detectorType"), static_cast<int>(DetectorType::YuNet)).toInt());
    settings.confidenceThreshold = static_cast<float>(
        m_settings.value(QStringLiteral("confidenceThreshold"), 0.7).toDouble());
    settings.minFaceSize = m_settings.value(QStringLiteral("minFaceSize"), 30).toInt();
    m_settings.endGroup();

    m_settings.beginGroup(QStringLiteral("Hiding"));
    settings.hidingMethod = static_cast<HidingMethod>(
        m_settings.value(QStringLiteral("hidingMethod"), static_cast<int>(HidingMethod::GaussianBlur)).toInt());
    settings.blurIntensity = m_settings.value(QStringLiteral("blurIntensity"), 51).toInt();
    settings.pixelSize = m_settings.value(QStringLiteral("pixelSize"), 10).toInt();
    settings.emojiPath = m_settings.value(QStringLiteral("emojiPath"), QString()).toString();
    m_settings.endGroup();

    m_settings.beginGroup(QStringLiteral("Output"));
    settings.outputDirectory = m_settings.value(QStringLiteral("outputDirectory"), QString()).toString();
    settings.outputFormat = static_cast<OutputFormat>(
        m_settings.value(QStringLiteral("outputFormat"), static_cast<int>(OutputFormat::JPEG)).toInt());
    settings.jpegQuality = m_settings.value(QStringLiteral("jpegQuality"), 90).toInt();
    settings.filenameSuffix = m_settings.value(QStringLiteral("filenameSuffix"), QStringLiteral("_gizli")).toString();
    settings.skipNoFace = m_settings.value(QStringLiteral("skipNoFace"), false).toBool();
    settings.maxThreads = m_settings.value(QStringLiteral("maxThreads"), 4).toInt();
    settings.enableMaxFileSize = m_settings.value(QStringLiteral("enableMaxFileSize"), false).toBool();
    settings.maxFileSizeKb = m_settings.value(QStringLiteral("maxFileSizeKb"), 1000).toInt();
    settings.maxFileSizeAction = static_cast<MaxFileSizeAction>(
        m_settings.value(QStringLiteral("maxFileSizeAction"), static_cast<int>(MaxFileSizeAction::CompressQualityOnly)).toInt());
    m_settings.endGroup();

    m_settings.beginGroup(QStringLiteral("MEB"));
    settings.resizeMode = static_cast<ResizeMode>(
        m_settings.value(QStringLiteral("resizeMode"), static_cast<int>(ResizeMode::Original)).toInt());
    settings.customWidth = m_settings.value(QStringLiteral("customWidth"), 1200).toInt();
    settings.customHeight = m_settings.value(QStringLiteral("customHeight"), 568).toInt();
    settings.maintainAspectRatio = m_settings.value(QStringLiteral("maintainAspectRatio"), true).toBool();
    m_settings.endGroup();

    m_settings.beginGroup(QStringLiteral("Watermark"));
    settings.watermarkType = static_cast<WatermarkType>(
        m_settings.value(QStringLiteral("type"), static_cast<int>(WatermarkType::None)).toInt());
    settings.watermarkImagePath = m_settings.value(QStringLiteral("imagePath"), QString()).toString();
    settings.watermarkText = m_settings.value(QStringLiteral("text"), QString()).toString();
    settings.watermarkPosition = static_cast<WatermarkPosition>(
        m_settings.value(QStringLiteral("position"), static_cast<int>(WatermarkPosition::BottomRight)).toInt());
    settings.watermarkOpacity = m_settings.value(QStringLiteral("opacity"), 85).toInt();
    settings.watermarkScalePercent = m_settings.value(QStringLiteral("scalePercent"), 15).toInt();
    settings.watermarkTextBackground = m_settings.value(QStringLiteral("textBackground"), true).toBool();
    m_settings.endGroup();

    // Eğer emojiPath boşsa varsayılan emoji yolunu belirle
    if (settings.emojiPath.isEmpty()) {
        settings.emojiPath = QStringLiteral(":/emojis/smile.png");
    }

    return settings;
}

QString SettingsManager::language() const
{
    return m_settings.value(QStringLiteral("General/language"), QStringLiteral("tr")).toString();
}

void SettingsManager::setLanguage(const QString& lang)
{
    m_settings.setValue(QStringLiteral("General/language"), lang);
    m_settings.sync();
    emit languageChanged(lang);
}

QString SettingsManager::lastDirectory() const
{
    return m_settings.value(QStringLiteral("General/lastDirectory"),
                            QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
}

void SettingsManager::setLastDirectory(const QString& dir)
{
    m_settings.setValue(QStringLiteral("General/lastDirectory"), dir);
    m_settings.sync();
}

QString SettingsManager::resolveModelsDirectory() const
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidatePaths = {
        appDir + QStringLiteral("/models"),
        appDir + QStringLiteral("/../resources/models"),
        appDir + QStringLiteral("/../../resources/models"),
        QDir::currentPath() + QStringLiteral("/resources/models"),
        QStringLiteral("/usr/share/yuzgizle/models"),
        QStringLiteral("/usr/local/share/yuzgizle/models")
    };

    for (const auto& path : candidatePaths) {
        QDir dir(path);
        if (dir.exists() && (dir.exists(QStringLiteral("face_detection_yunet_2023mar.onnx")) ||
                             dir.exists(QStringLiteral("haarcascade_frontalface_default.xml")))) {
            return dir.absolutePath();
        }
    }

    // Aday bulunamazsa kaynak ağacındaki varsayılan yolu dön
    return QDir::currentPath() + QStringLiteral("/resources/models");
}
