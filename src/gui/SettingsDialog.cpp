#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

SettingsDialog::SettingsDialog(const ProcessSettings& currentSettings,
                               const QString& currentLang,
                               QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Ayarlar"));
    setMinimumSize(600, 520);
    setupUi();
    loadSettingsToUi(currentSettings);

    if (currentLang == QStringLiteral("en")) {
        m_cmbLanguage->setCurrentIndex(1);
    } else {
        m_cmbLanguage->setCurrentIndex(0);
    }
}

void SettingsDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);

    // ==========================================
    // 1. Sekme: Yüz Algılama (Detection)
    // ==========================================
    auto* tabDetection = new QWidget(this);
    auto* detLayout = new QVBoxLayout(tabDetection);

    auto* grpDetector = new QGroupBox(tr("Algılama Motoru"), tabDetection);
    auto* grpDetLayout = new QGridLayout(grpDetector);

    grpDetLayout->addWidget(new QLabel(tr("Yöntem:"), grpDetector), 0, 0);
    m_cmbDetector = new QComboBox(grpDetector);
    m_cmbDetector->addItem(tr("YuNet (Derin Öğrenme / DNN) - Yüksek Doğruluk [Önerilen]"), static_cast<int>(DetectorType::YuNet));
    m_cmbDetector->addItem(tr("Haar Cascade - Klasik / Hızlı"), static_cast<int>(DetectorType::HaarCascade));
    grpDetLayout->addWidget(m_cmbDetector, 0, 1);

    grpDetLayout->addWidget(new QLabel(tr("Güven Eşiği:"), grpDetector), 1, 0);
    auto* confLayout = new QHBoxLayout();
    m_sldConfidence = new QSlider(Qt::Horizontal, grpDetector);
    m_sldConfidence->setRange(10, 95);
    m_sldConfidence->setValue(70);
    m_spnConfidence = new QDoubleSpinBox(grpDetector);
    m_spnConfidence->setRange(0.10, 0.95);
    m_spnConfidence->setSingleStep(0.05);
    m_spnConfidence->setValue(0.70);
    confLayout->addWidget(m_sldConfidence, 1);
    confLayout->addWidget(m_spnConfidence);
    grpDetLayout->addLayout(confLayout, 1, 1);

    grpDetLayout->addWidget(new QLabel(tr("Min. Yüz Boyutu:"), grpDetector), 2, 0);
    m_spnMinFaceSize = new QSpinBox(grpDetector);
    m_spnMinFaceSize->setRange(10, 300);
    m_spnMinFaceSize->setSuffix(tr(" px"));
    m_spnMinFaceSize->setValue(30);
    grpDetLayout->addWidget(m_spnMinFaceSize, 2, 1);

    detLayout->addWidget(grpDetector);

    auto* lblDetHelp = new QLabel(
        tr("<b>İpucu:</b> YuNet modeli profil ve açılı yüzleri de yüksek doğrulukla tespit eder.<br>"
           "Eğer fotoğraflarda bazı yüzler kaçıyorsa güven eşiğini düşürebilirsiniz (örn. 0.50)."), tabDetection);
    lblDetHelp->setWordWrap(true);
    lblDetHelp->setStyleSheet(QStringLiteral(
        "background: rgba(128, 128, 128, 0.12); "
        "border: 1px solid rgba(128, 128, 128, 0.25); "
        "padding: 8px; border-radius: 4px; font-size: 11px;"));
    detLayout->addWidget(lblDetHelp);
    detLayout->addStretch();
    m_tabs->addTab(tabDetection, tr("Yüz Algılama"));

    // ==========================================
    // 2. Sekme: Gizleme Yöntemi (Hiding)
    // ==========================================
    auto* tabHiding = new QWidget(this);
    auto* hidLayout = new QVBoxLayout(tabHiding);

    auto* grpHiding = new QGroupBox(tr("Gizleme Yöntemi"), tabHiding);
    auto* grpHidLayout = new QVBoxLayout(grpHiding);

    m_rbBlur = new QRadioButton(tr("Gaussian Bulanıklaştırma (Blur)"), grpHiding);
    auto* blurParams = new QHBoxLayout();
    blurParams->setContentsMargins(20, 0, 0, 0);
    blurParams->addWidget(new QLabel(tr("Yoğunluk:"), grpHiding));
    m_sldBlurIntensity = new QSlider(Qt::Horizontal, grpHiding);
    m_sldBlurIntensity->setRange(15, 99);
    m_sldBlurIntensity->setSingleStep(2);
    m_sldBlurIntensity->setValue(51);
    m_spnBlurIntensity = new QSpinBox(grpHiding);
    m_spnBlurIntensity->setRange(15, 99);
    m_spnBlurIntensity->setSingleStep(2);
    m_spnBlurIntensity->setValue(51);
    blurParams->addWidget(m_sldBlurIntensity, 1);
    blurParams->addWidget(m_spnBlurIntensity);

    m_rbPixelate = new QRadioButton(tr("Pikselleştirme (Mozaik)"), grpHiding);
    auto* pixParams = new QHBoxLayout();
    pixParams->setContentsMargins(20, 0, 0, 0);
    pixParams->addWidget(new QLabel(tr("Piksel Boyutu:"), grpHiding));
    m_sldPixelSize = new QSlider(Qt::Horizontal, grpHiding);
    m_sldPixelSize->setRange(4, 40);
    m_sldPixelSize->setValue(10);
    m_spnPixelSize = new QSpinBox(grpHiding);
    m_spnPixelSize->setRange(4, 40);
    m_spnPixelSize->setValue(10);
    pixParams->addWidget(m_sldPixelSize, 1);
    pixParams->addWidget(m_spnPixelSize);

    m_rbBlackBar = new QRadioButton(tr("Siyah Bant (Göz Hizası)"), grpHiding);

    m_rbEmoji = new QRadioButton(tr("Emoji / Smiley Kaplama"), grpHiding);
    auto* emojiParams = new QVBoxLayout();
    emojiParams->setContentsMargins(20, 0, 0, 0);

    // Hazır emoji galerisi (20 MEB Uyumlu Emoji Koleksiyonu)
    auto* emojiGalleryLayout = new QGridLayout();
    emojiGalleryLayout->setSpacing(6);
    const struct EmojiInfo {
        const char* name;
        const char* title;
    } emojiList[] = {
        {"smile", "Gülen Yüz"},
        {"cool", "Havalı Gözlüklü"},
        {"slight_smile", "Tebessüm"},
        {"grin", "Geniş Gülümseme"},
        {"star", "Yıldız Gözlü"},
        {"mask", "Maskeli / Hijyen"},
        {"party", "Kutlama / Parti"},
        {"sunglasses", "Güneş Gözlüğü"},
        {"angel", "Melek"},
        {"heart", "Kalp Gözlü"},
        {"cat", "Sevimli Kedi"},
        {"dog", "Sevimli Köpek"},
        {"panda", "Panda"},
        {"bear", "Ayıcık"},
        {"lion", "Cesur Aslan"},
        {"fox", "Sevimli Tilki"},
        {"robot", "Robot / Teknoloji"},
        {"rocket", "Roket / Bilim"},
        {"flower", "Çiçek / Doğa"},
        {"grad", "Mezuniyet Kepi"}
    };

    int col = 0;
    int row = 0;
    for (const auto& item : emojiList) {
        auto* btn = new QPushButton(grpHiding);
        btn->setFixedSize(40, 40);
        btn->setIcon(QIcon(QStringLiteral(":/emojis/%1.png").arg(QString::fromLatin1(item.name))));
        btn->setIconSize(QSize(28, 28));
        btn->setToolTip(tr(item.title));
        btn->setProperty("emojiName", QString::fromLatin1(item.name));
        connect(btn, &QPushButton::clicked, this, &SettingsDialog::onEmojiButtonClicked);
        emojiGalleryLayout->addWidget(btn, row, col);
        col++;
        if (col >= 10) {
            col = 0;
            row++;
        }
    }
    emojiParams->addLayout(emojiGalleryLayout);

    // Özel emoji seçici
    auto* customEmojiLayout = new QHBoxLayout();
    m_edtEmojiPath = new QLineEdit(grpHiding);
    m_edtEmojiPath->setPlaceholderText(tr("Özel emoji dosyası (PNG/SVG)..."));
    m_btnBrowseEmoji = new QPushButton(tr("Özel Seç..."), grpHiding);
    m_lblEmojiPreview = new QLabel(grpHiding);
    m_lblEmojiPreview->setFixedSize(32, 32);
    m_lblEmojiPreview->setScaledContents(true);
    customEmojiLayout->addWidget(m_edtEmojiPath, 1);
    customEmojiLayout->addWidget(m_btnBrowseEmoji);
    customEmojiLayout->addWidget(m_lblEmojiPreview);
    emojiParams->addLayout(customEmojiLayout);

    grpHidLayout->addWidget(m_rbBlur);
    grpHidLayout->addLayout(blurParams);
    grpHidLayout->addWidget(m_rbPixelate);
    grpHidLayout->addLayout(pixParams);
    grpHidLayout->addWidget(m_rbBlackBar);
    grpHidLayout->addWidget(m_rbEmoji);
    grpHidLayout->addLayout(emojiParams);

    hidLayout->addWidget(grpHiding);
    hidLayout->addStretch();
    m_tabs->addTab(tabHiding, tr("Gizleme Yöntemi"));

    // ==========================================
    // 3. Sekme: MEB Standartları ve Boyutlandırma
    // ==========================================
    auto* tabResize = new QWidget(this);
    auto* resLayout = new QVBoxLayout(tabResize);

    auto* grpMeb = new QGroupBox(tr("MEB Şablonları ve Sosyal Medya Boyutlandırma"), tabResize);
    auto* grpMebLayout = new QVBoxLayout(grpMeb);

    m_rbSizeOriginal = new QRadioButton(tr("Orijinal Boyutu Koru (Değişiklik yapma)"), grpMeb);
    m_rbSizeManset = new QRadioButton(tr("📰 MEB Web Sitesi Manşet Haberi (1200 × 568 px - 19:9 En-Boy Oranı) [Standart]"), grpMeb);
    m_rbSizeEOkul = new QRadioButton(tr("🎓 MEB e-Okul Standardı (35x45 mm / 133 × 171 px, < 20 KB JPEG)"), grpMeb);
    m_rbSizeMebbis = new QRadioButton(tr("🏛️ MEBBİS Vesikalık Standardı (394 × 512 px)"), grpMeb);
    m_rbSizeInstaSquare = new QRadioButton(tr("📷 Instagram Kare Gönderi (1080 × 1080 px - 1:1)"), grpMeb);
    m_rbSizeInstaPortrait = new QRadioButton(tr("📱 Instagram Dikey / Portre (1080 × 1350 px - 4:5)"), grpMeb);
    m_rbSizeInstaStory = new QRadioButton(tr("🎬 Instagram Hikaye / Reels (1080 × 1920 px - 9:16)"), grpMeb);
    m_rbSizeCustom = new QRadioButton(tr("📐 Özel Boyut Belirle"), grpMeb);

    auto* customDimLayout = new QHBoxLayout();
    customDimLayout->setContentsMargins(25, 0, 0, 0);
    customDimLayout->addWidget(new QLabel(tr("Genişlik:"), grpMeb));
    m_spnCustomWidth = new QSpinBox(grpMeb);
    m_spnCustomWidth->setRange(50, 8000);
    m_spnCustomWidth->setValue(1200);
    m_spnCustomWidth->setSuffix(tr(" px"));
    customDimLayout->addWidget(m_spnCustomWidth);

    customDimLayout->addWidget(new QLabel(tr("Yükseklik:"), grpMeb));
    m_spnCustomHeight = new QSpinBox(grpMeb);
    m_spnCustomHeight->setRange(50, 8000);
    m_spnCustomHeight->setValue(568);
    m_spnCustomHeight->setSuffix(tr(" px"));
    customDimLayout->addWidget(m_spnCustomHeight);
    customDimLayout->addStretch();

    m_chkMaintainAspect = new QCheckBox(tr("En-boy oranını koruyarak merkezden akıllı kırp"), grpMeb);
    m_chkMaintainAspect->setChecked(true);
    m_chkMaintainAspect->setStyleSheet(QStringLiteral("margin-left: 25px;"));

    grpMebLayout->addWidget(m_rbSizeOriginal);
    grpMebLayout->addWidget(m_rbSizeManset);
    grpMebLayout->addWidget(m_rbSizeEOkul);
    grpMebLayout->addWidget(m_rbSizeMebbis);
    grpMebLayout->addWidget(m_rbSizeInstaSquare);
    grpMebLayout->addWidget(m_rbSizeInstaPortrait);
    grpMebLayout->addWidget(m_rbSizeInstaStory);
    grpMebLayout->addWidget(m_rbSizeCustom);
    grpMebLayout->addLayout(customDimLayout);
    grpMebLayout->addWidget(m_chkMaintainAspect);

    resLayout->addWidget(grpMeb);

    auto* lblMebInfo = new QLabel(
        tr("<b>Standart ve Ölçü Notları:</b><br>"
           "• <b>MEB Manşet:</b> Okul web sitelerinde (*.meb.k12.tr) manşet alanı en az 1200 px genişlik ve 19:9 en-boy oranı ister (1200×568 px).<br>"
           "• <b>e-Okul:</b> MEB şartnamesine göre 35×45mm (133×171 px), JPEG ve 20 KB'dan küçük (&lt; 20 KB) olarak kaydedilir.<br>"
           "• <b>MEBBİS:</b> Yüksek kaliteli 394×512 piksel personel/öğrenci fotoğrafı standardı.<br>"
           "• <b>Instagram:</b> Sosyal medya paylaşımları için tam uyumlu kare, dikey ve Reels ölçüleri."), tabResize);
    lblMebInfo->setWordWrap(true);
    lblMebInfo->setStyleSheet(QStringLiteral(
        "background: rgba(128, 128, 128, 0.12); "
        "border: 1px solid rgba(128, 128, 128, 0.25); "
        "padding: 8px; border-radius: 4px; font-size: 11px;"));
    resLayout->addWidget(lblMebInfo);
    resLayout->addStretch();

    m_tabs->addTab(tabResize, tr("MEB / Boyutlandırma"));

    // ==========================================
    // 4. Sekme: Okul Logosu & Filigran (Watermark)
    // ==========================================
    auto* tabWatermark = new QWidget(this);
    auto* wmLayout = new QVBoxLayout(tabWatermark);

    auto* grpWm = new QGroupBox(tr("Okul Logosu ve Filigran Ayarları"), tabWatermark);
    auto* grpWmLayout = new QGridLayout(grpWm);
    grpWmLayout->setSpacing(10);

    grpWmLayout->addWidget(new QLabel(tr("Filigran Türü:"), grpWm), 0, 0);
    m_cmbWatermarkType = new QComboBox(grpWm);
    m_cmbWatermarkType->addItem(tr("❌ Filigran Yok"), static_cast<int>(WatermarkType::None));
    m_cmbWatermarkType->addItem(tr("🏫 Yalnızca Okul Logosu (Görsel)"), static_cast<int>(WatermarkType::Image));
    m_cmbWatermarkType->addItem(tr("✍️ Yalnızca Metin Filigran"), static_cast<int>(WatermarkType::Text));
    m_cmbWatermarkType->addItem(tr("✨ Okul Logosu ve Metin Birlikte"), static_cast<int>(WatermarkType::Both));
    grpWmLayout->addWidget(m_cmbWatermarkType, 0, 1);

    // Okul Logosu Görsel Seçimi
    grpWmLayout->addWidget(new QLabel(tr("Okul Logosu (PNG):"), grpWm), 1, 0);
    auto* imgPathLayout = new QHBoxLayout();
    m_edtWatermarkImage = new QLineEdit(grpWm);
    m_edtWatermarkImage->setPlaceholderText(tr("Şeffaf arka planlı okul logosu (PNG/SVG)..."));
    m_btnBrowseWatermarkImage = new QPushButton(tr("Gözat..."), grpWm);
    m_btnDefaultMebLogo = new QPushButton(tr("Uygulama Simgesi"), grpWm);
    m_btnDefaultMebLogo->setToolTip(tr("Uygulama simgesini logo olarak kullan"));
    m_lblWatermarkImagePreview = new QLabel(grpWm);
    m_lblWatermarkImagePreview->setFixedSize(36, 36);
    m_lblWatermarkImagePreview->setScaledContents(true);

    imgPathLayout->addWidget(m_edtWatermarkImage, 1);
    imgPathLayout->addWidget(m_btnBrowseWatermarkImage);
    imgPathLayout->addWidget(m_btnDefaultMebLogo);
    imgPathLayout->addWidget(m_lblWatermarkImagePreview);
    grpWmLayout->addLayout(imgPathLayout, 1, 1);

    // Filigran Metni
    grpWmLayout->addWidget(new QLabel(tr("Filigran Metni:"), grpWm), 2, 0);
    auto* textInputLayout = new QVBoxLayout();
    m_edtWatermarkText = new QLineEdit(grpWm);
    m_edtWatermarkText->setPlaceholderText(tr("Örn: T.C. MİLLÎ EĞİTİM BAKANLIĞI ... İLKOKULU"));
    textInputLayout->addWidget(m_edtWatermarkText);

    auto* quickTextLayout = new QHBoxLayout();
    auto* btnQuickMeb = new QPushButton(tr("T.C. Millî Eğitim Bakanlığı"), grpWm);
    btnQuickMeb->setStyleSheet(QStringLiteral("font-size: 11px; padding: 2px 6px;"));
    connect(btnQuickMeb, &QPushButton::clicked, this, [this]() {
        m_edtWatermarkText->setText(tr("T.C. Millî Eğitim Bakanlığı"));
    });
    quickTextLayout->addWidget(btnQuickMeb);
    quickTextLayout->addStretch();
    textInputLayout->addLayout(quickTextLayout);
    grpWmLayout->addLayout(textInputLayout, 2, 1);

    // Konum Seçimi
    grpWmLayout->addWidget(new QLabel(tr("Konum:"), grpWm), 3, 0);
    m_cmbWatermarkPosition = new QComboBox(grpWm);
    m_cmbWatermarkPosition->addItem(tr("Sağ Alt (Standart Kurumsal)"), static_cast<int>(WatermarkPosition::BottomRight));
    m_cmbWatermarkPosition->addItem(tr("Sol Alt"), static_cast<int>(WatermarkPosition::BottomLeft));
    m_cmbWatermarkPosition->addItem(tr("Sağ Üst"), static_cast<int>(WatermarkPosition::TopRight));
    m_cmbWatermarkPosition->addItem(tr("Sol Üst"), static_cast<int>(WatermarkPosition::TopLeft));
    m_cmbWatermarkPosition->addItem(tr("Merkez"), static_cast<int>(WatermarkPosition::Center));
    m_cmbWatermarkPosition->addItem(tr("🎯 Tuval Üzerinde Serbest Konum"), static_cast<int>(WatermarkPosition::Custom));
    grpWmLayout->addWidget(m_cmbWatermarkPosition, 3, 1);

    // Opaklık (Saydamlık)
    grpWmLayout->addWidget(new QLabel(tr("Opaklık (%):"), grpWm), 4, 0);
    auto* opacLayout = new QHBoxLayout();
    m_sldWatermarkOpacity = new QSlider(Qt::Horizontal, grpWm);
    m_sldWatermarkOpacity->setRange(10, 100);
    m_sldWatermarkOpacity->setValue(85);
    m_spnWatermarkOpacity = new QSpinBox(grpWm);
    m_spnWatermarkOpacity->setRange(10, 100);
    m_spnWatermarkOpacity->setValue(85);
    m_spnWatermarkOpacity->setSuffix(QStringLiteral(" %"));
    opacLayout->addWidget(m_sldWatermarkOpacity, 1);
    opacLayout->addWidget(m_spnWatermarkOpacity);
    grpWmLayout->addLayout(opacLayout, 4, 1);

    // Boyut Ölçeği (% of image width)
    grpWmLayout->addWidget(new QLabel(tr("Logo Boyutu (%):"), grpWm), 5, 0);
    auto* scaleLayout = new QHBoxLayout();
    m_sldWatermarkScale = new QSlider(Qt::Horizontal, grpWm);
    m_sldWatermarkScale->setRange(5, 40);
    m_sldWatermarkScale->setValue(15);
    m_spnWatermarkScale = new QSpinBox(grpWm);
    m_spnWatermarkScale->setRange(5, 40);
    m_spnWatermarkScale->setValue(15);
    m_spnWatermarkScale->setSuffix(QStringLiteral(" %"));
    scaleLayout->addWidget(m_sldWatermarkScale, 1);
    scaleLayout->addWidget(m_spnWatermarkScale);
    grpWmLayout->addLayout(scaleLayout, 5, 1);

    // Arka plan kontrast bandı
    m_chkWatermarkTextBg = new QCheckBox(tr("Her renk arka planda okunabilirlik için şık yarı saydam kontrast bandı ekle"), grpWm);
    m_chkWatermarkTextBg->setChecked(true);
    grpWmLayout->addWidget(m_chkWatermarkTextBg, 6, 0, 1, 2);

    wmLayout->addWidget(grpWm);

    auto* lblWmInfo = new QLabel(
        tr("<b>Toplu Okul Logosu & Filigran Ekleme:</b><br>"
           "Burada tanımladığınız okul logosu veya kurumsal metin filigranı, hem tekil kayıtlarda hem de toplu işlemde "
           "tüm fotoğrafların üzerine seçtiğiniz konum ve saydamlıkta otomatik olarak eklenir."), tabWatermark);
    lblWmInfo->setWordWrap(true);
    lblWmInfo->setStyleSheet(QStringLiteral(
        "background: rgba(128, 128, 128, 0.12); "
        "border: 1px solid rgba(128, 128, 128, 0.25); "
        "padding: 8px; border-radius: 4px; font-size: 11px;"));
    wmLayout->addWidget(lblWmInfo);
    wmLayout->addStretch();

    m_tabs->addTab(tabWatermark, tr("🏫 Okul Logosu & Filigran"));

    // ==========================================
    // 5. Sekme: Çıktı & Kayıt (Output)
    // ==========================================
    auto* tabOutput = new QWidget(this);
    auto* outLayout = new QVBoxLayout(tabOutput);

    auto* grpOutput = new QGroupBox(tr("Kayıt Ayarları"), tabOutput);
    auto* grpOutLayout = new QGridLayout(grpOutput);

    grpOutLayout->addWidget(new QLabel(tr("Çıktı Klasörü:"), grpOutput), 0, 0);
    auto* outDirLayout = new QHBoxLayout();
    m_edtOutputDir = new QLineEdit(grpOutput);
    m_edtOutputDir->setPlaceholderText(tr("Boş bırakılırsa kaynak klasörün yanında 'output' oluşturulur"));
    m_btnBrowseOutput = new QPushButton(tr("Gözat..."), grpOutput);
    outDirLayout->addWidget(m_edtOutputDir, 1);
    outDirLayout->addWidget(m_btnBrowseOutput);
    grpOutLayout->addLayout(outDirLayout, 0, 1);

    grpOutLayout->addWidget(new QLabel(tr("Çıktı Formatı:"), grpOutput), 1, 0);
    m_cmbOutputFormat = new QComboBox(grpOutput);
    m_cmbOutputFormat->addItem(tr("Kaynak Dosya ile Aynı Format"), static_cast<int>(OutputFormat::SameAsSource));
    m_cmbOutputFormat->addItem(QStringLiteral("JPEG (.jpg)"), static_cast<int>(OutputFormat::JPEG));
    m_cmbOutputFormat->addItem(QStringLiteral("PNG (.png)"), static_cast<int>(OutputFormat::PNG));
    m_cmbOutputFormat->addItem(QStringLiteral("WebP (.webp)"), static_cast<int>(OutputFormat::WEBP));
    m_cmbOutputFormat->addItem(QStringLiteral("BMP (.bmp)"), static_cast<int>(OutputFormat::BMP));
    grpOutLayout->addWidget(m_cmbOutputFormat, 1, 1);

    grpOutLayout->addWidget(new QLabel(tr("Görüntü Kalitesi:"), grpOutput), 2, 0);
    auto* qLayout = new QHBoxLayout();
    m_sldQuality = new QSlider(Qt::Horizontal, grpOutput);
    m_sldQuality->setRange(30, 100);
    m_sldQuality->setValue(90);
    m_spnQuality = new QSpinBox(grpOutput);
    m_spnQuality->setRange(30, 100);
    m_spnQuality->setValue(90);
    qLayout->addWidget(m_sldQuality, 1);
    qLayout->addWidget(m_spnQuality);
    grpOutLayout->addLayout(qLayout, 2, 1);

    grpOutLayout->addWidget(new QLabel(tr("Dosya Eki:"), grpOutput), 3, 0);
    m_edtSuffix = new QLineEdit(grpOutput);
    m_edtSuffix->setText(QStringLiteral("_gizli"));
    grpOutLayout->addWidget(m_edtSuffix, 3, 1);

    grpOutLayout->addWidget(new QLabel(tr("İş Parçacığı:"), grpOutput), 4, 0);
    m_spnMaxThreads = new QSpinBox(grpOutput);
    m_spnMaxThreads->setRange(1, std::max(2, QThread::idealThreadCount()));
    m_spnMaxThreads->setValue(std::min(4, std::max(1, QThread::idealThreadCount())));
    grpOutLayout->addWidget(m_spnMaxThreads, 4, 1);

    outLayout->addWidget(grpOutput);

    // Dosya Boyutu Sınırlayıcı Grubu
    m_grpMaxFileSize = new QGroupBox(tr("📦 Maksimum Dosya Boyutu Sınırlayıcı (KB / MB)"), tabOutput);
    auto* maxFileLayout = new QVBoxLayout(m_grpMaxFileSize);

    m_chkEnableMaxFileSize = new QCheckBox(tr("Maksimum dosya boyutunu sınırla"), m_grpMaxFileSize);
    maxFileLayout->addWidget(m_chkEnableMaxFileSize);

    auto* sizeInputLayout = new QHBoxLayout();
    sizeInputLayout->addWidget(new QLabel(tr("Azami Boyut:"), m_grpMaxFileSize));
    m_spnMaxFileSizeKb = new QSpinBox(m_grpMaxFileSize);
    m_spnMaxFileSizeKb->setRange(20, 50000);
    m_spnMaxFileSizeKb->setSingleStep(50);
    m_spnMaxFileSizeKb->setValue(1000);
    m_spnMaxFileSizeKb->setSuffix(QStringLiteral(" KB"));
    sizeInputLayout->addWidget(m_spnMaxFileSizeKb);

    auto* btn500k = new QPushButton(QStringLiteral("500 KB"), m_grpMaxFileSize);
    auto* btn1m = new QPushButton(QStringLiteral("1 MB"), m_grpMaxFileSize);
    auto* btn2m = new QPushButton(QStringLiteral("2 MB"), m_grpMaxFileSize);
    connect(btn500k, &QPushButton::clicked, this, [this]() { m_spnMaxFileSizeKb->setValue(500); });
    connect(btn1m, &QPushButton::clicked, this, [this]() { m_spnMaxFileSizeKb->setValue(1024); });
    connect(btn2m, &QPushButton::clicked, this, [this]() { m_spnMaxFileSizeKb->setValue(2048); });
    sizeInputLayout->addWidget(btn500k);
    sizeInputLayout->addWidget(btn1m);
    sizeInputLayout->addWidget(btn2m);
    sizeInputLayout->addStretch();
    maxFileLayout->addLayout(sizeInputLayout);

    m_rbCompressQualityOnly = new QRadioButton(
        tr("Çözünürlüğü (pikseli) değiştirmeden sadece JPEG kalitesini düşür"), m_grpMaxFileSize);
    m_rbCompressAndResize = new QRadioButton(
        tr("Gerekirse çözünürlüğü de kademeli küçülterek hedef boyuta sığdır"), m_grpMaxFileSize);
    m_rbCompressQualityOnly->setChecked(true);

    maxFileLayout->addWidget(m_rbCompressQualityOnly);
    maxFileLayout->addWidget(m_rbCompressAndResize);

    connect(m_chkEnableMaxFileSize, &QCheckBox::toggled, this, [this](bool checked) {
        m_spnMaxFileSizeKb->setEnabled(checked);
        m_rbCompressQualityOnly->setEnabled(checked);
        m_rbCompressAndResize->setEnabled(checked);
    });

    outLayout->addWidget(m_grpMaxFileSize);

    m_chkSkipNoFace = new QCheckBox(tr("Yüz bulunamayan fotoğrafları çıktı klasörüne kopyalama / atla"), tabOutput);
    outLayout->addWidget(m_chkSkipNoFace);
    outLayout->addStretch();
    m_tabs->addTab(tabOutput, tr("Çıktı & Kayıt"));

    // ==========================================
    // 5. Sekme: Genel / Dil
    // ==========================================
    auto* tabGeneral = new QWidget(this);
    auto* genLayout = new QVBoxLayout(tabGeneral);

    auto* grpGen = new QGroupBox(tr("Uygulama Tercihleri"), tabGeneral);
    auto* grpGenLayout = new QGridLayout(grpGen);

    grpGenLayout->addWidget(new QLabel(tr("Dil / Language:"), grpGen), 0, 0);
    m_cmbLanguage = new QComboBox(grpGen);
    m_cmbLanguage->addItem(QStringLiteral("Türkçe"), QStringLiteral("tr"));
    m_cmbLanguage->addItem(QStringLiteral("English"), QStringLiteral("en"));
    grpGenLayout->addWidget(m_cmbLanguage, 0, 1);

    genLayout->addWidget(grpGen);
    genLayout->addStretch();
    m_tabs->addTab(tabGeneral, tr("Genel"));

    mainLayout->addWidget(m_tabs);

    // Alt buton kutusu
    auto* bottomLayout = new QHBoxLayout();
    auto* btnReset = new QPushButton(tr("Varsayılana Sıfırla"), this);
    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    btnBox->button(QDialogButtonBox::Ok)->setText(tr("Kaydet"));
    btnBox->button(QDialogButtonBox::Cancel)->setText(tr("İptal"));

    bottomLayout->addWidget(btnReset);
    bottomLayout->addStretch();
    bottomLayout->addWidget(btnBox);
    mainLayout->addLayout(bottomLayout);

    // Sinyal bağlantıları
    connect(m_sldConfidence, &QSlider::valueChanged, this, [this](int val) {
        m_spnConfidence->setValue(val / 100.0);
    });
    connect(m_spnConfidence, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
        m_sldConfidence->setValue(static_cast<int>(val * 100));
    });

    connect(m_sldBlurIntensity, &QSlider::valueChanged, this, [this](int val) {
        if (val % 2 == 0) val += 1;
        m_spnBlurIntensity->setValue(val);
    });
    connect(m_spnBlurIntensity, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val) {
        if (val % 2 == 0) val += 1;
        m_sldBlurIntensity->setValue(val);
    });

    connect(m_sldPixelSize, &QSlider::valueChanged, m_spnPixelSize, &QSpinBox::setValue);
    connect(m_spnPixelSize, QOverload<int>::of(&QSpinBox::valueChanged), m_sldPixelSize, &QSlider::setValue);

    connect(m_sldQuality, &QSlider::valueChanged, m_spnQuality, &QSpinBox::setValue);
    connect(m_spnQuality, QOverload<int>::of(&QSpinBox::valueChanged), m_sldQuality, &QSlider::setValue);

    connect(m_rbBlur, &QRadioButton::toggled, this, &SettingsDialog::onHidingMethodToggled);
    connect(m_rbPixelate, &QRadioButton::toggled, this, &SettingsDialog::onHidingMethodToggled);
    connect(m_rbBlackBar, &QRadioButton::toggled, this, &SettingsDialog::onHidingMethodToggled);
    connect(m_rbEmoji, &QRadioButton::toggled, this, &SettingsDialog::onHidingMethodToggled);

    connect(m_rbSizeOriginal, &QRadioButton::toggled, this, &SettingsDialog::onResizeModeToggled);
    connect(m_rbSizeManset, &QRadioButton::toggled, this, &SettingsDialog::onResizeModeToggled);
    connect(m_rbSizeEOkul, &QRadioButton::toggled, this, &SettingsDialog::onResizeModeToggled);
    connect(m_rbSizeMebbis, &QRadioButton::toggled, this, &SettingsDialog::onResizeModeToggled);
    connect(m_rbSizeInstaSquare, &QRadioButton::toggled, this, &SettingsDialog::onResizeModeToggled);
    connect(m_rbSizeInstaPortrait, &QRadioButton::toggled, this, &SettingsDialog::onResizeModeToggled);
    connect(m_rbSizeInstaStory, &QRadioButton::toggled, this, &SettingsDialog::onResizeModeToggled);
    connect(m_rbSizeCustom, &QRadioButton::toggled, this, &SettingsDialog::onResizeModeToggled);

    connect(m_sldWatermarkOpacity, &QSlider::valueChanged, m_spnWatermarkOpacity, &QSpinBox::setValue);
    connect(m_spnWatermarkOpacity, QOverload<int>::of(&QSpinBox::valueChanged), m_sldWatermarkOpacity, &QSlider::setValue);

    connect(m_sldWatermarkScale, &QSlider::valueChanged, m_spnWatermarkScale, &QSpinBox::setValue);
    connect(m_spnWatermarkScale, QOverload<int>::of(&QSpinBox::valueChanged), m_sldWatermarkScale, &QSlider::setValue);

    connect(m_cmbWatermarkType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onWatermarkTypeChanged);
    connect(m_btnBrowseWatermarkImage, &QPushButton::clicked,
            this, &SettingsDialog::onBrowseWatermarkImage);

    connect(m_btnDefaultMebLogo, &QPushButton::clicked, this, [this]() {
        QString logoPath = QStringLiteral(":/icons/app_icon.png");
        m_edtWatermarkImage->setText(logoPath);
        m_lblWatermarkImagePreview->setPixmap(QPixmap(logoPath));
        if (m_cmbWatermarkType->currentIndex() == 0) {
            m_cmbWatermarkType->setCurrentIndex(1);
        }
    });

    connect(m_btnBrowseOutput, &QPushButton::clicked, this, &SettingsDialog::onBrowseOutputDir);
    connect(m_btnBrowseEmoji, &QPushButton::clicked, this, &SettingsDialog::onBrowseCustomEmoji);
    connect(btnReset, &QPushButton::clicked, this, &SettingsDialog::onResetDefaults);
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void SettingsDialog::loadSettingsToUi(const ProcessSettings& s)
{
    // Algılama
    int idx = m_cmbDetector->findData(static_cast<int>(s.detectorType));
    if (idx >= 0) m_cmbDetector->setCurrentIndex(idx);
    m_spnConfidence->setValue(s.confidenceThreshold);
    m_sldConfidence->setValue(static_cast<int>(s.confidenceThreshold * 100));
    m_spnMinFaceSize->setValue(s.minFaceSize);

    // Gizleme
    switch (s.hidingMethod) {
    case HidingMethod::GaussianBlur: m_rbBlur->setChecked(true); break;
    case HidingMethod::Pixelate: m_rbPixelate->setChecked(true); break;
    case HidingMethod::BlackBar: m_rbBlackBar->setChecked(true); break;
    case HidingMethod::Emoji: m_rbEmoji->setChecked(true); break;
    }

    m_sldBlurIntensity->setValue(s.blurIntensity);
    m_spnBlurIntensity->setValue(s.blurIntensity);
    m_sldPixelSize->setValue(s.pixelSize);
    m_spnPixelSize->setValue(s.pixelSize);

    m_selectedEmojiPath = s.emojiPath;
    m_edtEmojiPath->setText(s.emojiPath);
    if (!s.emojiPath.isEmpty()) {
        QPixmap pm(s.emojiPath);
        if (pm.isNull()) {
            pm.load(QStringLiteral(":/emojis/smile.png"));
        }
        m_lblEmojiPreview->setPixmap(pm);
    }

    // MEB Boyutlandırma
    switch (s.resizeMode) {
    case ResizeMode::Original: m_rbSizeOriginal->setChecked(true); break;
    case ResizeMode::MebManset: m_rbSizeManset->setChecked(true); break;
    case ResizeMode::EOkul: m_rbSizeEOkul->setChecked(true); break;
    case ResizeMode::Mebbis: m_rbSizeMebbis->setChecked(true); break;
    case ResizeMode::InstagramSquare: m_rbSizeInstaSquare->setChecked(true); break;
    case ResizeMode::InstagramPortrait: m_rbSizeInstaPortrait->setChecked(true); break;
    case ResizeMode::InstagramStory: m_rbSizeInstaStory->setChecked(true); break;
    case ResizeMode::CustomSize: m_rbSizeCustom->setChecked(true); break;
    }
    m_spnCustomWidth->setValue(s.customWidth);
    m_spnCustomHeight->setValue(s.customHeight);
    m_chkMaintainAspect->setChecked(s.maintainAspectRatio);

    // Okul Logosu ve Filigran
    int wmTypeIdx = m_cmbWatermarkType->findData(static_cast<int>(s.watermarkType));
    if (wmTypeIdx >= 0) m_cmbWatermarkType->setCurrentIndex(wmTypeIdx);
    m_edtWatermarkImage->setText(s.watermarkImagePath);
    if (!s.watermarkImagePath.isEmpty()) {
        m_lblWatermarkImagePreview->setPixmap(QPixmap(s.watermarkImagePath));
    } else {
        m_lblWatermarkImagePreview->clear();
    }
    m_edtWatermarkText->setText(s.watermarkText);
    int posIdx = m_cmbWatermarkPosition->findData(static_cast<int>(s.watermarkPosition));
    if (posIdx >= 0) m_cmbWatermarkPosition->setCurrentIndex(posIdx);
    m_customXRatio = s.watermarkCustomXRatio;
    m_customYRatio = s.watermarkCustomYRatio;
    m_sldWatermarkOpacity->setValue(s.watermarkOpacity);
    m_spnWatermarkOpacity->setValue(s.watermarkOpacity);
    m_sldWatermarkScale->setValue(s.watermarkScalePercent);
    m_spnWatermarkScale->setValue(s.watermarkScalePercent);
    m_chkWatermarkTextBg->setChecked(s.watermarkTextBackground);

    // Çıktı
    m_edtOutputDir->setText(s.outputDirectory);
    int fmtIdx = m_cmbOutputFormat->findData(static_cast<int>(s.outputFormat));
    if (fmtIdx >= 0) m_cmbOutputFormat->setCurrentIndex(fmtIdx);
    m_sldQuality->setValue(s.jpegQuality);
    m_spnQuality->setValue(s.jpegQuality);
    m_edtSuffix->setText(s.filenameSuffix);
    m_chkSkipNoFace->setChecked(s.skipNoFace);
    m_spnMaxThreads->setValue(s.maxThreads);

    m_chkEnableMaxFileSize->setChecked(s.enableMaxFileSize);
    m_spnMaxFileSizeKb->setValue(s.maxFileSizeKb > 0 ? s.maxFileSizeKb : 1000);
    m_spnMaxFileSizeKb->setEnabled(s.enableMaxFileSize);
    if (s.maxFileSizeAction == MaxFileSizeAction::CompressQualityOnly) {
        m_rbCompressQualityOnly->setChecked(true);
    } else {
        m_rbCompressAndResize->setChecked(true);
    }
    m_rbCompressQualityOnly->setEnabled(s.enableMaxFileSize);
    m_rbCompressAndResize->setEnabled(s.enableMaxFileSize);

    onHidingMethodToggled();
    onResizeModeToggled();
    onWatermarkTypeChanged(m_cmbWatermarkType->currentIndex());
}

