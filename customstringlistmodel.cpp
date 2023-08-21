#include "customstringlistmodel.h"



void CustomStringListModel::prependItem(const QString& item)
{
    insertRow(0);
    setData(index(0), item);
}

void CustomStringListModel::appendItem(const QString& item)
{
    int row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    QStringListModel::insertRow(row);
    setData(index(row), item);
    endInsertRows();
}
