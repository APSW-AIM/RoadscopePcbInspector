#ifndef CUSTOMSTRINGLISTMODEL_H
#define CUSTOMSTRINGLISTMODEL_H

#include <QStringListModel>

class CustomStringListModel : public QStringListModel
{
public:
    void prependItem(const QString& item);

    void appendItem(const QString& item);
};

#endif // CUSTOMSTRINGLISTMODEL_H
