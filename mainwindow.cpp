#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "mainwindowcontents.h"
//#include "ui_mainwindowcontents.h"

#include "console.h"

#include "contentwidget.h"
#include "horizontalbarwidget.h"

#include "settingsdialog.h"
//#include "ui_settingsdialog.h"

#include "statitem.h"

#include <QDateTime>
#include <QFile>
#include <QTimer>
#include <QStringListModel>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>

static constexpr std::chrono::milliseconds kPcbTestTimeout = std::chrono::milliseconds{30000};
static constexpr std::chrono::milliseconds kWriteTimeout = std::chrono::milliseconds{25000};
static constexpr std::chrono::milliseconds kSerialTxRetryInterval = std::chrono::milliseconds{1000};
static constexpr std::chrono::milliseconds kProgressUpdateInterval = std::chrono::milliseconds{100};


static constexpr const char* kTestHistoryFileName = "history.dat";


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_pMainWindow(new Ui::MainWindow)
    , m_pMainWindowContents(new MainWindowContents)
    , m_pConsole(new Console)
    , m_pStatusLabel(new QLabel)
    , m_pSettingsDialog(new SettingsDialog(this))
    , m_pPcbTestTimer(new QTimer(this))
    , m_pSerialResponseTimer(new QTimer(this))
    , m_pSerialTxRetryTimer(new QTimer(this))
    , m_pUiUpdateTimer(new QTimer(this))
    , m_pSerialPort(new QSerialPort(this))
    , m_readBuffer()
    , m_pcbTestHistoryListModel()
    , m_pcbTestElapsedTimer()
    , m_pStatItem(new StatItem())
{
    m_pMainWindow->setupUi(this);
    m_pMainWindow->statusbar->addWidget(m_pStatusLabel);

    m_pMainWindowContents->m_pUi->verticalLayoutPcbConsole->addWidget(m_pConsole);
    m_pMainWindowContents->m_pUi->pcbTestHistoryListView->setModel(&m_pcbTestHistoryListModel);
    m_pMainWindowContents->m_pUi->pcbTestHistoryListView->setEditTriggers(
        QAbstractItemView::NoEditTriggers);

    m_pPcbTestStatsWidget = new HorizontalBarWidget(
        m_pMainWindowContents->m_pUi->pcbTestStatsWidget);
    m_pPcbTestStatsWidget->load();
    m_pPcbTestStatsWidget->resize(m_pMainWindowContents->m_pUi->pcbTestStatsWidget->size());
    m_pPcbTestStatsWidget->setVisible(true);

    setCentralWidget(m_pMainWindowContents);

    initActionsConnections();

    m_pSerialResponseTimer->setSingleShot(true);
    m_pPcbTestTimer->setSingleShot(true);

    connect(m_pSerialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(m_pPcbTestTimer, &QTimer::timeout, this, &MainWindow::handlePcbTestTimeout);
    connect(m_pSerialResponseTimer, &QTimer::timeout, this, &MainWindow::handleWriteTimeout);
    connect(m_pSerialTxRetryTimer, &QTimer::timeout, this, &MainWindow::handleSerialTxRetry);
    connect(m_pUiUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePcbTestProgress);
    connect(m_pSerialPort, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(m_pSerialPort, &QSerialPort::bytesWritten, this, &MainWindow::handleBytesWritten);

    loadHistoryFromFile(m_pcbTestHistoryListModel);

    *m_pStatItem = StatItem::readFromFile();
    m_pPcbTestStatsWidget->SetData(m_pStatItem);

    setPcbTestEnabled(false);
}

MainWindow::~MainWindow()
{
    closeSerialPort();

    delete m_pStatItem;
    delete m_pPcbTestStatsWidget;
    delete m_pSerialPort;
    delete m_pUiUpdateTimer;
    delete m_pSerialTxRetryTimer;
    delete m_pSerialResponseTimer;
    delete m_pPcbTestTimer;
    delete m_pSettingsDialog;
    delete m_pStatusLabel;
    delete m_pConsole;
    delete m_pMainWindowContents;
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
    connect(m_pMainWindow->actionClear_Statistics_Data,
            &QAction::triggered,
            this,
            &MainWindow::clearStatisticsData);
    connect(m_pMainWindowContents->m_pUi->pcbTestButton,
            &QPushButton::clicked,
            this,
            &MainWindow::beginPcbTest);
    connect(m_pSettingsDialog->m_ui->applyButton,
            &QPushButton::clicked,
            this,
            &MainWindow::openSerialPort);
}

void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_pSettingsDialog->settings();
    m_pSerialPort->setPortName(p.name);
    m_pSerialPort->setBaudRate(p.baudRate);
    m_pSerialPort->setDataBits(p.dataBits);
    m_pSerialPort->setParity(p.parity);
    m_pSerialPort->setStopBits(p.stopBits);
    m_pSerialPort->setFlowControl(p.flowControl);

    showStatusMessage(tr("Connecting to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name,
                               p.stringBaudRate,
                               p.stringDataBits,
                               p.stringParity,
                               p.stringStopBits,
                               p.stringFlowControl));
    if (m_pSerialPort->open(QIODevice::ReadWrite))
    {
        m_pMainWindow->actionConnect->setEnabled(false);
        m_pMainWindow->actionDisconnect->setEnabled(true);

        setPcbTestEnabled(true);

        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                              .arg(p.name,
                                   p.stringBaudRate,
                                   p.stringDataBits,
                                   p.stringParity,
                                   p.stringStopBits,
                                   p.stringFlowControl));
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
    setPcbTestEnabled(false);

    if (m_pSerialPort->isOpen())
    {
        m_pSerialPort->close();
    }

    m_pMainWindow->actionConnect->setEnabled(true);
    m_pMainWindow->actionDisconnect->setEnabled(false);

    showStatusMessage(tr("%1 Disconnected").arg(m_pSerialPort->portName()));
}

void MainWindow::writeData(const QByteArray& data)
{
    const qint64 written = m_pSerialPort->write(data);

    m_pSerialPort->flush();

    if (written == data.size())
    {
        m_bytesToWrite += written;
        m_pSerialResponseTimer->start(kWriteTimeout);
    }
    else
    {
        if (m_testInProgress == true)
        {
            finishPcbTest(PcbTestResult::Failed_TxError);
        }

        qInfo() << "written=" << written << ", data.size()=" << data.size() << ", data=" << data;

        const QString error = tr("Failed to write all data to port %1.\n"
                                 "Error: %2")
                                  .arg(m_pSerialPort->portName(), m_pSerialPort->errorString());

        showWriteError(error);
    }
}

void MainWindow::readData()
{
    const QByteArray data = m_pSerialPort->readAll();
    m_pConsole->putData(data);

    m_readBuffer += data;

    const QByteArray crPattern = "\r\n";
    const int crIndex = m_readBuffer.indexOf("\r\n");
    if (crIndex != -1)
    {
        QByteArray completeStringBytes = m_readBuffer.left(crIndex);
        QString completeString = QString::fromUtf8(completeStringBytes);
        qInfo() << "Read: " << completeString;

        if (m_testInProgress)
        {
            // TEST PCB

            if (completeString.contains("amboot>"))
            {
                finishPcbTest(PcbTestResult::Failed_WrongRx);
            }
            else if (completeString.contains("a:\\>"))
            {
                finishPcbTest(PcbTestResult::Passed);
            }
            else
            {
                //
            }

            m_testHasReponse = true;
        }
        else
        {
            // qInfo() << "readData: m_testInProgress=false";
        }

        m_readBuffer.remove(0, crIndex + crPattern.size());
    }
    else
    {
        qInfo() << "Read (X): " << data;
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    const SettingsDialog::Settings pSettings = m_pSettingsDialog->settings();

    if (error != QSerialPort::NoError)
    {
        qInfo() << "QSerialPort Error: " << error << ", name=" << pSettings.name;

        QMessageBox::critical(this,
                              tr("Error"),
                              m_pSerialPort->errorString() + tr(" (%1)").arg(pSettings.name));

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
        showStatusMessage(tr("Failed to connect %1").arg(pSettings.name));
        break;

    case QSerialPort::NoError:
        break;

    default:
        showStatusMessage(tr("Serial Port Error: %1 (%2)").arg(pSettings.name, error));
        break;
    }
}

void MainWindow::handleSerialTxRetry()
{
    sendPcbTestSerialTx();
}

void MainWindow::handleBytesWritten(qint64 bytes)
{
    m_bytesToWrite -= bytes;

    if (m_bytesToWrite == 0)
    {
        if (m_testInProgress)
        {
            m_testSerialConnected = true;
        }

        // m_pTimer->stop();
    }
}

void MainWindow::handlePcbTestTimeout()
{
    const QString error = tr("Pcb test timed out for port %1.\n"
                             "Error: %2")
                              .arg(m_pSerialPort->portName(), m_pSerialPort->errorString());

    if (m_testInProgress == true)
    {
        if (m_testHasReponse == true)
        {
            finishPcbTest(PcbTestResult::Failed_WrongRx);
        }
        else
        {
            finishPcbTest(PcbTestResult::Failed_NoResponse);
        }
    }
    else if (m_testSerialConnected == false)
    {
        showWriteError(error);
    }
}

void MainWindow::handleWriteTimeout()
{
    const QString error = tr("Write operation timed out for port %1.\n"
                             "Error: %2")
                              .arg(m_pSerialPort->portName(), m_pSerialPort->errorString());

    if (m_testInProgress == true)
    {
        finishPcbTest(PcbTestResult::Failed_NoResponse);
    }
    else if (m_testSerialConnected == false)
    {
        showWriteError(error);
    }
}

void MainWindow::updatePcbTestProgress()
{
    const auto& pcbTestProgressBar = m_pMainWindowContents->m_pUi->pcbTestProgressBar;
    const auto progressIncrease = pcbTestProgressBar->maximum()
                                  / (double) (kPcbTestTimeout / kProgressUpdateInterval);

    m_pcbTestProgressValue += progressIncrease;
    pcbTestProgressBar->setValue((int) m_pcbTestProgressValue);

    if (m_pcbTestProgressValue >= (pcbTestProgressBar->maximum() / 10.0))
    {
        checkPcbTestFinished();
    }
}

void MainWindow::beginPcbTest()
{
    if (m_testInProgress == true)
    {
        qInfo() << "Already in test progress";
        return;
    }

    m_pPcbTestTimer->start(kPcbTestTimeout);

    m_testInProgress = true;
    m_testSerialConnected = false;
    m_testHasReponse = false;
    m_pcbTestResult = PcbTestResult::None;

    const auto& pcbTestResultLabel = m_pMainWindowContents->m_pUi->pcbTestResultLabel;
    pcbTestResultLabel->setStyleSheet("QLabel { background : white; }");
    pcbTestResultLabel->setText("Testing...");

    m_pConsole->clear();

    setPcbTestEnabled(false);

    m_pcbTestProgressValue = 0;
    m_pUiUpdateTimer->start(kProgressUpdateInterval);
    m_pSerialTxRetryTimer->start(kSerialTxRetryInterval);

    m_pcbTestElapsedTimer.start();

    // put the test command
    sendPcbTestSerialTx();
}

void MainWindow::clearStatisticsData()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Confirmation");
    msgBox.setText("Do you want to clear statistics data?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    int ret = msgBox.exec();
    if (ret == QMessageBox::Yes)
    {
        m_pStatItem->clear();
        m_pStatItem->writeStatItemToFile();
        m_pPcbTestStatsWidget->SetData(m_pStatItem);

        m_pcbTestHistoryListModel.setStringList(QStringList());
        saveHistoryToFile(m_pcbTestHistoryListModel);
        m_pcbTestCount = 0;
    }
}

void MainWindow::about()
{
    QMessageBox aboutMessageBox;
    aboutMessageBox.setIconPixmap(QPixmap(":images/aim_inspector_icon.png"));
    aboutMessageBox.setWindowTitle("About RoadscopePcbInspector");
    aboutMessageBox.setText(tr("RoadscopePcbInspector v1.0.0\n"
                               "2023 AIMatics APSW team\n"
                               "Build %1")
                                .arg(__TIMESTAMP__));

    aboutMessageBox.addButton("OK", QMessageBox::YesRole);
    aboutMessageBox.setStyleSheet("QLabel { min-width: 240px; text-align: left; }");

    aboutMessageBox.exec();
}

void MainWindow::setPcbTestEnabled(bool enable)
{
    m_pMainWindowContents->m_pUi->pcbTestButton->setEnabled(enable);
}

void MainWindow::finishPcbTest(const PcbTestResult& result)
{
    assert(m_testInProgress == true);

    m_pcbTestResult = result;
}

void MainWindow::checkPcbTestFinished()
{
    const auto& pcbTestProgressBar = m_pMainWindowContents->m_pUi->pcbTestProgressBar;
    const auto& pcbTestResultLabel = m_pMainWindowContents->m_pUi->pcbTestResultLabel;

    bool testDone = false;

    switch (m_pcbTestResult)
    {
    case PcbTestResult::Failed_NoResponse:
        testDone = true;
        m_pStatItem->count_failed_noResponse++;
        pcbTestResultLabel->setStyleSheet("QLabel { background : red; }");
        pcbTestResultLabel->setText("FAILED");
        break;

    case PcbTestResult::Failed_WrongRx:
        testDone = true;
        m_pStatItem->count_failed_wrongRx++;
        pcbTestResultLabel->setStyleSheet("QLabel { background : red; }");
        pcbTestResultLabel->setText("FAILED");
        break;

    case PcbTestResult::Passed:
        testDone = true;
        m_pStatItem->count_passed++;
        pcbTestResultLabel->setStyleSheet("QLabel { background : green; }");
        pcbTestResultLabel->setText("PASSED");
        break;

    case PcbTestResult::Failed_TxError:
        testDone = true;
        m_pStatItem->count_failed_txError++;
        pcbTestResultLabel->setStyleSheet("QLabel { background : yellow; }");
        pcbTestResultLabel->setText("ERROR");
        break;

    case PcbTestResult::None:
        break;

    default:
        pcbTestResultLabel->setStyleSheet("QLabel { background : yellow; }");
        break;
    }

    m_pStatItem->count_total++;

    if (testDone)
    {
        m_testInProgress = false;
        m_pUiUpdateTimer->stop();
        m_pSerialTxRetryTimer->stop();
        m_pSerialResponseTimer->stop();
        m_pPcbTestTimer->stop();

        m_pcbTestCount++;

        m_pPcbTestStatsWidget->SetData(m_pStatItem);
        m_pStatItem->writeStatItemToFile();

        pcbTestProgressBar->setValue(pcbTestProgressBar->maximum());

        setPcbTestEnabled(true);

        const QDateTime currentDateTime = QDateTime::currentDateTime();

        m_pcbTestHistoryListModel.appendItem(
            tr("[%1 (%2)] TEST %3: %4 in %5ms (%6)")
                .arg(currentDateTime.toString("yyyy-MM-dd hh:mm:ss"),
                     currentDateTime.timeZoneAbbreviation(),
                     QString::number(m_pcbTestCount),
                     QMetaEnum::fromType<PcbTestResult>().valueToKey(m_pcbTestResult),
                     QString::number(m_pcbTestElapsedTimer.elapsed()),
                     m_pSerialPort->portName()));

        saveHistoryToFile(m_pcbTestHistoryListModel);

        m_pMainWindowContents->m_pUi->pcbTestHistoryListView->scrollToBottom();
    }
}

void MainWindow::showStatusMessage(const QString& message)
{
    m_pStatusLabel->setText(message);
}

void MainWindow::showWriteError(const QString& message)
{
    QMessageBox::warning(this, tr("Warning"), message);
}

void MainWindow::sendPcbTestSerialTx()
{
    writeData("\r\n");
    //writeData("show ptb\n");
}

void MainWindow::saveHistoryToFile(QStringListModel& listModel)
{
    QFile file(kTestHistoryFileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QStringList dataList = listModel.stringList();
        QTextStream stream(&file);
        for (const QString& item : dataList)
        {
            stream << item << Qt::endl;
        }
        file.close();
    }
}

void MainWindow::loadHistoryFromFile(QStringListModel& listModel)
{
    QFile file(kTestHistoryFileName);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QStringList dataList;
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            QString str = stream.readLine();
            dataList.append(str);
        }
        file.close();
        listModel.setStringList(dataList);

        m_pcbTestCount = dataList.count();
    }

    m_pMainWindowContents->m_pUi->pcbTestHistoryListView->scrollToBottom();
}
