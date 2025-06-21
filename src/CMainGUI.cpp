#include "CMainGUI.h"
#include "ui_CMainGUI.h"

#include "CWorkspaceGUI.h"
#include "CRequestManager.h"

#include <QVBoxLayout>
#include <QDesktopServices>
#include <QMessageBox>
#include <QStandardPaths>


CMainGUI::CMainGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CMainGUI)
{
    ui->setupUi(this);

    // style
    setStyleSheet(
        "QWidget {"
            "font-family: Verdana;"
            "font-size: 10pt;"
            "color: #333;"
        "}\n"

        "QWidget:!enabled {"
            "color: #888;"
        "}\n"

        "QToolButton {"
            "min-height: 28px;"
            "min-width: 28px;"
        "}\n"

        "QTabBar::tab {"
            "min-width: 100px;"
		    "min-height: 30px;"
        "}\n"
    );

    // init
	qApp->setApplicationDisplayName(qApp->applicationName() + " " + qApp->applicationVersion());
    setWindowTitle(tr("Default Workspace"));

    connect(qApp, &QApplication::aboutToQuit, this, &CMainGUI::OnQuitApplication);

    // add default request manager
    auto reqMgr = new CRequestManager(this);

    // add default workspace
    m_activeWorkspace = new CWorkspaceGUI(*reqMgr, this);
	setCentralWidget(m_activeWorkspace);

    // restore settings
    RestoreSession();
}


CMainGUI::~CMainGUI()
{
    delete ui;
}


void CMainGUI::OnQuitApplication()
{
    StoreSession();
}


void CMainGUI::on_actionExit_triggered()
{
	qApp->quit();
}


void CMainGUI::on_actionAbout_triggered()
{
    QMessageBox aboutBox;
	aboutBox.setWindowTitle(tr("About the application"));
    aboutBox.setText(
        tr("<h2>%1</h2>"
            "<p>A minimalistic HTTP REST API test client</p>"
            "<p>License: <i>%2</i></p>"
            "<p><a href='https://github.com/BootsmannApp/bootsmann'>https://github.com/BootsmannApp/bootsmann</a></p>"
            "<hr>"
            "<p>If you like the application, please consider supporting its development with a donation:</p>"
			"<p><a href='https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=Z35EHHJ3729GG&source=url'>PayPal</a></p>"
            "<hr>"
            "<p>The application has been built using:</p>"
			"Qt framework &copy; <i>Qt Company</i> (<a href='https://qt.io'>https://qt.io</a>)<br>"
			"QHexView &copy; <i>Antonio Davide</i> (<a href='https://github.com/Dax89/QHexView'>https://github.com/Dax89/QHexView</a>)<br>"
			"QSourceHighliter &copy; <i>Waqar Ahmed</i> (<a href='https://github.com/Waqar144/QSourceHighlite'>https://github.com/Waqar144/QSourceHighlite</a>)<br>"
        )
        .arg(qApp->applicationDisplayName())
		.arg("MIT")
    );
	aboutBox.setIconPixmap(QPixmap(":/Icons/Anchor"));

    aboutBox.setDetailedText(
        tr("Bootsmann is a free and open-source HTTP REST API test client. "
           "It is designed to be simple and easy to use, while providing all the necessary features for testing APIs.") +
		"\n\n" + 
        tr("Platform: %1\n\n"
            "Started from: %2\n")
        .arg(qApp->platformName())
        .arg(qApp->applicationFilePath())
	);

	aboutBox.setFixedWidth(800);
	aboutBox.exec();
}


void CMainGUI::on_actionAboutQt_triggered()
{
    QApplication::aboutQt();
}


void CMainGUI::on_actionDonate_triggered()
{
    QUrl url("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=Z35EHHJ3729GG&source=url");
    QDesktopServices::openUrl(url);
}


void CMainGUI::on_actionNewRequest_triggered()
{
    m_activeWorkspace->CreateNewRequest();
}


void CMainGUI::on_actionSaveRequest_triggered()
{
    m_activeWorkspace->SaveCurrentRequest();
}


void CMainGUI::on_actionLoadRequest_triggered()
{
    m_activeWorkspace->LoadRequest();
}


// IO Session stuff

QString CMainGUI::GetConfigFileName() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/session.ini";
}


void CMainGUI::StoreSession()
{
    QString fileName = GetConfigFileName();
	QSettings settings(fileName, QSettings::IniFormat);

    // current workspace
    m_activeWorkspace->Store(settings);

	// geometry and state
    settings.beginGroup("Session");
    settings.setValue("Geometry", saveGeometry());
	settings.setValue("State", saveState());
    settings.endGroup();

    // store other settings if needed
    // ...
    
	settings.sync();
}


void CMainGUI::RestoreSession()
{
    QString fileName = GetConfigFileName();
    QSettings settings(fileName, QSettings::IniFormat);

	// restore geometry and state
    settings.beginGroup("Session");
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("State").toByteArray());
    settings.endGroup();

    // restore current workspace
    m_activeWorkspace->Restore(settings);
    
    // restore other settings if needed
	// ...
}

