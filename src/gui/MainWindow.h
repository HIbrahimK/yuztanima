#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ProcessSettings.h"
#include "BatchResult.h"
#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include <QThread>
#include <QMap>

class FileListWidget;
class ImagePreviewWidget;
class BatchProcessor;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAddFiles();
    void onAddFolder();
    void onOpenSettings();
    void onStartBatch();
    void onCancelBatch();
    void onOpenOutputDir();
    void onAbout();
    void onFileSelected(const QString& filePath);
    void onFacesModified(const QString& filePath, const QList<FaceItem>& faces);
    void onSaveSingle(const QString& filePath);
    void onBatchProgress(int current, int total, const QString& currentFile);
    void onFileProcessed(const FileResult& result);
    void onBatchCompleted(const BatchResult& result);
    void onBatchError(const QString& file, const QString& error);

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();
    void updateUiForProcessing(bool isProcessing);

    FileListWidget* m_fileListWidget = nullptr;
    ImagePreviewWidget* m_previewWidget = nullptr;

    // Durum çubuğu bileşenleri
    QProgressBar* m_progressBar = nullptr;
    QLabel* m_lblStatus = nullptr;

    // Eylemler
    QAction* m_actStart = nullptr;
    QAction* m_actCancel = nullptr;
    QAction* m_actSaveSingle = nullptr;
    QAction* m_actAddFiles = nullptr;
    QAction* m_actAddFolder = nullptr;
    QAction* m_actSettings = nullptr;
    QAction* m_actOpenOutput = nullptr;

    // İşlemci ve iş parçacığı
    BatchProcessor* m_batchProcessor = nullptr;
    QThread* m_workerThread = nullptr;

    // Aktif ayarlar ve durum
    ProcessSettings m_settings;
    QString m_modelsDir;
    QString m_currentPreviewFile;
    QString m_lastOutputDir;

    // Fotoğraf başına yapılan özel manuel düzenlemeler ve muafiyetler
    QMap<QString, QList<FaceItem>> m_manualFaceOverrides;
};

#endif // MAIN_WINDOW_H