ProcessSettings SettingsDialog::settings() const
{
    ProcessSettings s;
    s.detectorType = static_cast<DetectorType>(m_cmbDetector->currentData().toInt());
    s.confidenceThreshold = static_cast<float>(m_spnConfidence->value());
    s.minFaceSize = m_spnMinFaceSize->value();

    if (m_rbBlur->isChecked()) s.hidingMethod = HidingMethod::GaussianBlur;
    else if (m_rbPixelate->isChecked()) s.hidingMethod = HidingMethod::Pixelate;
    else if (m_rbBlackBar->isChecked()) s.hidingMethod = HidingMethod::BlackBar;
    else if (m_rbEmoji->isChecked()) s.hidingMethod = HidingMethod::Emoji;

    s.blurIntensity = m_spnBlurIntensity->value();
    s.pixelSize = m_spnPixelSize->value();
    s.emojiPath = m_edtEmojiPath->text().trimmed();

    if (m_rbSizeOriginal->isChecked()) s.resizeMode = ResizeMode::Original;
    else if (m_rbSizeManset->isChecked()) s.resizeMode = ResizeMode::MebManset;
    else if (m_rbSizeEOkul->isChecked()) s.resizeMode = ResizeMode::EOkul;
    else if (m_rbSizeMebbis->isChecked()) s.resizeMode = ResizeMode::Mebbis;
    else if (m_rbSizeInstaSquare->isChecked()) s.resizeMode = ResizeMode::InstagramSquare;
    else if (m_rbSizeInstaPortrait->isChecked()) s.resizeMode = ResizeMode::InstagramPortrait;
    else if (m_rbSizeInstaStory->isChecked()) s.resizeMode = ResizeMode::InstagramStory;
    else if (m_rbSizeCustom->isChecked()) s.resizeMode = ResizeMode::CustomSize;

    s.customWidth = m_spnCustomWidth->value();
    s.customHeight = m_spnCustomHeight->value();
    s.maintainAspectRatio = m_chkMaintainAspect->isChecked();

    s.watermarkType = static_cast<WatermarkType>(m_cmbWatermarkType->currentData().toInt());
    s.watermarkImagePath = m_edtWatermarkImage->text().trimmed();
    s.watermarkText = m_edtWatermarkText->text().trimmed();
    s.watermarkPosition = static_cast<WatermarkPosition>(m_cmbWatermarkPosition->currentData().toInt());
    s.watermarkCustomXRatio = m_customXRatio;
    s.watermarkCustomYRatio = m_customYRatio;
    s.watermarkOpacity = m_spnWatermarkOpacity->value();
    s.watermarkScalePercent = m_spnWatermarkScale->value();
    s.watermarkTextBackground = m_chkWatermarkTextBg->isChecked();

    s.outputDirectory = m_edtOutputDir->text().trimmed();
    s.outputFormat = static_cast<OutputFormat>(m_cmbOutputFormat->currentData().toInt());
    s.jpegQuality = m_spnQuality->value();
    s.filenameSuffix = m_edtSuffix->text().trimmed();
    s.skipNoFace = m_chkSkipNoFace->isChecked();
    s.maxThreads = m_spnMaxThreads->value();

    s.enableMaxFileSize = m_chkEnableMaxFileSize->isChecked();
    s.maxFileSizeKb = m_spnMaxFileSizeKb->value();
    s.maxFileSizeAction = m_rbCompressQualityOnly->isChecked()
        ? MaxFileSizeAction::CompressQualityOnly
        : MaxFileSizeAction::CompressAndResize;

    return s;
}

