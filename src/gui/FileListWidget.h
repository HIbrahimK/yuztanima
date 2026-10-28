#ifndef FILE_LIST_WIDGET_H
#define FILE_LIST_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QStringList>

class FileListWidget : public QWidget {
    Q_OBJECT

public:
    explicit FileListWidget(QWidget* parent = nullptr);
    ~FileListWidget() override = default;

    /// Dosyaları listeye ekler (yinelenenleri eler)
    void addFiles(const QStringList& filePaths);

    /// Klasör içeriğini ekler (alt dizinler opsiyonel)
    void addDirectory(const QString& dirPath);

    /// İşlem için seçili (işaretli) dosyaların yollarını döndürür
    QStringList checkedFiles() const;

    /// Listedeki tüm dosya yollarını döndürür
    QStringList allFiles() const;

    /// Belirtilen dosyanın durumunu günceller
    void updateFileStatus(const QString& filePath, const QString& status, int facesFound, bool isError = false);

    /// Tüm durumları sıfırlar (yeni işlem öncesi)
    void resetStatuses();

    /// Toplam dosya sayısı
    int fileCount() const;

    /// İşlem durumuna göre Başlat / Bitir butonlarını ve kontrolleri günceller
    void setProcessingState(bool isProcessing);

signals:
    void fileSelected(const QString& filePath);
    void fileCountChanged(int totalCount, int checkedCount);
    void startBatchRequested();
    void cancelBatchRequested();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onSelectAllClicked();
    void onDeselectAllClicked();
    void onRemoveSelectedClicked();
    void onClearAllClicked();
    void onItemSelectionChanged();
    void onItemChanged(QTableWidgetItem* item);

private:
    void setupUi();
    void addSingleFile(const QString& filePath);
    void updateCounterLabel();

    QTableWidget* m_table = nullptr;
    QPushButton* m_btnSelectAll = nullptr;
    QPushButton* m_btnDeselectAll = nullptr;
    QPushButton* m_btnRemove = nullptr;
    QPushButton* m_btnClear = nullptr;
    QLabel* m_lblCount = nullptr;

    QPushButton* m_btnStartBatch = nullptr;
    QPushButton* m_btnCancelBatch = nullptr;
    bool m_isProcessing = false;

    QStringList m_fileList; // Sıralı dosya listesi
};

#endif // FILE_LIST_WIDGET_H
