#ifndef BATCH_PROCESSOR_H
#define BATCH_PROCESSOR_H

#include "FaceHider.h"
#include "ProcessSettings.h"
#include "BatchResult.h"
#include <QObject>
#include <QStringList>
#include <QMap>
#include <atomic>

/**
 * @brief Toplu fotoğraf işleme motoru
 *
 * Çoklu fotoğrafı sırasıyla veya paralel olarak işler.
 * İlerleme raporlaması, MEB standart boyutlandırması ve iptal mekanizması sağlar.
 */
class BatchProcessor : public QObject {
    Q_OBJECT

public:
    explicit BatchProcessor(QObject* parent = nullptr);
    ~BatchProcessor() override = default;

    /// Manuel kullanıcı seçimlerini tanımlar (muaf yüzler ve elle eklenen alanlar)
    void setManualOverrides(const QMap<QString, QList<FaceItem>>& overrides);

public slots:
    /**
     * @brief Toplu işlemi başlatır
     * @param files İşlenecek dosya yolları
     * @param settings İşlem ayarları
     * @param modelsDir Model dosyaları dizini
     */
    void processBatch(const QStringList& files,
                      const ProcessSettings& settings,
                      const QString& modelsDir);

    /**
     * @brief İşlemi iptal eder
     */
    void cancel();

public:
    bool isRunning() const { return m_running; }

signals:
    void progressUpdated(int current, int total, const QString& currentFile);
    void fileProcessed(const FileResult& result);
    void batchCompleted(const BatchResult& result);
    void errorOccurred(const QString& file, const QString& error);

private:
    std::atomic<bool> m_cancelled{false};
    std::atomic<bool> m_running{false};
    QMap<QString, QList<FaceItem>> m_manualOverrides;

    FileResult processFile(const QString& filePath,
                           const ProcessSettings& settings,
                           FaceHider& hider,
                           IHidingStrategy* rawStrategy);

    QString buildOutputPath(const QString& inputPath,
                            const ProcessSettings& settings);
};

#endif // BATCH_PROCESSOR_H
