// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef HORIZONTALBARWIDGET_H
#define HORIZONTALBARWIDGET_H

#include "contentwidget.h"
#include "statitem.h"

#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>

QT_FORWARD_DECLARE_CLASS(QBarSet)
QT_FORWARD_DECLARE_CLASS(QValueAxis)
QT_FORWARD_DECLARE_CLASS(QChart)

class HorizontalBarWidget : public ContentWidget
{
    Q_OBJECT
public:
    explicit HorizontalBarWidget(QWidget* parent = nullptr);

public:
    void SetData(const StatItem* item);

private:
    QBarSet* dataSet_txError;
    QBarSet* dataSet_wrongRx;
    QBarSet* dataSet_noResponse;
    QBarSet* dataSet_passed;

    QValueAxis* axisX;

    QChart* chart;
};

#endif
