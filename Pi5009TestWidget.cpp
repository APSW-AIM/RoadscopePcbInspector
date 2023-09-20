#include "Pi5009TestWidget.h"
#include "ui_Pi5009TestWidget.h"

#include "console.h"
#include "RenderArea.h"


#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtGui/QPalette>

Pi5009TestWidget::Pi5009TestWidget(QWidget *parent)
    : QWidget(parent)
    , m_pUi(new Ui::Pi5009TestWidget)
    , m_pConsole(new Console)
{
    m_pUi->setupUi(this);

    m_pUi->verticalLayoutConsole->addWidget(m_pConsole);

    connect(m_pUi->cabinCamButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::showCabinCam);

    connect(m_pUi->backCamButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::showBackCam);

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

    connect(m_pUi->drawRectangleButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::drawRectangle);

    connect(m_pUi->clearRectangleButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::clearRectangle);

    connect(m_pUi->drawTextButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::drawText);

    connect(m_pUi->showFbButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::showFrameBuffer);

    connect(m_pUi->hideFbButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::hideFrameBuffer);

    connect(m_pUi->clearFbButton,
            &QPushButton::clicked,
            this,
            &Pi5009TestWidget::clearFrameBuffer);

    connect(m_pUi->redColorSlider,
            &QSlider::valueChanged,
            this,
            &Pi5009TestWidget::colorChanged);

    connect(m_pUi->greenColorSlider,
            &QSlider::valueChanged,
            this,
            &Pi5009TestWidget::colorChanged);

    connect(m_pUi->blueColorSlider,
            &QSlider::valueChanged,
            this,
            &Pi5009TestWidget::colorChanged);


    m_rectPath.moveTo(20.0, 30.0);
    m_rectPath.lineTo(80.0, 30.0);
    m_rectPath.lineTo(80.0, 70.0);
    m_rectPath.lineTo(20.0, 70.0);
    m_rectPath.closeSubpath();

    m_pRectRenderArea = new RenderArea(m_rectPath);
    m_pRectRenderArea->setFillRule(Qt::FillRule::WindingFill);
    m_pRectRenderArea->setPenColor(getSliderColor());
    m_pRectRenderArea->setFillGradient(QColorConstants::Transparent, QColorConstants::Transparent);


    m_pUi->rectMdiArea->addSubWindow(m_pRectRenderArea);

    const auto rectSubWindow = m_pUi->rectMdiArea->currentSubWindow();


    rectSubWindow->setWindowFlags(rectSubWindow->windowFlags() & ~(Qt::WindowMinMaxButtonsHint  | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint ));
    rectSubWindow->setMaximumSize(m_pUi->rectMdiArea->maximumSize());
    rectSubWindow->move(100, 100);
    rectSubWindow->setWindowTitle("Rectangle");

    m_pTextDisplay = new QPlainTextEdit(this);

    m_pUi->textMdiArea->addSubWindow(m_pTextDisplay);
    m_pTextDisplay->setFixedHeight(50);
    m_pTextDisplay->setPlainText("Pedestrian Warning!");

    const auto textSubWindow = m_pUi->textMdiArea->currentSubWindow();
    textSubWindow->setWindowFlags(textSubWindow->windowFlags() & ~(Qt::WindowMinMaxButtonsHint  | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint ));
    textSubWindow->setMaximumSize(m_pUi->textMdiArea->maximumSize());
    textSubWindow->move(10, 10);
    textSubWindow->setWindowTitle("Text");
    textSubWindow->resize(160, 100);
    textSubWindow->setFixedHeight(100);
}

