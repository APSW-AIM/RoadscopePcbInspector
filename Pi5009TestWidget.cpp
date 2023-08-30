#include "Pi5009TestWidget.h"
#include "ui_Pi5009TestWidget.h"

#include "console.h"

Pi5009TestWidget::Pi5009TestWidget(QWidget *parent)
    : QWidget(parent)
    , m_pUi(new Ui::Pi5009TestWidget)
    , m_pConsole(new Console)

{
    m_pUi->setupUi(this);

    connect(m_pUi->frontCamButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::showFrontCam);

    connect(m_pUi->rearCamButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::showRearCam);

    connect(m_pUi->leftCamButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::showLeftCam);

    connect(m_pUi->rightCamButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::showRightCam);

    connect(m_pUi->quadViewButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::showQuadView);
}

Pi5009TestWidget::~Pi5009TestWidget()
{
    delete m_pConsole;
    delete m_pUi;
}

void Pi5009TestWidget::onSerialStateChanged(
    const QString& portName, SerialState serialState, const QString& extra)
{
    switch (serialState)
    {
    case SerialState::Connected:
        break;

    case SerialState::Disconnected:
        break;

    case SerialState::RxError:
        break;

    case SerialState::TxError:
        break;

    case SerialState::DoneBytesWritten:
        break;

    case SerialState::Timeout:
        break;

    default:
        break;
    }
}

void Pi5009TestWidget::onPutConsole(const QByteArray& data)
{
    m_pConsole->putData(data);
}

void Pi5009TestWidget::showFrontCam()
{
    emit writeSerialData("viewmode front\r");
}

void Pi5009TestWidget::showRearCam()
{
    emit writeSerialData("viewmode rear\r");
}

void Pi5009TestWidget::showLeftCam()
{
    emit writeSerialData("viewmode left\r");
}

void Pi5009TestWidget::showRightCam()
{
    emit writeSerialData("viewmode right\r");
}

void Pi5009TestWidget::showQuadView()
{
    emit writeSerialData("viewmode quad\r");
}
