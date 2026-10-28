#include "MainWindow.h"
#include "FileListWidget.h"
#include "ImagePreviewWidget.h"
#include "SettingsDialog.h"
#include "BatchResultDialog.h"
#include "AboutDialog.h"
#include "BatchProcessor.h"
#include "SettingsManager.h"
#include "ImageUtils.h"

#include <QSplitter>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QIcon>
#include <QFileInfo>
#include <QDir>
#include <opencv2/imgcodecs.hpp>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Ayarları ve model dizinini yükle
    m_settings = SettingsManager::instance().loadSettings();
    m_modelsDir = SettingsManager::instance().resolveModelsDirectory();

    setWindowTitle(tr("YüzGizle — MEB KVKK Uyumlu Fotoğraf Yüz Gizleme"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/app_icon.png")));
    resize(1240, 780);

    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupConnections();

    // İş parçacığını ve BatchProcessor'ı başlat
    m_workerThread = new QThread(this);
    m_batchProcessor = new BatchProcessor();
    m_batchProcessor->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_batchProcessor, &QObject::deleteLater);
    connect(m_batchProcessor, &BatchProcessor::progressUpdated, this, &MainWindow::onBatchProgress);
    connect(m_batchProcessor, &BatchProcessor::fileProcessed, this, &MainWindow::onFileProcessed);
    connect(m_batchProcessor, &BatchProcessor::batchCompleted, this, &MainWindow::onBatchCompleted);
    connect(m_batchProcessor, &BatchProcessor::errorOccurred, this, &MainWindow::onBatchError);

    m_workerThread->start();
}

MainWindow::~MainWindow()
{
    if (m_batchProcessor && m_batchProcessor->isRunning()) {
        m_batchProcessor->cancel();
    }
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait(3000);
    }
}

void MainWindow::setupUi()
{
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    m_fileListWidget = new FileListWidget(this);
    m_previewWidget = new ImagePreviewWidget(this);

    splitter->addWidget(m_fileListWidget);
    splitter->addWidget(m_previewWidget);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 7);

    setCentralWidget(splitter);
}

