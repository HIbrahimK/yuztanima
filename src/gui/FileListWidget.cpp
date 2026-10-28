#include "FileListWidget.h"
#include "ImageUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileInfo>
#include <QDir>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QDirIterator>

FileListWidget::FileListWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    setAcceptDrops(true);
}

void FileListWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(6);

    // Tablo bileşeni
    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({tr("Seç"), tr("Dosya Adı"), tr("Boyut"), tr("Durum")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    mainLayout->addWidget(m_table);

    // Butonlar ve sayaç
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(6);

    m_btnSelectAll = new QPushButton(tr("Tümünü Seç"), this);
    m_btnDeselectAll = new QPushButton(tr("Kaldır"), this);
    m_btnRemove = new QPushButton(tr("Sil"), this);
    m_btnClear = new QPushButton(tr("Temizle"), this);

    btnLayout->addWidget(m_btnSelectAll);
    btnLayout->addWidget(m_btnDeselectAll);
    btnLayout->addWidget(m_btnRemove);
    btnLayout->addWidget(m_btnClear);

    mainLayout->addLayout(btnLayout);

    m_lblCount = new QLabel(tr("Fotoğraf sayısı: 0"), this);
    m_lblCount->setStyleSheet(QStringLiteral("font-size: 11px; opacity: 0.85;"));
    mainLayout->addWidget(m_lblCount);

    // Ana Aksiyon Butonları (Başlat / Bitir)
    auto* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(6);

    m_btnStartBatch = new QPushButton(tr("▶️ İşlemi Başlat"), this);
    m_btnStartBatch->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #2e7d32; color: #ffffff; font-weight: bold; font-size: 12px; padding: 7px 12px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #388e3c; }"
        "QPushButton:pressed { background-color: #1b5e20; }"
        "QPushButton:disabled { background-color: rgba(128, 128, 128, 0.25); color: #888888; }"
    ));
    m_btnStartBatch->setEnabled(false);

    m_btnCancelBatch = new QPushButton(tr("⏹️ İşlemi Bitir"), this);
    m_btnCancelBatch->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #c62828; color: #ffffff; font-weight: bold; font-size: 12px; padding: 7px 12px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #d32f2f; }"
        "QPushButton:pressed { background-color: #b71c1c; }"
        "QPushButton:disabled { background-color: rgba(128, 128, 128, 0.25); color: #888888; }"
    ));
    m_btnCancelBatch->setEnabled(false);

    actionLayout->addWidget(m_btnStartBatch, 1);
    actionLayout->addWidget(m_btnCancelBatch, 1);
    mainLayout->addLayout(actionLayout);

    connect(m_btnSelectAll, &QPushButton::clicked, this, &FileListWidget::onSelectAllClicked);
    connect(m_btnDeselectAll, &QPushButton::clicked, this, &FileListWidget::onDeselectAllClicked);
    connect(m_btnRemove, &QPushButton::clicked, this, &FileListWidget::onRemoveSelectedClicked);
    connect(m_btnClear, &QPushButton::clicked, this, &FileListWidget::onClearAllClicked);
    connect(m_btnStartBatch, &QPushButton::clicked, this, &FileListWidget::startBatchRequested);
    connect(m_btnCancelBatch, &QPushButton::clicked, this, &FileListWidget::cancelBatchRequested);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &FileListWidget::onItemSelectionChanged);
    connect(m_table, &QTableWidget::itemChanged, this, &FileListWidget::onItemChanged);
}

void FileListWidget::addFiles(const QStringList& filePaths)
{
    const auto supportedExts = ImageUtils::supportedExtensions();

    m_table->setSortingEnabled(false);
    for (const auto& path : filePaths) {
        QFileInfo fi(path);
        if (fi.exists() && fi.isFile() && supportedExts.contains(fi.suffix().toLower())) {
            addSingleFile(fi.absoluteFilePath());
        }
    }
    m_table->setSortingEnabled(true);
    updateCounterLabel();
}

void FileListWidget::addDirectory(const QString& dirPath)
{
    const auto supportedExts = ImageUtils::supportedExtensions();
    QStringList nameFilters;
    for (const auto& ext : supportedExts) {
        nameFilters << QStringLiteral("*.%1").arg(ext);
    }

    QDirIterator it(dirPath, nameFilters, QDir::Files, QDirIterator::Subdirectories);
    QStringList found;
    while (it.hasNext()) {
        found << it.next();
    }
    addFiles(found);
}

void FileListWidget::addSingleFile(const QString& filePath)
{
    if (m_fileList.contains(filePath)) {
        return; // Zaten listede var
    }

    QFileInfo fi(filePath);
    int row = m_table->rowCount();
    m_table->insertRow(row);

    // 0: Checkbox
    auto* checkItem = new QTableWidgetItem();
    checkItem->setCheckState(Qt::Checked);
    checkItem->setData(Qt::UserRole, filePath); // Dosya tam yolu
    m_table->setItem(row, 0, checkItem);

    // 1: Dosya adı
    auto* nameItem = new QTableWidgetItem(fi.fileName());
    nameItem->setToolTip(filePath);
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    m_table->setItem(row, 1, nameItem);

    // 2: Dosya boyutu
    qint64 sizeKb = fi.size() / 1024;
    QString sizeStr = (sizeKb > 1024)
        ? QStringLiteral("%1 MB").arg(sizeKb / 1024.0, 0, 'f', 1)
        : QStringLiteral("%1 KB").arg(sizeKb);
    auto* sizeItem = new QTableWidgetItem(sizeStr);
    sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sizeItem->setFlags(sizeItem->flags() & ~Qt::ItemIsEditable);
    m_table->setItem(row, 2, sizeItem);

    // 3: Durum
    auto* statusItem = new QTableWidgetItem(tr("Hazır"));
    statusItem->setFlags(statusItem->flags() & ~Qt::ItemIsEditable);
    m_table->setItem(row, 3, statusItem);

    m_fileList.append(filePath);
}

