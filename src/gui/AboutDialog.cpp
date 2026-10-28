#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QDialogButtonBox>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Hakkında — YüzGizle"));
    setFixedSize(460, 320);
    setupUi();
}

void AboutDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    auto* topLayout = new QHBoxLayout();
    auto* lblIcon = new QLabel(this);
    lblIcon->setPixmap(QIcon(QStringLiteral(":/icons/app_icon.png")).pixmap(64, 64));
    lblIcon->setFixedSize(64, 64);

    auto* infoLayout = new QVBoxLayout();
    auto* lblTitle = new QLabel(tr("<h2>🎓 YüzGizle</h2><b>Sürüm:</b> 1.0.0"), this);
    auto* lblSub = new QLabel(tr("MEB KVKK Uyumlu Fotoğraf Yüz Gizleme Uygulaması"), this);
    lblSub->setStyleSheet(QStringLiteral("font-size: 12px; opacity: 0.85;"));
    infoLayout->addWidget(lblTitle);
    infoLayout->addWidget(lblSub);

    topLayout->addWidget(lblIcon);
    topLayout->addLayout(infoLayout, 1);
    mainLayout->addLayout(topLayout);

    auto* lblDesc = new QLabel(
        tr("<p>Bu uygulama, Milli Eğitim Bakanlığı'na bağlı okulların web sitelerine (*.meb.k12.tr) "
           "ve sosyal medya hesaplarına yüklenecek etkinlik fotoğraflarında öğrenci yüzlerini "
           "<b>6698 sayılı Kişisel Verilerin Korunması Kanunu (KVKK)</b> kapsamında toplu ve otomatik "
           "olarak gizlemek için geliştirilmiştir.</p>"
           "<p><b>Kullanılan Teknolojiler:</b> C++17, Qt 6, OpenCV 5 (YuNet DNN & Haar Cascade).</p>"), this);
    lblDesc->setWordWrap(true);
    lblDesc->setStyleSheet(QStringLiteral(
        "background: rgba(128, 128, 128, 0.12); "
        "border: 1px solid rgba(128, 128, 128, 0.25); "
        "border-radius: 6px; "
        "padding: 12px; "
        "font-size: 12px;"));
    mainLayout->addWidget(lblDesc);

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    btnBox->button(QDialogButtonBox::Ok)->setText(tr("Kapat"));
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    mainLayout->addWidget(btnBox);
}