QString SettingsDialog::language() const
{
    return m_cmbLanguage->currentData().toString();
}

void SettingsDialog::onBrowseOutputDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Çıktı Klasörü Seç"), m_edtOutputDir->text());
    if (!dir.isEmpty()) {
        m_edtOutputDir->setText(dir);
    }
}

void SettingsDialog::onBrowseCustomEmoji()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Emoji Dosyası Seç"), QString(),
                                                tr("Görüntü Dosyaları (*.png *.svg *.webp)"));
    if (!file.isEmpty()) {
        m_edtEmojiPath->setText(file);
        m_selectedEmojiPath = file;
        m_lblEmojiPreview->setPixmap(QPixmap(file));
        m_rbEmoji->setChecked(true);
    }
}

void SettingsDialog::onBrowseWatermarkImage()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Okul Logosu Seç"), QString(),
                                                tr("Görsel Dosyaları (*.png *.svg *.webp *.jpg *.jpeg)"));
    if (!file.isEmpty()) {
        m_edtWatermarkImage->setText(file);
        m_lblWatermarkImagePreview->setPixmap(QPixmap(file));
        if (m_cmbWatermarkType->currentIndex() == 0) {
            m_cmbWatermarkType->setCurrentIndex(1); // Yalnızca Logo
        }
    }
}

