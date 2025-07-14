#include "CBookmarkManager.h"
#include "CRequestGUI.h"

#include <QMessageBox>
#include <QInputDialog>


CBookmarkManager::CBookmarkManager()
{
	// Constructor implementation
}


CBookmarkManager::~CBookmarkManager()
{
	// Destructor implementation
}


bool CBookmarkManager::AddNewBookmark(const CRequestGUI& requestUI, QSettings& settings)
{
	QString bookmarkName = QInputDialog::getText(nullptr, tr("Add to Bookmarks"),
		tr("Enter a name for the bookmark:"), QLineEdit::Normal, "");

	if (bookmarkName.isEmpty())
		return false;  // user canceled

	if (IsBookmarked(bookmarkName)) {
		auto result = QMessageBox::question(nullptr, tr("Bookmark Exists"),
			tr("A bookmark with this name already exists. Replace?"), 
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

		if (result != QMessageBox::Yes)
			return false;  // bookmark already exists
	}

	// test
	QString requestData = requestUI.StoreToString();
	m_bookmarks[bookmarkName] = requestData;

	return true;
}


bool CBookmarkManager::IsBookmarked(const QString& name) const
{
	return m_bookmarks.contains(name);
}


QStringList CBookmarkManager::GetBookmarks() const
{
	// Return a list of bookmark names
	return m_bookmarks.keys();
}


// IO

bool CBookmarkManager::Store(QSettings& settings) const
{
	// Store bookmarks to settings
	settings.beginGroup("Bookmarks");

	settings.setValue("Content", m_bookmarks);

	settings.endGroup();

	return true;
}


bool CBookmarkManager::Restore(QSettings& settings)
{
	// Restore bookmarks from settings
	settings.beginGroup("Bookmarks");
	
	m_bookmarks = settings.value("Content").toMap();

	settings.endGroup();

	return true;
}
