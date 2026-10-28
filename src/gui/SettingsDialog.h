#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include "ProcessSettings.h"
#include <QDialog>
#include <QTabWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(const ProcessSettings& currentSettings,
                            const QString& currentLang,
                            QWidget* parent = nullptr);
    ~SettingsDialog() override = default;

    /// Düzenlenen ayarları döndürür
    ProcessSettings settings() const;

    /// Seçilen dili döndürür ("tr", "en")
    QString language() const;

private slots:
    void onBrowseOutputDir();
    void onBrowseCustomEmoji();
    void onBrowseWatermarkImage();
    void onHidingMethodToggled();
    void onResizeModeToggled();
    void onWatermarkTypeChanged(int index);
    void onResetDefaults();
    void onEmojiButtonClicked();

private:
    void setupUi();
    void loadSettingsToUi(const ProcessSettings& s);

    // Sekmeler
    QTabWidget* m_tabs = nullptr;

    // Algılama kontrolleri
    QComboBox* m_cmbDetector = nullptr;
    QSlider* m_sldConfidence = nullptr;
    QDoubleSpinBox* m_spnConfidence = nullptr;
    QSpinBox* m_spnMinFaceSize = nullptr;

    // Gizleme kontrolleri
    QRadioButton* m_rbBlur = nullptr;
    QRadioButton* m_rbPixelate = nullptr;
    QRadioButton* m_rbBlackBar = nullptr;
    QRadioButton* m_rbEmoji = nullptr;
    QSlider* m_sldBlurIntensity = nullptr;
    QSpinBox* m_spnBlurIntensity = nullptr;
    QSlider* m_sldPixelSize = nullptr;
    QSpinBox* m_spnPixelSize = nullptr;
    QLineEdit* m_edtEmojiPath = nullptr;
    QLabel* m_lblEmojiPreview = nullptr;
    QPushButton* m_btnBrowseEmoji = nullptr;

    // MEB Boyutlandırma kontrolleri
    QRadioButton* m_rbSizeOriginal = nullptr;
    QRadioButton* m_rbSizeManset = nullptr;
    QRadioButton* m_rbSizeEOkul = nullptr;
    QRadioButton* m_rbSizeMebbis = nullptr;
    QRadioButton* m_rbSizeInstaSquare = nullptr;
    QRadioButton* m_rbSizeInstaPortrait = nullptr;
    QRadioButton* m_rbSizeInstaStory = nullptr;
    QRadioButton* m_rbSizeCustom = nullptr;
    QSpinBox* m_spnCustomWidth = nullptr;
    QSpinBox* m_spnCustomHeight = nullptr;
    QCheckBox* m_chkMaintainAspect = nullptr;

    // Okul Logosu ve Filigran kontrolleri
    QComboBox* m_cmbWatermarkType = nullptr;
    QLineEdit* m_edtWatermarkImage = nullptr;
    QPushButton* m_btnBrowseWatermarkImage = nullptr;
    QLabel* m_lblWatermarkImagePreview = nullptr;
    QPushButton* m_btnDefaultMebLogo = nullptr;
    QLineEdit* m_edtWatermarkText = nullptr;
    QComboBox* m_cmbWatermarkPosition = nullptr;
    QSlider* m_sldWatermarkOpacity = nullptr;
    QSpinBox* m_spnWatermarkOpacity = nullptr;
    QSlider* m_sldWatermarkScale = nullptr;
    QSpinBox* m_spnWatermarkScale = nullptr;
    QCheckBox* m_chkWatermarkTextBg = nullptr;

    // Çıktı kontrolleri
    QLineEdit* m_edtOutputDir = nullptr;
    QPushButton* m_btnBrowseOutput = nullptr;
    QComboBox* m_cmbOutputFormat = nullptr;
    QSlider* m_sldQuality = nullptr;
    QSpinBox* m_spnQuality = nullptr;
    QLineEdit* m_edtSuffix = nullptr;
    QCheckBox* m_chkSkipNoFace = nullptr;
    QSpinBox* m_spnMaxThreads = nullptr;

    // Dosya boyutu sınırı kontrolleri (KB / MB)
    QGroupBox* m_grpMaxFileSize = nullptr;
    QCheckBox* m_chkEnableMaxFileSize = nullptr;
    QSpinBox* m_spnMaxFileSizeKb = nullptr;
    QRadioButton* m_rbCompressQualityOnly = nullptr;
    QRadioButton* m_rbCompressAndResize = nullptr;

    // Genel / Dil
    QComboBox* m_cmbLanguage = nullptr;

    QString m_selectedEmojiPath;
};

#endif // SETTINGS_DIALOG_H
