#include "BatchResultDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

BatchResultDialog::BatchResultDialog(const BatchResult& result, QWidget* parent)
    : QDialog(parent)
    , m_outputDir(result.outputDirectory)
{
    setWindowTitle(tr("İşlem Sonuç Raporu"));
    setMinimumSize(640, 480);
    setupUi(result);
}

void BatchResultDialog::setupUi(const BatchResult& result)
{
    auto* mainLayout = new QVBoxLayout(this);

    // İstatistik paneli
    auto* statsLayout = new QGridLayout();
    auto createStatCard = [](const QString& label, const QString& val, const QString& accentColor, const QString& bgRgba) {
        auto* w = new QWidget();
        w->setStyleSheet(QStringLiteral(
            "background: %1; border: 1px solid %2; border-radius: 8px; padding: 4px;"
        ).arg(bgRgba, accentColor));
        auto* l = new QVBoxLayout(w);
        l->setContentsMargins(6, 6, 6, 6);
        l->setSpacing(2);
        auto* lblVal = new QLabel(val);
        lblVal->setStyleSheet(QStringLiteral("font-size: 19px; font-weight: bold; color: %1;").arg(accentColor));
        lblVal->setAlignment(Qt::AlignCenter);
        auto* lblText = new QLabel(label);
        lblText->setStyleSheet(QStringLiteral("font-size: 11px; font-weight: 500; opacity: 0.85;"));
        lblText->setAlignment(Qt::AlignCenter);
        l->addWidget(lblVal);
        l->addWidget(lblText);
        return w;
    };

    statsLayout->addWidget(createStatCard(tr("Toplam Dosya"), QString::number(result.totalFiles), QStringLiteral("#2196f3"), QStringLiteral("rgba(33, 150, 243, 0.15)")), 0, 0);
    statsLayout->addWidget(createStatCard(tr("Başarılı"), QString::number(result.successCount), QStringLiteral("#4caf50"), QStringLiteral("rgba(76, 175, 80, 0.15)")), 0, 1);
    statsLayout->addWidget(createStatCard(tr("Hatalı"), QString::number(result.errorCount), QStringLiteral("#f44336"), QStringLiteral("rgba(244, 67, 54, 0.15)")), 0, 2);
    statsLayout->addWidget(createStatCard(tr("Yüz Yok"), QString::number(result.noFaceCount), QStringLiteral("#ff9800"), QStringLiteral("rgba(255, 152, 0, 0.15)")), 0, 3);
    statsLayout->addWidget(createStatCard(tr("Gizlenen Yüz"), QString::number(result.totalFacesHidden), QStringLiteral("#ab47bc"), QStringLiteral("rgba(171, 71, 188, 0.15)")), 0, 4);
    statsLayout->addWidget(createStatCard(tr("Geçen Süre"), QStringLiteral("%1 s").arg(result.elapsedSeconds, 0, 'f', 1), QStringLiteral("#78909c"), QStringLiteral("rgba(120, 144, 156, 0.15)")), 0, 5);

    mainLayout->addLayout(statsLayout);

    // Detay Tablosu
    auto* table = new QTableWidget(this);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({tr("Dosya"), tr("Durum"), tr("Gizlenen Yüz"), tr("Çıktı / Açıklama")});
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);

    table->setRowCount(result.fileResults.size());
    for (int i = 0; i < result.fileResults.size(); ++i) {
        const auto& f = result.fileResults[i];
        QFileInfo fi(f.filePath);

        auto* itemFile = new QTableWidgetItem(fi.fileName());
        table->setItem(i, 0, itemFile);

        auto* itemStatus = new QTableWidgetItem(f.success ? tr("Başarılı") : tr("Hata"));
        if (!f.success) {
            itemStatus->setForeground(QBrush(QColor(220, 50, 50)));
        } else {
            itemStatus->setForeground(QBrush(QColor(40, 160, 60)));
        }
        table->setItem(i, 1, itemStatus);

        auto* itemFaces = new QTableWidgetItem(QString::number(f.facesFound));
        itemFaces->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 2, itemFaces);

        QString detail = f.success ? f.outputPath : f.errorMessage;
        auto* itemDetail = new QTableWidgetItem(detail);
        table->setItem(i, 3, itemDetail);
    }
    mainLayout->addWidget(table, 1);

    // Butonlar
    auto* btnLayout = new QHBoxLayout();
    auto* btnOpenDir = new QPushButton(tr("📁 Çıktı Klasörünü Aç"), this);
    auto* btnClose = new QPushButton(tr("Kapat"), this);
    btnClose->setDefault(true);

    btnLayout->addWidget(btnOpenDir);
    btnLayout->addStretch();
    btnLayout->addWidget(btnClose);
    mainLayout->addLayout(btnLayout);

    connect(btnOpenDir, &QPushButton::clicked, this, &BatchResultDialog::onOpenOutputDir);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
}

void BatchResultDialog::onOpenOutputDir()
{
    if (!m_outputDir.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_outputDir));
    }
}
