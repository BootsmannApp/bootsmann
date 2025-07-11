#include "CWorkspaceGUI.h"
#include "ui_CWorkspaceGUI.h"

#include "CRequestGUI.h"

#include <QTabBar>
#include <QToolButton>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QTemporaryFile>
#include <QStandardPaths>


const int INFO_TAB_INDEX = 0;
const int LOG_TAB_INDEX = 1;

CWorkspaceGUI::CWorkspaceGUI(CRequestManager& reqMgr, QWidget *parent)
:   m_reqMgr(reqMgr),
    QWidget(parent),
    ui(new Ui::CWorkspaceGUI)
{
    ui->setupUi(this);

	ui->Tabs->setMovable(true);

    // INFO and LOG non-closable
    ui->Tabs->tabBar()->tabButton(INFO_TAB_INDEX, QTabBar::RightSide)->hide();
    ui->Tabs->tabBar()->tabButton(LOG_TAB_INDEX, QTabBar::RightSide)->hide();

    // temp: hide them
    ui->Tabs->tabBar()->setTabVisible(INFO_TAB_INDEX, false);
    ui->Tabs->tabBar()->setTabVisible(LOG_TAB_INDEX, false);

    // add tool button
    auto addButton = new QToolButton(this);
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    addButton->setToolTip(tr("Create new HTTP request"));
    addButton->setIcon(QIcon(":/Icons/Add"));
    addButton->setShortcut(Qt::CTRL | Qt::Key_N);

    connect(addButton, &QToolButton::clicked, this, [=](){
        AddRequestTab();
    });

    ui->Tabs->setCornerWidget(addButton, Qt::TopRightCorner);

    // handle closing tabs
    connect(ui->Tabs->tabBar(), &QTabBar::tabCloseRequested, this, &CWorkspaceGUI::CloseRequestTab);

	// tabs context menu
	ui->Tabs->setContextMenuPolicy(Qt::CustomContextMenu);

    // add default request
    AddRequestTab();
}


CWorkspaceGUI::~CWorkspaceGUI()
{
    delete ui;
}


bool CWorkspaceGUI::Store(QSettings& settings) const
{
    settings.beginGroup("Workspace");

    // save name
    settings.setValue("Name", m_name);

	// save the last used paths
	settings.setValue("LastLoadPath", m_lastLoadPath);
	settings.setValue("LastSavePath", m_lastSavePath);
    
	// save the current tab index
	settings.setValue("CurrentTabIndex", ui->Tabs->currentIndex());

	// save the number of tabs
	int tabCount = ui->Tabs->count();       

    // exclude INFO and LOG tabs
	int cnt = 0;
    for (int i = 0; i < tabCount; ++i) {
        auto requestUI = dynamic_cast<CRequestGUI*>(ui->Tabs->widget(i));
        if (requestUI) {
            if (requestUI->IsDefault()) {
                // skip default request
                continue;
			}

            settings.beginGroup(QString("Tab%1").arg(++cnt));
            requestUI->Store(settings);
			settings.setValue("TabTitle", ui->Tabs->tabText(i));
			settings.setValue("TabToolTip", ui->Tabs->tabToolTip(i));
            settings.endGroup();
        }
	}

    settings.setValue("TabCount", cnt);

	// finish saving
    settings.endGroup();
	return true;
}


