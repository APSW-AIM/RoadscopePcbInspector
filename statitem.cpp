#include "statitem.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QFile>

StatItem::StatItem()
    : count_total(0)
    , count_passed(0)
    , count_failed_noResponse(0)
    , count_failed_wrongRx(0)
    , count_failed_txError(0)
    , count_notTested(0)
{
}

void StatItem::clear()
{
    count_total = 0;
    count_passed = 0;
    count_failed_noResponse = 0;
    count_failed_wrongRx = 0;
    count_failed_txError = 0;
    count_notTested = 0;
}

void StatItem::writeStatItemToFile(const QString& fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QDataStream out(&file);
        out << (*this);
        file.close();
    }
    else
    {
        qDebug() << "Failed to open file for writing";
    }
}

StatItem StatItem::readFromFile(const QString& fileName)
{
    StatItem item;
    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly))
    {
        QDataStream in(&file);
        in >> item;
        file.close();
    }
    else
    {
        qDebug() << "Failed to open file for reading.";
    }

    return item;
}
