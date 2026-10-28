#ifndef BATCH_RESULT_H
#define BATCH_RESULT_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QMetaType>

/// Tek dosyanın işlem sonucu
struct FileResult {
    QString filePath;       ///< Kaynak dosya yolu
    QString outputPath;     ///< Çıktı dosya yolu
    int facesFound = 0;     ///< Algılanan yüz sayısı
    bool success = false;   ///< İşlem başarılı mı
    QString errorMessage;   ///< Hata mesajı (varsa)
};

/// Toplu işlem sonucu
struct BatchResult {
    int totalFiles = 0;             ///< Toplam dosya sayısı
    int successCount = 0;           ///< Başarılı dosya sayısı
    int errorCount = 0;             ///< Hatalı dosya sayısı
    int noFaceCount = 0;            ///< Yüz bulunamayan dosya sayısı
    int totalFacesHidden = 0;       ///< Toplam gizlenen yüz sayısı
    double elapsedSeconds = 0.0;    ///< Geçen süre (saniye)
    QString outputDirectory;        ///< Çıktı klasörü

    QList<FileResult> fileResults;  ///< Her dosyanın detaylı sonucu

    /// Başarı yüzdesi
    double successRate() const {
        return totalFiles > 0 ? (static_cast<double>(successCount) / totalFiles) * 100.0 : 0.0;
    }
};

Q_DECLARE_METATYPE(FileResult)
Q_DECLARE_METATYPE(BatchResult)

#endif // BATCH_RESULT_H
