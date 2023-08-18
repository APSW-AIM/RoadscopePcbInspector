// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "horizontalbarwidget.h"

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QHorizontalBarSeries>
#include <QtCharts/QLegend>
#include <QtCharts/QValueAxis>
#include <QtGui/QBrush>

HorizontalBarWidget::HorizontalBarWidget(QWidget* parent)
    : ContentWidget(parent)
    , dataSet_txError(new QBarSet("TxError"))
    , dataSet_wrongRx(new QBarSet("WrongRx"))
    , dataSet_noResponse(new QBarSet("NoResponse"))
    , dataSet_passed(new QBarSet("Passed"))
    , axisX(new QValueAxis())
    , chart(new QChart())
{
    //![1]
    *dataSet_txError << 0;
    *dataSet_wrongRx << 0;
    *dataSet_noResponse << 0;
    *dataSet_passed << 0;
    //![1]

    //![2]
    auto series = new QHorizontalBarSeries;
    series->append(dataSet_txError);
    series->append(dataSet_passed);
    series->append(dataSet_noResponse);
    series->append(dataSet_wrongRx);

    //![2]

    series->setLabelsVisible(true);
    series->setLabelsPosition(QAbstractBarSeries::LabelsPosition::LabelsOutsideEnd);

    //![3]
    chart->addSeries(series);
    chart->setTitle("Statistics");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    //![3]

    chart->setMargins(QMargins{1, 1, 1, 1});

    chart->setBackgroundBrush(QBrush(Qt::gray));

    //![4]
    QStringList categories{"Count"};
    auto axisY = new QBarCategoryAxis;
    axisY->append(categories);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    axisX->applyNiceNumbers();
    //![4]

    //![5]
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    //![5]

    //![6]
    createDefaultChartView(chart);
    //![6]
}

void HorizontalBarWidget::SetData(const StatItem* item)
{
    dataSet_noResponse->replace(0, item->count_failed_noResponse);
    dataSet_wrongRx->replace(0, item->count_failed_wrongRx);
    dataSet_txError->replace(0, item->count_failed_txError);
    dataSet_passed->replace(0, item->count_passed);

    const auto maxVal = qMax(qMax(qMax(item->count_failed_noResponse, item->count_failed_txError),
                                  item->count_failed_wrongRx),
                             item->count_passed);

    axisX->setMax(((maxVal + 20) / 10) * 10);

    const auto sum = item->count_failed_noResponse + item->count_failed_txError
                     + item->count_failed_wrongRx + item->count_passed;

    chart->setTitle(tr("Statistics (total=%1)").arg(sum));
}
