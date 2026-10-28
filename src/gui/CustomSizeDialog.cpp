#include "CustomSizeDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDialogButtonBox>

CustomSizeDialog::CustomSizeDialog(int initialWidth,
                                   int initialHeight,
                                   bool initialMaintainAspect,
                                   QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("📐 Özel Boyut Belirle"));
    setMinimumWidth(380);
    setupUi();

    m_spnWidth->setValue(initialWidth > 0 ? initialWidth : 1200);
    m_spnHeight->setValue(initialHeight > 0 ? initialHeight : 568);
    m_chkMaintainAspect->setChecked(initialMaintainAspect);
}

void CustomSizeDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    // Boyut Giriş Grubu
    auto* grpDim = new QGroupBox(tr("Piksel Ölçüleri"), this);
    auto* grid = new QGridLayout(grpDim);
    grid->setSpacing(10);

    grid->addWidget(new QLabel(tr("Genişlik (px):"), grpDim), 0, 0);
    m_spnWidth = new QSpinBox(grpDim);
    m_spnWidth->setRange(50, 10000);
    m_spnWidth->setSingleStep(10);
    m_spnWidth->setSuffix(QStringLiteral(" px"));
    grid->addWidget(m_spnWidth, 0, 1);

    grid->addWidget(new QLabel(tr("Yükseklik (px):"), grpDim), 1, 0);
    m_spnHeight = new QSpinBox(grpDim);
    m_spnHeight->setRange(50, 10000);
    m_spnHeight->setSingleStep(10);
    m_spnHeight->setSuffix(QStringLiteral(" px"));
    grid->addWidget(m_spnHeight, 1, 1);

    m_chkMaintainAspect = new QCheckBox(tr("En-boy oranını koruyarak merkezden akıllı kırp"), grpDim);
    m_chkMaintainAspect->setChecked(true);
    grid->addWidget(m_chkMaintainAspect, 2, 0, 1, 2);

    mainLayout->addWidget(grpDim);

    // Hazır Şablon Butonları Grubu
    auto* grpPresets = new QGroupBox(tr("Hızlı Hazır Şablonlar"), this);
    auto* presetGrid = new QGridLayout(grpPresets);
    presetGrid->setSpacing(6);

    auto addPresetBtn = [this, presetGrid](const QString& title, int w, int h, int row, int col) {
        auto* btn = new QPushButton(title, this);
        btn->setStyleSheet(QStringLiteral(
            "QPushButton { background: rgba(128, 128, 128, 0.12); padding: 5px 8px; border-radius: 4px; font-size: 11px; }"
            "QPushButton:hover { background: rgba(33, 150, 243, 0.25); color: #2196f3; }"));
        connect(btn, &QPushButton::clicked, this, [this, w, h]() {
            applyPreset(w, h);
        });
        presetGrid->addWidget(btn, row, col);
    };

    addPresetBtn(tr("📰 1200 × 568 (MEB Manşet 19:9)"), 1200, 568, 0, 0);
    addPresetBtn(tr("🎓 133 × 171 (e-Okul <20KB)"), 133, 171, 0, 1);
    addPresetBtn(tr("🏛️ 394 × 512 (MEBBİS Vesikalık)"), 394, 512, 1, 0);
    addPresetBtn(tr("📷 1080 × 1080 (Instagram Kare)"), 1080, 1080, 1, 1);
    addPresetBtn(tr("📱 1080 × 1350 (Instagram Dikey)"), 1080, 1350, 2, 0);
    addPresetBtn(tr("🎬 1080 × 1920 (Instagram Reels)"), 1080, 1920, 2, 1);
    addPresetBtn(tr("🖥️ 1920 × 1080 (Full HD 16:9)"), 1920, 1080, 3, 0);
    addPresetBtn(tr("💻 1280 × 720 (HD 16:9)"), 1280, 720, 3, 1);

    mainLayout->addWidget(grpPresets);

    // Tamam / İptal Butonları
    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    btnBox->button(QDialogButtonBox::Ok)->setText(tr("Uygula"));
    btnBox->button(QDialogButtonBox::Cancel)->setText(tr("İptal"));

    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(btnBox);
}

void CustomSizeDialog::applyPreset(int w, int h)
{
    m_spnWidth->setValue(w);
    m_spnHeight->setValue(h);
}

int CustomSizeDialog::targetWidth() const
{
    return m_spnWidth ? m_spnWidth->value() : 1200;
}

int CustomSizeDialog::targetHeight() const
{
    return m_spnHeight ? m_spnHeight->value() : 568;
}

bool CustomSizeDialog::maintainAspect() const
{
    return m_chkMaintainAspect ? m_chkMaintainAspect->isChecked() : true;
}