bool CWorkspaceGUI::Restore(QSettings& settings)
{
	// clear existing tabs
    while (ui->Tabs->count() > 2) { // keep INFO and LOG tabs
        CloseRequestTab(2);
	}

    settings.beginGroup("Workspace");

    // restore name
    m_name = settings.value("Name").toString();

	// restore the last used paths
	m_lastLoadPath = settings.value("LastLoadPath", m_lastLoadPath).toString();
	m_lastSavePath = settings.value("LastSavePath", m_lastSavePath).toString();

    // restore the number of tabs
    int tabCount = settings.value("TabCount", 0).toInt();
    for (int i = 0; i < tabCount; ++i) {
        settings.beginGroup(QString("Tab%1").arg(i + 1));

        int index = AddRequestTab();
		auto requestUI = GetRequest(index);
        if (requestUI->Restore(settings)) {
			// set tab title and tooltip
			QString tabTitle = settings.value("TabTitle", tr("Request %1").arg(i + 1)).toString();
			QString tabToolTip = settings.value("TabToolTip", tabTitle).toString();
            ui->Tabs->setTabToolTip(index, tabToolTip);
            requestUI->Init();
        } else {
            //delete requestUI;
        }
        settings.endGroup();
    }

    // restore the current tab index
    int currentTabIndex = settings.value("CurrentTabIndex", -1).toInt();
    ui->Tabs->setCurrentIndex(currentTabIndex);

    settings.endGroup();

    if (ui->Tabs->count() ==2) {
        // if no request tabs, add a default one
        AddRequestTab();
	}

	return true;
}


int CWorkspaceGUI::CloneCurrentRequest()
{
    auto requestUI = GetCurrentRequest();
    if (!requestUI)
		return -1;  // no current request

    QTemporaryFile tempIni;
    tempIni.open();  // Creates a temp file
    QSettings settings(tempIni.fileName(), QSettings::IniFormat);
	requestUI->Store(settings);
	settings.sync();  // Ensure all data is written

	int newTabIndex = AddRequestTab(ui->Tabs->currentIndex() + 1);
	ui->Tabs->setCurrentIndex(newTabIndex);

	auto newRequestUI = GetCurrentRequest();
    if (!newRequestUI)
        return -1;  // no current request

	newRequestUI->Restore(settings);
	newRequestUI->Init();

    return newTabIndex;
}


bool CWorkspaceGUI::SaveCurrentRequest()
{
    auto requestUI = GetCurrentRequest();
    if (!requestUI)
        return false;  // no current request

	QString savePath = QFileDialog::getSaveFileName(this, tr("Save Request"), m_lastSavePath, tr("Bootsmann Requests (*.bor);;All Files (*)"));
	if (savePath.isEmpty())
		return false;

	m_lastSavePath = QFileInfo(savePath).absolutePath();

	QSettings settings(savePath, QSettings::IniFormat);
	return requestUI->Store(settings);
}


bool CWorkspaceGUI::RebaseCurrentRequest()
{
    auto requestUI = GetCurrentRequest();
    if (!requestUI)
        return false;  // no current request

	QString currentUrl = requestUI->GetRequestURL(true);

	QString newUrl = QInputDialog::getText(this, tr("Rebase Request"), 
        tr("Enter new URL for the request (omitted parts will not be changed):"), QLineEdit::Normal, 
        currentUrl);

    if (newUrl.isEmpty())
		return false;  // user canceled

	// set new URL
    return requestUI->RebaseURL(newUrl);
}


bool CWorkspaceGUI::RebaseOpenRequests()
{
    auto requestUI = GetCurrentRequest();
    if (!requestUI)
        return false;  // no current request

    QString currentUrl = requestUI->GetRequestURL(true);

    QString newUrl = QInputDialog::getText(this, tr("Rebase Open Requests"),
        tr("Enter new URL for all the open requests (omitted parts will not be changed):"), QLineEdit::Normal,
        currentUrl);

    if (newUrl.isEmpty())
        return false;  // user canceled

	int tabCount = ui->Tabs->count();
    for (int i = 0; i < tabCount; ++i) {
        auto requestUI = dynamic_cast<CRequestGUI*>(ui->Tabs->widget(i));
        if (requestUI) {
            requestUI->RebaseURL(newUrl);
        }
    }

    return true;
}


