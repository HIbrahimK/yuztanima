#include "MainWindow.h"
#include "SettingsManager.h"
#include "ProcessSettings.h"
#include "BatchResult.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QStyleFactory>
#include <QDir>

int main(int argc, char* argv[])
{
    // High DPI desteği
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("YuzGizle"));
    app.setApplicationDisplayName(QStringLiteral("YüzGizle — MEB Fotoğraf Yüz Gizleme"));
    app.setOrganizationName(QStringLiteral("MEB"));
    app.setOrganizationDomain(QStringLiteral("meb.gov.tr"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    // Modern Fusion stili
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    // İş parçacıkları arası veri tipleri kaydı
    qRegisterMetaType<ProcessSettings>("ProcessSettings");
    qRegisterMetaType<BatchResult>("BatchResult");
    qRegisterMetaType<FileResult>("FileResult");

    // Çeviri yükleme
    QString lang = SettingsManager::instance().language();
    QTranslator translator;
    if (lang == QStringLiteral("en")) {
        if (translator.load(QStringLiteral(":/translations/yuzgizle_en.qm"))) {
            app.installTranslator(&translator);
        }
    }

    MainWindow window;
    window.show();

    return app.exec();
}
