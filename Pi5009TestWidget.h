#ifndef PI5009TESTWIDGET_H
#define PI5009TESTWIDGET_H

#include <QtWidgets/QWidget>

#include "SerialState.h"

namespace Ui {
class Pi5009TestWidget;
}


class Console;

class Pi5009TestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Pi5009TestWidget(QWidget *parent = nullptr);
    ~Pi5009TestWidget();


Q_SIGNALS:
    void writeSerialData(const QByteArray& data);
    void writeSerialDataDone();

public slots:
//    void clearStatisticsData();
//    void onSerialRxCompleteString(const QString& completeString);
    void onSerialStateChanged(const QString& portName, SerialState serialState, const QString& extra);
    void onPutConsole(const QByteArray& data);


private slots:
    void showFrontCam();
    void showRearCam();
    void showLeftCam();
    void showRightCam();
    void showQuadView();

private:
    Ui::Pi5009TestWidget* m_pUi = nullptr;

    Console* m_pConsole = nullptr;
};

#endif // PI5009TESTWIDGET_H