bool CWorkspaceGUI::LoadRequest()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Load Request"), m_lastLoadPath, tr("Bootsmann Requests (*.bor);;All Files (*)"));
    if (filePath.isEmpty())
        return false;

	m_lastLoadPath = QFileInfo(filePath).absolutePath();

    auto requestUI = GetCurrentRequest();
    if (!requestUI || !requestUI->IsDefault()) {
        // if no request or it is not default -> create a new tab
        AddRequestTab();
        requestUI = GetCurrentRequest();
    }

    if (!requestUI)
        return false;

    QSettings settings(filePath, QSettings::IniFormat);
    return requestUI->Restore(settings);
}


bool CWorkspaceGUI::HasRequests() const 
{
    // INFO and LOG tabs are always present
    return ui->Tabs->count() > 2; 
}


// Bookmarks

bool CWorkspaceGUI::BookmarkCurrentRequest()
{
    auto requestUI = GetCurrentRequest();
    if (!requestUI)
        return false;  // no current request

	QString bookmarkName = QInputDialog::getText(this, tr("Add to Bookmarks"),
		tr("Enter a name for the bookmark:"), QLineEdit::Normal, "");

    if (bookmarkName.isEmpty())
		return false;  // user canceled

    // simple for now
	AddBookmark(bookmarkName, *requestUI);

    return true;
}


bool CWorkspaceGUI::AddBookmark(const QString& name, const CRequestGUI& request)
{
    QString fileName = m_filePath.isEmpty() ? GetDefaultWorkspaceFileName() : m_filePath;
    QSettings settings(fileName, QSettings::IniFormat);

	settings.beginGroup("Bookmarks");
	settings.beginGroup(name);
	request.Store(settings);
	settings.endGroup();
	settings.endGroup();

	// update the bookmarks list

    return true;
}




// IO stuff

QString CWorkspaceGUI::GetDefaultWorkspaceFileName()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/Default.bow";
}


bool CWorkspaceGUI::SaveWorkspace()
{
    QString defaultPath = m_filePath.isEmpty() ?
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) :
        m_filePath;

    QString savePath = QFileDialog::getSaveFileName(this, tr("Save Workspace"), defaultPath,
        tr("Bootsmann Workspaces (*.bow);;All Files (*)"));

    if (savePath.isEmpty())
        return false;

    m_filePath = savePath;

	// Generate name from file name if not set
    if (m_name.isEmpty())
		m_name = QFileInfo(savePath).baseName();

    QSettings settings(savePath, QSettings::IniFormat);
    return this->Store(settings);
}


bool CWorkspaceGUI::LoadWorkspace()
{
    QString defaultPath = m_filePath.isEmpty() ?
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) :
        m_filePath;

    QString filePath = QFileDialog::getOpenFileName(this, tr("Load Workspace"), defaultPath,
        tr("Bootsmann Workspaces (*.bow);;All Files (*)"));

    if (filePath.isEmpty())
        return false;

    // store current workspace
    StoreSession();

    m_filePath = filePath;

    QSettings settings(filePath, QSettings::IniFormat);
    return this->Restore(settings);
}


void CWorkspaceGUI::StoreSession()
{
    if (m_filePath.size()) {
        QSettings settings(m_filePath, QSettings::IniFormat);
        this->Store(settings);
    }
}


CRequestGUI* CWorkspaceGUI::GetCurrentRequest()
{
    return dynamic_cast<CRequestGUI*>(ui->Tabs->currentWidget());
}


CRequestGUI* CWorkspaceGUI::GetRequest(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= ui->Tabs->count())
        return nullptr;

    return dynamic_cast<CRequestGUI*>(ui->Tabs->widget(tabIndex));
}


