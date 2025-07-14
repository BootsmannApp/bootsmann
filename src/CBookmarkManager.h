#pragma once

#include <QSettings>
#include <QVariantMap>

class CRequestGUI;


class CBookmarkManager : public QObject
{
	Q_OBJECT

public:
	CBookmarkManager();
	~CBookmarkManager();

	bool AddNewBookmark(const CRequestGUI& requestUI, QSettings& settings);
	bool IsBookmarked(const QString& name) const;
	QStringList GetBookmarks() const;

	// IO
	bool Store(QSettings& settings) const;
	bool Restore(QSettings& settings);

private:
	QVariantMap m_bookmarks; // name -> request data
};
