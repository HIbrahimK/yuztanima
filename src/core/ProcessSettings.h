#ifndef PROCESS_SETTINGS_H
#define PROCESS_SETTINGS_H

#include <QString>
#include <QList>
#include <QMetaType>
#include <opencv2/core.hpp>

/// Yüz algılama yöntemi
enum class DetectorType {
    YuNet,        ///< DNN tabanlı YuNet (ONNX) — yüksek doğruluk
    HaarCascade   ///< Haar Cascade — hızlı, düşük doğruluk
};

/// Yüz gizleme yöntemi
enum class HidingMethod {
    GaussianBlur,   ///< Gaussian bulanıklaştırma
    Pixelate,       ///< Pikselleştirme (mozaik)
    BlackBar,       ///< Siyah bant (göz hizası)
    Emoji           ///< Emoji/smiley kaplama
};

/// Çıktı görüntü formatı
enum class OutputFormat {
    SameAsSource,   ///< Kaynak dosya ile aynı format
    JPEG,           ///< JPEG (.jpg)
    PNG,            ///< PNG (.png)
    WEBP,           ///< WebP (.webp)
    BMP             ///< BMP (.bmp)
};

/// Yeniden boyutlandırma ve MEB şablon modları
enum class ResizeMode {
    Original,          ///< Orijinal boyutu koru
    MebManset,         ///< MEB Web Sitesi Manşet Haberi: 1200x568 px (19:9 en-boy oranı, en az 1200px)
    EOkul,             ///< MEB e-Okul Standardı: 133x171 px (35x45mm vesikalık), < 20 KB JPEG
    Mebbis,            ///< MEBBİS Vesikalık Standardı: 394x512 px
    InstagramSquare,   ///< Instagram Kare Gönderi: 1080x1080 px (1:1)
    InstagramPortrait, ///< Instagram Dikey / Portre: 1080x1350 px (4:5)
    InstagramStory,    ///< Instagram Hikaye / Reels: 1080x1920 px (9:16)
    CustomSize         ///< Kullanıcı tanımlı özel boyut
};

/// Filigran ve okul logosu ekleme türü
enum class WatermarkType {
    None,          ///< Filigran yok
    Image,         ///< Yalnızca Okul Logosu (PNG/şeffaf görsel)
    Text,          ///< Yalnızca Metin Filigran
    Both           ///< Okul Logosu ve Metin Birlikte
};

/// Filigran ve logo konumu
enum class WatermarkPosition {
    BottomRight,   ///< Sağ Alt (Varsayılan standart)
    BottomLeft,    ///< Sol Alt
    TopRight,      ///< Sağ Üst
    TopLeft,       ///< Sol Üst
    Center,        ///< Merkez
    Custom         ///< Tuval üzerinde serbest konum
};

/// Manuel veya algılanmış yüz bölgesi
struct FaceItem {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    bool enabled = true;     ///< true: gizlenecek (öğrenci), false: muaf tutulacak (öğretmen/idareci)
    bool isManual = false;    ///< true: kullanıcı elle seçti, false: model otomatik algıladı
    QString label;

    cv::Rect toCvRect() const {
        return cv::Rect(x, y, width, height);
    }

    static FaceItem fromCvRect(const cv::Rect& r, bool enabled = true, bool isManual = false, const QString& label = QString()) {
        FaceItem item;
        item.x = r.x;
        item.y = r.y;
        item.width = r.width;
        item.height = r.height;
        item.enabled = enabled;
        item.isManual = isManual;
        item.label = label;
        return item;
    }
};

/// Maksimum dosya boyutu eylem türü
enum class MaxFileSizeAction {
    CompressQualityOnly,   ///< Çözünürlüğü değiştirmeden sadece kaliteyi düşür
    CompressAndResize      ///< Gerekirse piksel çözünürlüğünü de kademeli küçült
};

/// Toplu işlem ayarları
struct ProcessSettings {
    // -- Algılama ayarları --
    DetectorType detectorType = DetectorType::YuNet;
    float confidenceThreshold = 0.7f;   ///< Güven eşiği (0.0 - 1.0)
    int minFaceSize = 30;               ///< Minimum yüz boyutu (piksel)

    // -- Gizleme ayarları --
    HidingMethod hidingMethod = HidingMethod::GaussianBlur;
    int blurIntensity = 51;             ///< Blur kernel boyutu (tek sayı, 15-99)
    int pixelSize = 10;                 ///< Mozaik piksel boyutu (5-30)
    QString emojiPath;                  ///< Seçili emoji dosya yolu

    // -- MEB Standartları ve Boyutlandırma --
    ResizeMode resizeMode = ResizeMode::Original;
    int customWidth = 1200;             ///< Özel boyut genişlik (varsayılan MEB manşet 1200)
    int customHeight = 568;             ///< Özel boyut yükseklik (varsayılan MEB manşet 19:9 -> 568)
    bool maintainAspectRatio = true;    ///< En-boy oranını koruyarak ortadan kırp

    // -- Okul Logosu ve Filigran Ayarları --
    WatermarkType watermarkType = WatermarkType::None;
    QString watermarkImagePath;         ///< Okul logosu dosya yolu (şeffaf PNG/SVG)
    QString watermarkText;              ///< Filigran metni (örn: "T.C. Millî Eğitim Bakanlığı")
    WatermarkPosition watermarkPosition = WatermarkPosition::BottomRight;
    double watermarkCustomXRatio = 0.88; ///< Tuval üzerinde serbest X konumu (0.0 - 1.0)
    double watermarkCustomYRatio = 0.88; ///< Tuval üzerinde serbest Y konumu (0.0 - 1.0)
    int watermarkOpacity = 85;          ///< Filigran opaklığı (%10 - %100)
    int watermarkScalePercent = 15;     ///< Logo genişliğinin fotoğraf genişliğine oranı (%5 - %40)
    bool watermarkTextBackground = true;///< Metin için yarı saydam kontrast arka planı

    // -- Çıktı ayarları --
    QString outputDirectory;            ///< Çıktı klasörü (boşsa kaynak yanına "output")
    OutputFormat outputFormat = OutputFormat::JPEG;
    int jpegQuality = 90;               ///< JPEG kalitesi (1-100)
    QString filenameSuffix = "_gizli";  ///< Dosya adına eklenecek ek

    // -- Dosya Boyutu Sınırlayıcı (KB / MB) --
    bool enableMaxFileSize = false;     ///< Maksimum dosya boyutu sınırı aktif mi?
    int maxFileSizeKb = 1000;           ///< Maksimum hedef dosya boyutu (KB) (örn: 500, 1024)
    MaxFileSizeAction maxFileSizeAction = MaxFileSizeAction::CompressQualityOnly;

    // -- Gelişmiş --
    int maxThreads = 4;                 ///< Paralel iş parçacığı sayısı
    bool skipNoFace = false;            ///< Yüz bulunamazsa dosyayı atla
};

Q_DECLARE_METATYPE(FaceItem)
Q_DECLARE_METATYPE(QList<FaceItem>)
Q_DECLARE_METATYPE(ResizeMode)
Q_DECLARE_METATYPE(WatermarkType)
Q_DECLARE_METATYPE(WatermarkPosition)
Q_DECLARE_METATYPE(MaxFileSizeAction)
Q_DECLARE_METATYPE(ProcessSettings)

#endif // PROCESS_SETTINGS_H