int CWorkspaceGUI::AddRequestTab(int tabIndex)
{
    auto requestUI = new CRequestGUI(m_reqMgr, this);

    if (tabIndex >= 0 && tabIndex < ui->Tabs->count()) {
        // insert at specified index
        ui->Tabs->insertTab(tabIndex, requestUI, tr("New Request"));
    }
    else
    tabIndex = ui->Tabs->addTab(requestUI, tr("New Request"));
    
    ui->Tabs->setCurrentIndex(tabIndex);
    requestUI->Init();

    connect(requestUI, &CRequestGUI::RequestTitleChanged, this, [=](const QString& title){
        int tabIndex = ui->Tabs->indexOf(requestUI);
        
        ui->Tabs->setTabToolTip(tabIndex, title);

        // truncate long titles
        if (title.size() > 20)
            ui->Tabs->setTabText(tabIndex, title.left(20) + "...");
        else
		    ui->Tabs->setTabText(tabIndex, title);
	});

    connect(requestUI, &CRequestGUI::RequestFailed, this, [=](){
		int tabIndex = ui->Tabs->indexOf(requestUI);

        ui->Tabs->tabBar()->setTabTextColor(tabIndex, Qt::red);
	});

    connect(requestUI, &CRequestGUI::RequestDone, this, [=](){
        int tabIndex = ui->Tabs->indexOf(requestUI);
        
        ui->Tabs->tabBar()->setTabTextColor(tabIndex, Qt::darkGreen);
    });

    connect(requestUI, &CRequestGUI::RequestCleared, this, [=]() {
        int tabIndex = ui->Tabs->indexOf(requestUI);

        ui->Tabs->tabBar()->setTabTextColor(tabIndex, Qt::black);
    });

    connect(requestUI, &CRequestGUI::RequestStarted, this, [=]() {
        int tabIndex = ui->Tabs->indexOf(requestUI);

        ui->Tabs->tabBar()->setTabTextColor(tabIndex, Qt::gray);
    });

    return tabIndex;
}


void CWorkspaceGUI::CloseRequestTab(int index)
{
    auto pageUI = ui->Tabs->widget(index);
    ui->Tabs->removeTab(index);
    pageUI->deleteLater();
}


// privates

void CWorkspaceGUI::on_Tabs_customContextMenuRequested(const QPoint& pos)
{
    int tabIndex = ui->Tabs->tabBar()->tabAt(pos);
    if (tabIndex == -1)
		return;     // means no tab at this position

	// if INFO or LOG tab, do not show context menu
    if (tabIndex == INFO_TAB_INDEX || tabIndex == LOG_TAB_INDEX)
        return;

	// set clicked tab as current
	ui->Tabs->setCurrentIndex(tabIndex);

    QMenu contextMenu(this);
    contextMenu.addAction(tr("New Request"), this, [this]() {
        int currentIndex = ui->Tabs->currentIndex();
		AddRequestTab(currentIndex + 1);    // next after current
    });

    contextMenu.addAction(tr("Duplicate Request"), this, [this]() {
        //int currentIndex = ui->Tabs->currentIndex();
		CloneCurrentRequest();
    });

    //contextMenu.addAction(tr("Load Request..."), this, [this]() {
    //    LoadRequest();
    //});

    contextMenu.addAction(tr("Save Request..."), this, [this]() {
        SaveCurrentRequest();
    });

    contextMenu.addSeparator();

    contextMenu.addAction(tr("Rebase Request..."), this, [this]() {
        RebaseCurrentRequest();
    });

    contextMenu.addSeparator();

    contextMenu.addAction(tr("Close Request"), this, [this]() {
        int currentIndex = ui->Tabs->currentIndex();
        CloseRequestTab(currentIndex);
    });

    contextMenu.addAction(tr("Close Other Requests"), this, [this]() {
        int openRequestCount = ui->Tabs->count() - 2 - 1;
        int btn = QMessageBox::question(this, tr("Close Requests"),
            tr("%1 requests are about to close, continue?").arg(openRequestCount),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (btn == QMessageBox::No)
            return;

        int currentIndex = ui->Tabs->currentIndex();
        for (int i = ui->Tabs->count() - 1; i >= 0; --i) {
            if (i != currentIndex && i > 1) { // keep INFO and LOG tabs
                CloseRequestTab(i);
            }
        }
	});


    contextMenu.exec(ui->Tabs->mapToGlobal(pos));
}


