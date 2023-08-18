#ifndef STATITEM_H
#define STATITEM_H

#include <QDataStream>
#include <QString>

#define STAT_ITEM_DEFAULT_FILE_NAME "statitem.dat"

class StatItem
{
public:
    StatItem();

public:
    int count_total;
    int count_passed;
    int count_failed_noResponse;
    int count_failed_wrongRx;
    int count_failed_txError;
    int count_notTested;

public:
    void clear();

    void writeStatItemToFile(const QString& fileName = STAT_ITEM_DEFAULT_FILE_NAME);

    static StatItem readFromFile(const QString& fileName = STAT_ITEM_DEFAULT_FILE_NAME);

public:
    friend QDataStream& operator<<(QDataStream& out, const StatItem& item)
    {
        out << item.count_total << item.count_passed << item.count_failed_noResponse
            << item.count_failed_wrongRx << item.count_failed_txError << item.count_notTested;
        return out;
    }

    friend QDataStream& operator>>(QDataStream& in, StatItem& item)
    {
        in >> item.count_total >> item.count_passed >> item.count_failed_noResponse
            >> item.count_failed_wrongRx >> item.count_failed_txError >> item.count_notTested;
        return in;
    }
};

#endif // STATITEM_H
