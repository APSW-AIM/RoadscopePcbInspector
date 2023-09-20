#ifndef PI5009TESTWIDGET_H
#define PI5009TESTWIDGET_H

#include <QtWidgets/QWidget>
#include <QtGui/QPainterPath>

#include "SerialState.h"


QT_BEGIN_NAMESPACE
class QPlainTextEdit;
class QColor;
QT_END_NAMESPACE

namespace Ui {
class Pi5009TestWidget;
}


class Console;
class RenderArea;

class Pi5009TestWidget : public QWidget
{
    Q_OBJECT

private:
    const int DISPLAY_RESOL_WIDTH = 1920;
    const int DISPLAY_RESOL_HEIGHT = 1080;

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
    void showCabinCam();
    void showBackCam();
    void showLeftCam();
    void showRightCam();
    void showQuadView();
    void drawRectangle();
    void clearRectangle();
    void drawText();

    void showFrameBuffer();
    void hideFrameBuffer();
    void clearFrameBuffer();

    void colorChanged();


private:
    void putSerial(const QByteArray& data);

    QColor getSliderColor() const;


private:
    Ui::Pi5009TestWidget* m_pUi = nullptr;

    Console* m_pConsole = nullptr;
    RenderArea* m_pRectRenderArea = nullptr;
    QPlainTextEdit* m_pTextDisplay = nullptr;

    QPainterPath m_rectPath;
};

#endif // PI5009TESTWIDGET_H