QStringList FileListWidget::checkedFiles() const
{
    QStringList list;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto* item = m_table->item(row, 0);
        if (item && item->checkState() == Qt::Checked) {
            list.append(item->data(Qt::UserRole).toString());
        }
    }
    return list;
}

QStringList FileListWidget::allFiles() const
{
    return m_fileList;
}

void FileListWidget::updateFileStatus(const QString& filePath, const QString& status, int facesFound, bool isError)
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto* item = m_table->item(row, 0);
        if (item && item->data(Qt::UserRole).toString() == filePath) {
            auto* statusItem = m_table->item(row, 3);
            if (statusItem) {
                QString text = status;
                if (facesFound >= 0) {
                    text += QStringLiteral(" (%1 yüz)").arg(facesFound);
                }
                statusItem->setText(text);
                if (isError) {
                    statusItem->setForeground(QBrush(QColor(244, 67, 54)));
                } else if (status.contains(tr("Tamamlandı"))) {
                    statusItem->setForeground(QBrush(QColor(76, 175, 80)));
                }
            }
            break;
        }
    }
}

void FileListWidget::resetStatuses()
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto* statusItem = m_table->item(row, 3);
        if (statusItem) {
            statusItem->setText(tr("Hazır"));
            statusItem->setForeground(QBrush(palette().text().color()));
        }
    }
}

int FileListWidget::fileCount() const
{
    return m_table->rowCount();
}

void FileListWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void FileListWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void FileListWidget::dropEvent(QDropEvent* event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    QStringList filesToAdd;
    QStringList dirsToAdd;

    for (const auto& url : urls) {
        QString localPath = url.toLocalFile();
        QFileInfo fi(localPath);
        if (fi.isDir()) {
            dirsToAdd.append(localPath);
        } else if (fi.isFile()) {
            filesToAdd.append(localPath);
        }
    }

    if (!filesToAdd.isEmpty()) {
        addFiles(filesToAdd);
    }
    for (const auto& dir : dirsToAdd) {
        addDirectory(dir);
    }

    event->acceptProposedAction();
}

void FileListWidget::onSelectAllClicked()
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto* item = m_table->item(row, 0);
        if (item) item->setCheckState(Qt::Checked);
    }
    updateCounterLabel();
}

void FileListWidget::onDeselectAllClicked()
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto* item = m_table->item(row, 0);
        if (item) item->setCheckState(Qt::Unchecked);
    }
    updateCounterLabel();
}

void FileListWidget::onRemoveSelectedClicked()
{
    auto selectedRanges = m_table->selectedRanges();
    QList<int> rowsToDelete;

    for (const auto& range : selectedRanges) {
        for (int r = range.topRow(); r <= range.bottomRow(); ++r) {
            if (!rowsToDelete.contains(r)) {
                rowsToDelete.append(r);
            }
        }
    }

    std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());

    for (int r : rowsToDelete) {
        auto* item = m_table->item(r, 0);
        if (item) {
            m_fileList.removeAll(item->data(Qt::UserRole).toString());
        }
        m_table->removeRow(r);
    }

    updateCounterLabel();
}

void FileListWidget::onClearAllClicked()
{
    m_table->setRowCount(0);
    m_fileList.clear();
    updateCounterLabel();
}

void FileListWidget::onItemSelectionChanged()
{
    int currentRow = m_table->currentRow();
    if (currentRow >= 0 && currentRow < m_table->rowCount()) {
        auto* item = m_table->item(currentRow, 0);
        if (item) {
            emit fileSelected(item->data(Qt::UserRole).toString());
        }
    }
}

void FileListWidget::onItemChanged(QTableWidgetItem* item)
{
    if (item && item->column() == 0) {
        updateCounterLabel();
    }
}

void FileListWidget::updateCounterLabel()
{
    int total = m_table->rowCount();
    int checked = checkedFiles().size();
    m_lblCount->setText(tr("Toplam: %1 fotoğraf (%2 seçili)").arg(total).arg(checked));
    
    if (m_btnStartBatch) {
        m_btnStartBatch->setEnabled(!m_isProcessing && checked > 0);
    }
    if (m_btnCancelBatch) {
        m_btnCancelBatch->setEnabled(m_isProcessing);
    }

    emit fileCountChanged(total, checked);
}

void FileListWidget::setProcessingState(bool isProcessing)
{
    m_isProcessing = isProcessing;
    int checked = checkedFiles().size();

    if (m_btnStartBatch) {
        m_btnStartBatch->setEnabled(!m_isProcessing && checked > 0);
    }
    if (m_btnCancelBatch) {
        m_btnCancelBatch->setEnabled(m_isProcessing);
    }

    m_btnSelectAll->setEnabled(!m_isProcessing);
    m_btnDeselectAll->setEnabled(!m_isProcessing);
    m_btnRemove->setEnabled(!m_isProcessing);
    m_btnClear->setEnabled(!m_isProcessing);
    m_table->setEnabled(!m_isProcessing);
}
