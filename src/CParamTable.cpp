#include "CParamTable.h"

#include <QHeaderView>


CParamTable::CParamTable(QWidget* parent)
	: QTableWidget(parent)
{
	setColumnCount(3);
	setHorizontalHeaderLabels({ tr("Name"), tr("Value"), tr("Use") });
	horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
	setColumnWidth(2, 40);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
}


int CParamTable::AddRow(const QString& name, const QString& value, bool on)
{
	int row = rowCount();
	insertRow(row);

	setItem(row, 0, new QTableWidgetItem(name));
	setItem(row, 1, new QTableWidgetItem(value));

	auto checkItem = new QTableWidgetItem(1);
	checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
	checkItem->setCheckState(on ? Qt::Checked : Qt::Unchecked);
	setItem(row, 2, checkItem);

	setCurrentCell(row, 0);

	return row;
}


int CParamTable::AddRowIfNotExists(const QString& name, const QString& value, bool on)
{
	// Check if the parameter already exists (by name)
	for (int i = 0; i < rowCount(); ++i) {
		QTableWidgetItem* item = this->item(i, 0);
		if (item && item->text() == name) {
			// Already exists, update value and return row index
			QTableWidgetItem* valueItem = this->item(i, 1);
			if (valueItem) {
				valueItem->setText(value);
			}
			else {
				setItem(i, 1, new QTableWidgetItem(value));
			}
			setCurrentCell(i, 0);
			return i;
		}
	}

	// Add new row
	return AddRow(name, value, on);
}


bool CParamTable::DeleteRow(int row)
{
	if (row < 0 || row >= rowCount())
		return false;

	removeRow(row);
	return true;
}


bool CParamTable::DeleteCurrentRow()
{
	int row = currentRow();
	
	if (row >= 0) {
		removeRow(row);
		if (row < rowCount()) {
			setCurrentCell(row, 0);
		}
		return true;
	}

	return false;
}


int CParamTable::DeleteActiveRows()
{
	int count = 0;
	for (int i = 0; i < rowCount(); ++i) {
		QTableWidgetItem* item = this->item(i, 2);
		if (item->checkState() == Qt::Checked) {
			removeRow(i);
			i--;
			count++;
		}
	}

	return count;
}


int CParamTable::DeleteInactiveRows()
{
	int count = 0;
	for (int i = 0; i < rowCount(); ++i) {
		QTableWidgetItem* item = this->item(i, 2);
		if (item->checkState() == Qt::Unchecked) {
			removeRow(i);
			i--;
			count++;
		}
	}

	return count;
}


int CParamTable::DeleteActiveRows(const QString& name)
{
	int count = 0;
	for (int i = 0; i < rowCount(); ++i) {
		QTableWidgetItem* item = this->item(i, 2);
		if (item->checkState() == Qt::Checked && this->item(i, 0)->text() == name) {
			removeRow(i);
			i--;
			count++;
		}
	}

	return count;
}


bool CParamTable::IsActive(int row) const
{
	if (row < 0 || row >= rowCount())
		return false;

	QTableWidgetItem* item = this->item(row, 2);
	return item && item->checkState() == Qt::Checked;
}


bool CParamTable::SetActive(int row, bool on)
{
	if (row < 0 || row >= rowCount())
		return false;

	QTableWidgetItem* item = this->item(row, 2);
	if (item) {
		item->setCheckState(on ? Qt::Checked : Qt::Unchecked);
		return true;
	}

	return false;
}


int CParamTable::FindRow(const QString& name, const QString& value) const
{
	for (int i = 0; i < rowCount(); ++i) {
		QTableWidgetItem* nameItem = this->item(i, 0);
		QTableWidgetItem* valueItem = this->item(i, 1);
		if (nameItem && valueItem &&
			nameItem->text() == name && valueItem->text() == value) {
			return i; // Found
		}
	}
	return -1; // Not found
}


CParamTable::ParamList CParamTable::GetEnabledParams() const
{
	ParamList result;

	for (int row = 0; row < rowCount(); ++row) {
		auto item = this->item(row, 2);
		if (item && item->checkState() == Qt::Checked) {
			QString name = this->item(row, 0)->text();
			QString value = this->item(row, 1)->text();
			result.append({ name, value });
		}
	}

	return result;
}


// IO

void CParamTable::Store(QSettings& settings) const
{
	settings.beginWriteArray("Items", rowCount());
	for (int row = 0; row < rowCount(); ++row) {
		settings.setArrayIndex(row);
		settings.setValue("Key", item(row, 0) ? item(row, 0)->text() : "");
		settings.setValue("Value", item(row, 1) ? item(row, 1)->text() : "");
		settings.setValue("Active", item(row, 2) ? (item(row, 2)->checkState() == Qt::Checked) : false);
	}
	settings.endArray();
}


void CParamTable::Restore(QSettings& settings)
{
	setRowCount(0);

	int rows = settings.beginReadArray("Items");
	for (int row = 0; row < rows; ++row) {
		settings.setArrayIndex(row);
		auto key = settings.value("Key").toString();
		auto value = settings.value("Value").toString();
		bool active = settings.value("Active").toBool();
		AddRow(key, value, active);
	}
	settings.endArray();
}