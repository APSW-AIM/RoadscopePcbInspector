#ifndef CUSTOMSTRINGLISTMODEL_H
#define CUSTOMSTRINGLISTMODEL_H

#include <QStringListModel>

class CustomStringListModel : public QStringListModel
{
public:
    void prependItem(const QString& item)
    {
        insertRow(0);
        setData(index(0), item);
    }

    void appendItem(const QString& item)
    {
        int row = rowCount();
        beginInsertRows(QModelIndex(), row, row);
        QStringListModel::insertRow(row);
        setData(index(row), item);
        endInsertRows();
    }
};

#endif // CUSTOMSTRINGLISTMODEL_H
