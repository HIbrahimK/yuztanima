#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include "ProcessSettings.h"
#include <QObject>
#include <QSettings>
#include <QString>

/**
 * @brief Kullanıcı ayarlarını yöneten sınıf (QSettings)
 */
class SettingsManager : public QObject {
    Q_OBJECT

public:
    static SettingsManager& instance();

    /// Ayarları diske kaydeder
    void saveSettings(const ProcessSettings& settings);

    /// Ayarları diskten okur
    ProcessSettings loadSettings() const;

    /// Uygulama dilini döndürür ("tr", "en")
    QString language() const;

    /// Uygulama dilini kaydeder
    void setLanguage(const QString& lang);

    /// Son açılan dosya/klasör yolunu döndürür
    QString lastDirectory() const;

    /// Son açılan dosya/klasör yolunu günceller
    void setLastDirectory(const QString& dir);

    /// Model dizinini tespit eder
    QString resolveModelsDirectory() const;

signals:
    void settingsChanged(const ProcessSettings& newSettings);
    void languageChanged(const QString& newLanguage);

private:
    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager() override = default;

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    mutable QSettings m_settings;
};

#endif // SETTINGS_MANAGER_H
