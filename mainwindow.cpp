#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "R9PcbTesterWidget.h"
//#include "ui_R9PcbTesterWidget.h"

#include "Pi5009TestWidget.h"

#include "settingsdialog.h"
//#include "ui_settingsdialog.h"


#include <QtCore/QTimer>
#include <QtCore/QStringListModel>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStackedWidget>

#include "config.h"


static constexpr std::chrono::milliseconds kWriteTimeout = std::chrono::milliseconds{25000};



MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_pMainWindow(new Ui::MainWindow)
    , m_pMainWindowContentsStack(new QStackedWidget(this))
    , m_pR9PcbTesterWidget(new R9PcbTesterWidget)
    , m_pPi5009TestWidget(new Pi5009TestWidget)
    , m_pStatusLabel(new QLabel)
    , m_pSettingsDialog(new SettingsDialog(this))
    , mCurrentSettings()
    , m_pSerialResponseTimer(new QTimer(this))
    , m_pSerialPort(new QSerialPort(this))
    , m_readBuffer()
{
    m_pMainWindow->setupUi(this);
    m_pMainWindow->statusbar->addWidget(m_pStatusLabel);

    m_pMainWindowContentsStack->addWidget(m_pR9PcbTesterWidget);
    m_pMainWindowContentsStack->addWidget(m_pPi5009TestWidget);

    setCentralWidget(m_pMainWindowContentsStack);

    initActionsConnections();

    m_pSerialResponseTimer->setSingleShot(true);

    connect(m_pSerialResponseTimer, &QTimer::timeout, this, &MainWindow::handleWriteTimeout);

    connect(m_pSerialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(m_pSerialPort, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(m_pSerialPort, &QSerialPort::bytesWritten, this, &MainWindow::handleBytesWritten);
}

MainWindow::~MainWindow()
{
    closeSerialPort();

    delete m_pSerialPort;
    delete m_pSerialResponseTimer;
    delete m_pSettingsDialog;
    delete m_pStatusLabel;

    delete m_pPi5009TestWidget;
    delete m_pR9PcbTesterWidget;
    delete m_pMainWindowContentsStack;
    delete m_pMainWindow;
};

void MainWindow::initActionsConnections()
{
    connect(m_pMainWindow->actionConnect,
            &QAction::triggered,
            m_pSettingsDialog,
            &SettingsDialog::show);
    connect(m_pMainWindow->actionDisconnect,
            &QAction::triggered,
            this,
            &MainWindow::closeSerialPort);
    connect(m_pMainWindow->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_pMainWindow->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_pMainWindow->actionAbout_QT, &QAction::triggered, qApp, &QApplication::aboutQt);

    connect(m_pSettingsDialog->m_ui->applyButton,
            &QPushButton::clicked,
            this,
            &MainWindow::openSerialPort);
}

void MainWindow::openSerialPort()
{
    mCurrentSettings = m_pSettingsDialog->settings();

    m_pSerialPort->setPortName(mCurrentSettings.name);
    m_pSerialPort->setBaudRate(mCurrentSettings.baudRate);
    m_pSerialPort->setDataBits(mCurrentSettings.dataBits);
    m_pSerialPort->setParity(mCurrentSettings.parity);
    m_pSerialPort->setStopBits(mCurrentSettings.stopBits);
    m_pSerialPort->setFlowControl(mCurrentSettings.flowControl);

    showStatusMessage(tr("Connecting to %1 : %2, %3, %4, %5, %6")
                          .arg(mCurrentSettings.name,
                               mCurrentSettings.stringBaudRate,
                               mCurrentSettings.stringDataBits,
                               mCurrentSettings.stringParity,
                               mCurrentSettings.stringStopBits,
                               mCurrentSettings.stringFlowControl));
    if (m_pSerialPort->open(QIODevice::ReadWrite))
    {
        m_pMainWindow->actionConnect->setEnabled(false);
        m_pMainWindow->actionDisconnect->setEnabled(true);


        changeProductType(mCurrentSettings.productType);


        emit serialStateChanged(m_pSerialPort->portName(), SerialState::Connected);

        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6 for %7")
                              .arg(mCurrentSettings.name,
                                   mCurrentSettings.stringBaudRate,
                                   mCurrentSettings.stringDataBits,
                                   mCurrentSettings.stringParity,
                                   mCurrentSettings.stringStopBits,
                                   mCurrentSettings.stringFlowControl,
                                   (mCurrentSettings.productType == SettingsDialog::Roadscope9) ? "Roadscope9" : "PI5009"));
    }
    // else
    // {
    //     QMessageBox::critical(this, tr("Error"), m_pSerialPort->errorString());
    //
    //     showStatusMessage(tr("Open error"));
    // }
}

void MainWindow::closeSerialPort()
{
    if (m_pSerialPort->isOpen())
    {
        m_pSerialPort->close();

        emit serialStateChanged(m_pSerialPort->portName(), SerialState::Disconnected);
    }

    m_pMainWindow->actionConnect->setEnabled(true);
    m_pMainWindow->actionDisconnect->setEnabled(false);

    showStatusMessage(tr("%1 Disconnected").arg(m_pSerialPort->portName()));
}

void MainWindow::writeSerialData(const QByteArray& data)
{
    const qint64 written = m_pSerialPort->write(data);

    m_pSerialPort->flush();

    if (written == data.size())
    {
        qInfo() << "written=" << written << ", data.size()=" << data.size() << ", data=" << data << " [OK]";

        m_bytesToWrite += written;
        m_pSerialResponseTimer->start(kWriteTimeout);
    }
    else
    {
        qInfo() << "written=" << written << ", data.size()=" << data.size() << ", data=" << data;

        const QString error = tr("Failed to write all data to port %1.\n"
                                 "Error: %2")
                                  .arg(m_pSerialPort->portName(), m_pSerialPort->errorString());

        emit serialStateChanged(m_pSerialPort->portName(), SerialState::TxError, error);

        QMessageBox::warning(this, tr("Warning"), error);
    }
}

void MainWindow::writeSerialDataDone()
{
    m_pSerialResponseTimer->stop();
}

void MainWindow::readData()
{
    const QByteArray crPattern = "\r\n";
    const QByteArray data = m_pSerialPort->readAll();

    m_readBuffer += data;

    int crIndex = m_readBuffer.indexOf(crPattern);
    int nCompleteCount = 0;
    while (crIndex != -1)
    {
        nCompleteCount++;
        const QByteArray completeStringBytes = m_readBuffer.left(crIndex);
        const QString completeString = QString::fromUtf8(completeStringBytes);
        const QString debugOutput = tr("Read(%1): ").arg(nCompleteCount) + completeString + "\n";
        qInfo() << debugOutput;

        emit putConsole(debugOutput.toUtf8());

        emit serialRxComplete(completeString);

        m_readBuffer.remove(0, crIndex + crPattern.size());

        crIndex = m_readBuffer.indexOf(crPattern);
    }

    if (m_readBuffer.length() > 1024)
    {
        emit putConsole("Error! readBuffer is too long");
        m_readBuffer.clear();
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError)
    {
        qInfo() << "QSerialPort Error: " << error << ", name=" << mCurrentSettings.name;

        QMessageBox::critical(this,
                              tr("Error"),
                              m_pSerialPort->errorString() + tr(" (%1)").arg(mCurrentSettings.name));

        // if (m_testInProgress)
        // {
        //     finishPcbTestWithFailed();
        // }
    }

    switch (error)
    {
    case QSerialPort::ResourceError:
        closeSerialPort();
        break;

    case QSerialPort::DeviceNotFoundError:
    case QSerialPort::PermissionError:
        showStatusMessage(tr("Failed to connect %1").arg(mCurrentSettings.name));
        break;

    case QSerialPort::NoError:
        break;

    default:
        showStatusMessage(tr("Serial Port Error: %1 (%2)").arg(mCurrentSettings.name, error));
        break;
    }
}


void MainWindow::handleBytesWritten(qint64 bytes)
{
    m_bytesToWrite -= bytes;

    if (m_bytesToWrite == 0)
    {
        emit serialStateChanged(m_pSerialPort->portName(), SerialState::DoneBytesWritten);


        // m_pTimer->stop();
    }
}


void MainWindow::handleWriteTimeout()
{
    const QString error = tr("Write operation timed out for port %1.\n"
                             "Error: %2")
                              .arg(m_pSerialPort->portName(), m_pSerialPort->errorString());

    emit serialStateChanged(m_pSerialPort->portName(), SerialState::Timeout, error);
}


void MainWindow::about()
{
    QMessageBox aboutMessageBox;
    aboutMessageBox.setIconPixmap(QPixmap(":images/aim_inspector_icon.png"));
    aboutMessageBox.setWindowTitle("About " PROJECT_NAME_STR);
    aboutMessageBox.setText(tr("%1 %2\n"
                               "2023 AIMatics APSW team\n"
                               "Build %3")
                                .arg(
                                    PROJECT_NAME_STR,
                                    PROJECT_VERSION_STR,
                                     __DATE__ " " __TIME__));

    aboutMessageBox.addButton("OK", QMessageBox::YesRole);
    aboutMessageBox.setStyleSheet("QLabel { min-width: 240px; text-align: left; }");

    aboutMessageBox.exec();
}

void MainWindow::changeProductType(SettingsDialog::ProductType productType)
{
    disconnect(m_serialStateChangedConnection);
    disconnect(m_serialRxCompleteConnection);
    disconnect(m_clearStatConnection);
    disconnect(m_writeSerialDataConnection);
    disconnect(m_writeSerialDataDoneConnection);
    disconnect(m_putConsoleConnection);

    //QtPrivate::FunctionPointer<
    switch (productType)
    {
    case SettingsDialog::Roadscope9:

        m_serialStateChangedConnection = connect(this, &MainWindow::serialStateChanged,
                                                 m_pR9PcbTesterWidget, &R9PcbTesterWidget::onSerialStateChanged);

        m_serialRxCompleteConnection = connect(this, &MainWindow::serialRxComplete,
                                               m_pR9PcbTesterWidget, &R9PcbTesterWidget::onSerialRxCompleteString);

        m_clearStatConnection = connect(m_pMainWindow->actionClear_Statistics_Data,
                &QAction::triggered,
                m_pR9PcbTesterWidget,
                &R9PcbTesterWidget::clearStatisticsData);

        m_writeSerialDataConnection = connect(m_pR9PcbTesterWidget, &R9PcbTesterWidget::writeSerialData,
                                              this, &MainWindow::writeSerialData);

        m_writeSerialDataDoneConnection = connect(m_pR9PcbTesterWidget, &R9PcbTesterWidget::writeSerialDataDone,
                                              this, &MainWindow::writeSerialDataDone);

        m_pMainWindowContentsStack->setCurrentWidget(m_pR9PcbTesterWidget);

        m_putConsoleConnection = connect(this, &MainWindow::putConsole,
                                        m_pR9PcbTesterWidget, &R9PcbTesterWidget::onPutConsole);

        break;

    case SettingsDialog::PI5009:

        m_serialStateChangedConnection = connect(this, &MainWindow::serialStateChanged,
                                                 m_pPi5009TestWidget, &Pi5009TestWidget::onSerialStateChanged);

        m_writeSerialDataConnection = connect(m_pPi5009TestWidget, &Pi5009TestWidget::writeSerialData,
                                              this, &MainWindow::writeSerialData);

        m_writeSerialDataDoneConnection = connect(m_pPi5009TestWidget, &Pi5009TestWidget::writeSerialDataDone,
                                                  this, &MainWindow::writeSerialDataDone);



        m_pMainWindowContentsStack->setCurrentWidget(m_pPi5009TestWidget);

        m_putConsoleConnection = connect(this, &MainWindow::putConsole,
                                         m_pPi5009TestWidget, &Pi5009TestWidget::onPutConsole);

        break;

    default:
        break;
    }

}

void MainWindow::showStatusMessage(const QString& message)
{
    m_pStatusLabel->setText(message);
}