void SettingsDialog::onWatermarkTypeChanged(int /*index*/)
{
    WatermarkType type = static_cast<WatermarkType>(m_cmbWatermarkType->currentData().toInt());
    bool enableImg = (type == WatermarkType::Image || type == WatermarkType::Both);
    bool enableText = (type == WatermarkType::Text || type == WatermarkType::Both);
    bool anyEnabled = (type != WatermarkType::None);

    m_edtWatermarkImage->setEnabled(enableImg);
    m_btnBrowseWatermarkImage->setEnabled(enableImg);
    m_btnDefaultMebLogo->setEnabled(enableImg);
    m_edtWatermarkText->setEnabled(enableText);
    m_cmbWatermarkPosition->setEnabled(anyEnabled);
    m_sldWatermarkOpacity->setEnabled(anyEnabled);
    m_spnWatermarkOpacity->setEnabled(anyEnabled);
    m_sldWatermarkScale->setEnabled(enableImg);
    m_spnWatermarkScale->setEnabled(enableImg);
    m_chkWatermarkTextBg->setEnabled(anyEnabled);
}

void SettingsDialog::onHidingMethodToggled()
{
    m_sldBlurIntensity->setEnabled(m_rbBlur->isChecked());
    m_spnBlurIntensity->setEnabled(m_rbBlur->isChecked());

    m_sldPixelSize->setEnabled(m_rbPixelate->isChecked());
    m_spnPixelSize->setEnabled(m_rbPixelate->isChecked());

    bool isEmoji = m_rbEmoji->isChecked();
    m_edtEmojiPath->setEnabled(isEmoji);
    m_btnBrowseEmoji->setEnabled(isEmoji);
}

void SettingsDialog::onResizeModeToggled()
{
    bool isCustom = m_rbSizeCustom->isChecked();
    m_spnCustomWidth->setEnabled(isCustom);
    m_spnCustomHeight->setEnabled(isCustom);
    m_chkMaintainAspect->setEnabled(isCustom);

    // Eğer e-Okul seçildiyse formatı otomatik JPEG yap ve uyar
    if (m_rbSizeEOkul->isChecked()) {
        int jpegIdx = m_cmbOutputFormat->findData(static_cast<int>(OutputFormat::JPEG));
        if (jpegIdx >= 0) m_cmbOutputFormat->setCurrentIndex(jpegIdx);
    }
}

void SettingsDialog::onResetDefaults()
{
    ProcessSettings def;
    loadSettingsToUi(def);
}

void SettingsDialog::onEmojiButtonClicked()
{
    auto* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString name = btn->property("emojiName").toString();
    QString path = QStringLiteral(":/emojis/%1.png").arg(name);

    m_edtEmojiPath->setText(path);
    m_selectedEmojiPath = path;
    m_lblEmojiPreview->setPixmap(QPixmap(path));
    m_rbEmoji->setChecked(true);
}