Pi5009TestWidget::~Pi5009TestWidget()
{
    delete m_pTextDisplay;
    delete m_pRectRenderArea;
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

void Pi5009TestWidget::putSerial(const QByteArray& data)
{
    m_pConsole->putData("> " + data);
    emit writeSerialData(data);
}

QColor Pi5009TestWidget::getSliderColor() const
{
#if 0
    qInfo() << "getSliderColor: R=" << m_pUi->redColorSlider->value()
        << ", G=" << m_pUi->greenColorSlider->value()
        << ", B=" << m_pUi->blueColorSlider->value();
#endif
    return QColor(qRgb(m_pUi->redColorSlider->value(), m_pUi->greenColorSlider->value(), m_pUi->blueColorSlider->value()));
}

void Pi5009TestWidget::showCabinCam()
{
    putSerial("viewmode cabin\r");
}

void Pi5009TestWidget::showBackCam()
{
    putSerial("viewmode back\r");
}

void Pi5009TestWidget::showLeftCam()
{
    putSerial("viewmode left\r");
}

void Pi5009TestWidget::showRightCam()
{
    putSerial("viewmode right\r");
}

void Pi5009TestWidget::showQuadView()
{
    putSerial("viewmode quad\r");
}

void Pi5009TestWidget::drawRectangle()
{
    const double widthRatio = (double)DISPLAY_RESOL_WIDTH / m_pUi->rectMdiArea->width();
    const double heightRatio = (double)DISPLAY_RESOL_HEIGHT / m_pUi->rectMdiArea->height();
    const auto subWindow = m_pUi->rectMdiArea->currentSubWindow();
    const auto subWindowPos = subWindow->pos();
    const auto subWindowWidth = subWindow->width();
    const auto subWindowHeight = subWindow->height();

    const int x = (int)(subWindowPos.x() * widthRatio);
    const int y = (int)(subWindowPos.y() * heightRatio);
    const int w = (int)(subWindowWidth * widthRatio);
    const int h = (int)(subWindowHeight * heightRatio);
    const int bFill = 0;
    const int thkickness = 7;
    const unsigned int color = (getSliderColor().rgba() & 0xFFFFFF);
    const int tranparancy = 255;
    QString rectangleTemplate = "rectangle %1 %2 %3 %4 %5 %6 %7 %8\r";
    QString rectangleStr = rectangleTemplate
                               .arg(x)
                               .arg(y)
                               .arg(w)
                               .arg(h)
                               .arg(bFill)
                               .arg(thkickness)
                               .arg(color)
                               .arg(tranparancy);

    putSerial(rectangleStr.toUtf8());
}


void Pi5009TestWidget::clearRectangle()
{
    const double widthRatio = (double)DISPLAY_RESOL_WIDTH / m_pUi->rectMdiArea->width();
    const double heightRatio = (double)DISPLAY_RESOL_HEIGHT / m_pUi->rectMdiArea->height();
    const auto subWindow = m_pUi->rectMdiArea->currentSubWindow();
    const auto subWindowPos = subWindow->pos();
    const auto subWindowWidth = subWindow->width();
    const auto subWindowHeight = subWindow->height();

    const int x = (int)(subWindowPos.x() * widthRatio);
    const int y = (int)(subWindowPos.y() * heightRatio);
    const int w = (int)(subWindowWidth * widthRatio);
    const int h = (int)(subWindowHeight * heightRatio);
    QString rectangleTemplate = "clearrect %1 %2 %3 %4\r";
    QString rectangleStr = rectangleTemplate
                               .arg(x)
                               .arg(y)
                               .arg(w)
                               .arg(h);

    putSerial(rectangleStr.toUtf8());
}

void Pi5009TestWidget::drawText()
{
    const double widthRatio = (double)DISPLAY_RESOL_WIDTH / m_pUi->textMdiArea->width();
    const double heightRatio = (double)DISPLAY_RESOL_HEIGHT / m_pUi->textMdiArea->height();
    const auto subWindow = m_pUi->textMdiArea->currentSubWindow();
    const auto subWindowPos = subWindow->pos();
    const auto subWindowWidth = subWindow->width();
//    const auto subWindowHeight = subWindow->height();

    const auto text = m_pTextDisplay->toPlainText();
    const int x = (int)(subWindowPos.x() * widthRatio);
    const int y = (int)(subWindowPos.y() * heightRatio);
    const int scale = (int)(7 * ((double)subWindowWidth/ m_pUi->textMdiArea->width()));
    const unsigned int color_bg = 0xFF00FF; // CHROMAKEY_COLOR
    const unsigned int color_pen = (getSliderColor().rgba() & 0xFFFFFF);
    QString rectangleTemplate = "puttext %1 %2 %3 %4 %5 %6\r";
    QString rectangleStr = rectangleTemplate
                               .arg(x)
                               .arg(y)
                               .arg(scale)
                               .arg(color_bg)
                               .arg(color_pen)
                               .arg(text);


    putSerial(rectangleStr.toUtf8());
}


void Pi5009TestWidget::showFrameBuffer()
{
    putSerial("fb show\r");
}

void Pi5009TestWidget::hideFrameBuffer()
{
    putSerial("fb hide\r");
}

void Pi5009TestWidget::clearFrameBuffer()
{
    putSerial("fb clear\r");
}

void Pi5009TestWidget::colorChanged()
{
    const int color = getSliderColor().rgba();
    m_pRectRenderArea->setPenColor(color);
}