void MainWindow::setupMenuBar()
{
    // Dosya Menüsü
    auto* menuFile = menuBar()->addMenu(tr("&Dosya"));
    m_actAddFiles = menuFile->addAction(tr("📁 Fotoğraf Ekle..."), QKeySequence::Open, this, &MainWindow::onAddFiles);
    m_actAddFolder = menuFile->addAction(tr("📂 Klasör Ekle..."), this, &MainWindow::onAddFolder);
    menuFile->addSeparator();
    m_actSaveSingle = menuFile->addAction(tr("💾 Bu Fotoğrafı Kaydet..."), QKeySequence::Save, this, [this]() {
        onSaveSingle(m_currentPreviewFile);
    });
    m_actSaveSingle->setEnabled(false);
    menuFile->addSeparator();
    m_actOpenOutput = menuFile->addAction(tr("📂 Çıktı Klasörünü Aç"), this, &MainWindow::onOpenOutputDir);
    menuFile->addSeparator();
    menuFile->addAction(tr("&Çıkış"), QKeySequence::Quit, this, &QWidget::close);

    // İşlem Menüsü
    auto* menuProcess = menuBar()->addMenu(tr("&İşlem"));
    m_actStart = menuProcess->addAction(tr("▶️ Toplu İşlemi Başlat"), QKeySequence(Qt::Key_F5), this, &MainWindow::onStartBatch);
    m_actCancel = menuProcess->addAction(tr("⏹️ İşlemi Durdur"), QKeySequence(Qt::Key_Escape), this, &MainWindow::onCancelBatch);
    m_actCancel->setEnabled(false);

    // Ayarlar Menüsü
    auto* menuSettings = menuBar()->addMenu(tr("&Ayarlar"));
    m_actSettings = menuSettings->addAction(tr("⚙️ Tercihler & MEB Ayarları..."), this, &MainWindow::onOpenSettings);

    // Yardım Menüsü
    auto* menuHelp = menuBar()->addMenu(tr("&Yardım"));
    menuHelp->addAction(tr("ℹ️ Hakkında & MEB Standartları"), this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar()
{
    auto* toolbar = addToolBar(tr("Ana Araç Çubuğu"));
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    toolbar->addAction(m_actAddFiles);
    toolbar->addAction(m_actAddFolder);
    toolbar->addSeparator();
    toolbar->addAction(m_actSaveSingle);
    toolbar->addSeparator();
    toolbar->addAction(m_actStart);
    toolbar->addAction(m_actCancel);
    toolbar->addSeparator();
    toolbar->addAction(m_actSettings);
    toolbar->addAction(m_actOpenOutput);
}

void MainWindow::setupStatusBar()
{
    m_lblStatus = new QLabel(tr("Hazır"), this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMaximumWidth(250);
    m_progressBar->setVisible(false);

    statusBar()->addWidget(m_lblStatus, 1);
    statusBar()->addPermanentWidget(m_progressBar);
}

void MainWindow::setupConnections()
{
    connect(m_fileListWidget, &FileListWidget::fileSelected, this, &MainWindow::onFileSelected);
    connect(m_fileListWidget, &FileListWidget::startBatchRequested, this, &MainWindow::onStartBatch);
    connect(m_fileListWidget, &FileListWidget::cancelBatchRequested, this, &MainWindow::onCancelBatch);
    connect(m_previewWidget, &ImagePreviewWidget::facesModified, this, &MainWindow::onFacesModified);
    connect(m_previewWidget, &ImagePreviewWidget::saveSingleRequested, this, &MainWindow::onSaveSingle);
    connect(m_previewWidget, &ImagePreviewWidget::resizePresetChanged, this, [this](ResizeMode mode) {
        m_settings.resizeMode = mode;
    });
    connect(m_previewWidget, &ImagePreviewWidget::customSizeModified, this, [this](int w, int h, bool maintain) {
        m_settings.customWidth = w;
        m_settings.customHeight = h;
        m_settings.maintainAspectRatio = maintain;
        m_settings.resizeMode = ResizeMode::CustomSize;
    });
}

void MainWindow::onAddFiles()
{
    QString lastDir = SettingsManager::instance().lastDirectory();
    QStringList files = QFileDialog::getOpenFileNames(
        this, tr("İşlenecek Fotoğrafları Seç"), lastDir, ImageUtils::fileFilter());

    if (!files.isEmpty()) {
        SettingsManager::instance().setLastDirectory(QFileInfo(files.first()).absolutePath());
        m_fileListWidget->addFiles(files);
        if (m_currentPreviewFile.isEmpty()) {
            onFileSelected(files.first());
        }
    }
}

void MainWindow::onAddFolder()
{
    QString lastDir = SettingsManager::instance().lastDirectory();
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Fotoğrafların Bulunduğu Klasörü Seç"), lastDir);

    if (!dir.isEmpty()) {
        SettingsManager::instance().setLastDirectory(dir);
        m_fileListWidget->addDirectory(dir);
    }
}

void MainWindow::onOpenSettings()
{
    QString currentLang = SettingsManager::instance().language();
    SettingsDialog dlg(m_settings, currentLang, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_settings = dlg.settings();
        SettingsManager::instance().saveSettings(m_settings);

        if (dlg.language() != currentLang) {
            SettingsManager::instance().setLanguage(dlg.language());
            QMessageBox::information(this, tr("Dil Değişikliği"),
                tr("Dil değişikliğinin tam uygulanması için uygulamayı yeniden başlatmanız önerilir."));
        }

        // Önizlemeyi yeni ayarlarla güncelle
        if (!m_currentPreviewFile.isEmpty()) {
            QList<FaceItem> existing = m_manualFaceOverrides.value(m_currentPreviewFile);
            m_previewWidget->showPreview(m_currentPreviewFile, m_settings, m_modelsDir, existing);
        }
    }
}

void MainWindow::onStartBatch()
{
    QStringList filesToProcess = m_fileListWidget->checkedFiles();
    if (filesToProcess.isEmpty()) {
        QMessageBox::warning(this, tr("Uyarı"), tr("İşlenecek işaretli fotoğraf bulunamadı!\nLütfen listeden fotoğraf seçin."));
        return;
    }

    m_fileListWidget->resetStatuses();
    updateUiForProcessing(true);

    m_progressBar->setRange(0, filesToProcess.size());
    m_progressBar->setValue(0);
    m_progressBar->setVisible(true);

    m_lblStatus->setText(tr("Toplu işlem başlatılıyor..."));

    // Kullanıcının yaptığı manuel seçimleri ve muafiyetleri işlemciye aktar
    m_batchProcessor->setManualOverrides(m_manualFaceOverrides);

    // BatchProcessor'ı arka plandaki thread'de çalıştır
    bool ok = QMetaObject::invokeMethod(m_batchProcessor, "processBatch",
                              Qt::QueuedConnection,
                              Q_ARG(QStringList, filesToProcess),
                              Q_ARG(ProcessSettings, m_settings),
                              Q_ARG(QString, m_modelsDir));

    if (!ok) {
        qWarning() << "HATA: BatchProcessor::processBatch invokeMethod başarısız oldu!";
        m_lblStatus->setText(tr("Hata: İşlem başlatılamadı!"));
        updateUiForProcessing(false);
        m_progressBar->setVisible(false);
    }
}

void MainWindow::onCancelBatch()
{
    if (m_batchProcessor && m_batchProcessor->isRunning()) {
        m_lblStatus->setText(tr("İşlem durduruluyor..."));
        m_batchProcessor->cancel();
    } else {
        updateUiForProcessing(false);
        m_progressBar->setVisible(false);
        m_lblStatus->setText(tr("İşlem iptal edildi."));
    }
}

void MainWindow::onOpenOutputDir()
{
    QString dir = m_lastOutputDir;
    if (dir.isEmpty() || !QDir(dir).exists()) {
        dir = m_settings.outputDirectory;
    }
    if (dir.isEmpty() || !QDir(dir).exists()) {
        dir = SettingsManager::instance().lastDirectory();
    }

    if (!dir.isEmpty() && QDir(dir).exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    } else {
        QMessageBox::information(this, tr("Bilgi"), tr("Henüz bir çıktı klasörü oluşturulmadı."));
    }
}

void MainWindow::onAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::onFileSelected(const QString& filePath)
{
    m_currentPreviewFile = filePath;
    m_actSaveSingle->setEnabled(!filePath.isEmpty());

    QList<FaceItem> existing = m_manualFaceOverrides.value(filePath);
    m_previewWidget->showPreview(filePath, m_settings, m_modelsDir, existing);
}

void MainWindow::onFacesModified(const QString& filePath, const QList<FaceItem>& faces)
{
    m_manualFaceOverrides[filePath] = faces;

    int hidingCount = 0;
    int exemptCount = 0;
    for (const auto& f : faces) {
        if (f.enabled) hidingCount++;
        else exemptCount++;
    }

    QString status = tr("%1 gizlenecek, %2 muaf").arg(hidingCount).arg(exemptCount);
    m_fileListWidget->updateFileStatus(filePath, status, hidingCount);
}

void MainWindow::onSaveSingle(const QString& filePath)
{
    if (filePath.isEmpty()) return;

    QFileInfo fi(filePath);

    // Çıktı dosya uzantısını belirle
    QString ext;
    if (m_settings.resizeMode == ResizeMode::EOkul) {
        ext = QStringLiteral(".jpg");
    } else {
        switch (m_settings.outputFormat) {
        case OutputFormat::SameAsSource: ext = QStringLiteral(".%1").arg(fi.suffix().toLower()); break;
        case OutputFormat::JPEG: ext = QStringLiteral(".jpg"); break;
        case OutputFormat::PNG: ext = QStringLiteral(".png"); break;
        case OutputFormat::WEBP: ext = QStringLiteral(".webp"); break;
        case OutputFormat::BMP: ext = QStringLiteral(".bmp"); break;
        }
    }

    QString defaultSaveName = fi.completeBaseName() + m_settings.filenameSuffix + ext;
    QString defaultDir = m_settings.outputDirectory.isEmpty() ? fi.absolutePath() : m_settings.outputDirectory;
    QString defaultPath = QDir(defaultDir).filePath(defaultSaveName);

    QString filter;
    if (ext == ".jpg") filter = tr("JPEG Görüntüsü (*.jpg *.jpeg)");
    else if (ext == ".png") filter = tr("PNG Görüntüsü (*.png)");
    else if (ext == ".webp") filter = tr("WebP Görüntüsü (*.webp)");
    else filter = tr("Tüm Dosyalar (*.*)");

    QString savePath = QFileDialog::getSaveFileName(this, tr("Fotoğrafı Kaydet"), defaultPath, filter);
    if (savePath.isEmpty()) return;

    cv::Mat processed = m_previewWidget->currentProcessedImage();
    if (processed.empty()) {
        QMessageBox::warning(this, tr("Hata"), tr("Kaydedilecek işlenmiş görüntü bulunamadı!"));
        return;
    }

    bool success = ImageUtils::saveImageWithSettings(savePath, processed, m_settings);
    if (success) {
        qint64 sizeBytes = ImageUtils::getFileSize(savePath);
        double sizeKb = sizeBytes / 1024.0;
        m_lastOutputDir = QFileInfo(savePath).absolutePath();
        m_lblStatus->setText(tr("Kaydedildi: %1 (%2 KB)").arg(QFileInfo(savePath).fileName()).arg(sizeKb, 0, 'f', 1));
        QMessageBox::information(this, tr("Kayıt Başarılı"),
            tr("Fotoğraf başarıyla kaydedildi:\n%1\n\nDosya Boyutu: %2 KB (%3 byte)")
                .arg(savePath)
                .arg(sizeKb, 0, 'f', 1)
                .arg(sizeBytes));
    } else {
        QMessageBox::critical(this, tr("Hata"), tr("Fotoğraf kaydedilemedi!"));
    }
}

void MainWindow::onBatchProgress(int current, int total, const QString& currentFile)
{
    m_progressBar->setValue(current);
    m_lblStatus->setText(tr("İşleniyor (%1/%2): %3").arg(current).arg(total).arg(currentFile));
}

void MainWindow::onFileProcessed(const FileResult& result)
{
    QString status = result.success ? tr("Tamamlandı") : tr("Hata");
    m_fileListWidget->updateFileStatus(result.filePath, status, result.facesFound, !result.success);
}

void MainWindow::onBatchCompleted(const BatchResult& result)
{
    updateUiForProcessing(false);
    m_progressBar->setVisible(false);
    m_lastOutputDir = result.outputDirectory;

    m_lblStatus->setText(tr("Tamamlandı: %1/%2 başarılı, %3 yüz gizlendi (%4 sn)")
                         .arg(result.successCount)
                         .arg(result.totalFiles)
                         .arg(result.totalFacesHidden)
                         .arg(result.elapsedSeconds, 0, 'f', 1));

    // Sonuç özet diyalogunu göster
    BatchResultDialog dlg(result, this);
    dlg.exec();
}

void MainWindow::onBatchError(const QString& file, const QString& error)
{
    m_lblStatus->setText(tr("Hata (%1): %2").arg(QFileInfo(file).fileName()).arg(error));
}

void MainWindow::updateUiForProcessing(bool isProcessing)
{
    m_actStart->setEnabled(!isProcessing);
    m_actCancel->setEnabled(isProcessing);
    m_actAddFiles->setEnabled(!isProcessing);
    m_actAddFolder->setEnabled(!isProcessing);
    m_actSettings->setEnabled(!isProcessing);
    m_actSaveSingle->setEnabled(!isProcessing && !m_currentPreviewFile.isEmpty());
    m_fileListWidget->setProcessingState(isProcessing);
}
