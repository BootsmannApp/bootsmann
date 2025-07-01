#include "CMainGUI.h"

#include <QApplication>
#include <QWebEngineProfile>

int main(int argc, char *argv[])
{
	// Disable GPU and plugins for QtWebEngine to avoid issues with some systems
    //qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu --disable-plugins --disable-extensions --disable-features=NetworkServiceInProcess");

    QApplication a(argc, argv);
    a.setApplicationName("Bootsmann");

    //QWebEngineProfile* profile = QWebEngineProfile::defaultProfile();
    //profile->setHttpCacheMaximumSize(10 * 1024 * 1024); // 10 MB
    //profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    //profile->setCachePath(""); // In-memory only
    //profile->setPersistentStoragePath("");


    CMainGUI w;
    w.resize(1024,768);

    // restore settings
    w.RestoreSession();

    return a.exec();
}
