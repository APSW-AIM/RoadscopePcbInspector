#include "R9PcbTesterWidget.h"

#include <QtCore/QTimer>
#include <QtWidgets/QMessageBox>
#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtCore/QMetaEnum>

#include "console.h"
#include "statitem.h"
#include "horizontalbarwidget.h"

static constexpr std::chrono::milliseconds kPcbTestTimeout = std::chrono::milliseconds{30000};
static constexpr std::chrono::milliseconds kProgressUpdateInterval = std::chrono::milliseconds{100};
static constexpr std::chrono::milliseconds kSerialTxRetryInterval = std::chrono::milliseconds{1000};

static constexpr const char* kTestHistoryFileName = "history.dat";

R9PcbTesterWidget::R9PcbTesterWidget(QWidget* parent)
    : QWidget(parent)
    , m_pUi(new Ui::R9PcbTesterWidget)
    , m_pConsole(new Console(this))
    , m_pPcbTestTimer(new QTimer(this))
    , m_pSerialTxRetryTimer(new QTimer(this))
    , m_pUiUpdateTimer(new QTimer(this))
    , m_pcbTestHistoryListModel()
    , m_pcbTestElapsedTimer()
    , m_pStatItem(new StatItem())

{
    m_pUi->setupUi(this);


    m_pPcbTestStatsWidget = new HorizontalBarWidget(m_pUi->pcbTestStatsWidget);
    m_pPcbTestStatsWidget->load();
    m_pPcbTestStatsWidget->resize(m_pUi->pcbTestStatsWidget->size());
    m_pPcbTestStatsWidget->setVisible(true);



    m_pUi->verticalLayoutPcbConsole->addWidget(m_pConsole);
    m_pUi->pcbTestHistoryListView->setModel(&m_pcbTestHistoryListModel);
    m_pUi->pcbTestHistoryListView->setEditTriggers(
        QAbstractItemView::NoEditTriggers);


    m_pPcbTestTimer->setSingleShot(true);


    connect(m_pPcbTestTimer, &QTimer::timeout, this, &R9PcbTesterWidget::handlePcbTestTimeout);
    connect(m_pUiUpdateTimer, &QTimer::timeout, this, &R9PcbTesterWidget::updatePcbTestProgress);
    connect(m_pSerialTxRetryTimer, &QTimer::timeout, this, &R9PcbTesterWidget::handleSerialTxRetry);

    connect(m_pUi->pcbTestButton,
            &QPushButton::clicked,
            this,
            &R9PcbTesterWidget::beginPcbTest);

    loadHistoryFromFile(m_pcbTestHistoryListModel);

    (*m_pStatItem) = StatItem::readFromFile();
    m_pPcbTestStatsWidget->SetData(m_pStatItem);

    setPcbTestEnabled(false);
}

R9PcbTesterWidget::~R9PcbTesterWidget()
{

    delete m_pStatItem;
    delete m_pPcbTestStatsWidget;

    delete m_pUiUpdateTimer;
    delete m_pSerialTxRetryTimer;


    delete m_pPcbTestTimer;

    delete m_pConsole;
    delete m_pUi;
}


void R9PcbTesterWidget::updatePcbTestProgress()
{
    const auto& pcbTestProgressBar = m_pUi->pcbTestProgressBar;
    const auto progressIncrease = pcbTestProgressBar->maximum()
                                  / (double) (kPcbTestTimeout / kProgressUpdateInterval);

    m_pcbTestProgressValue += progressIncrease;
    pcbTestProgressBar->setValue((int) m_pcbTestProgressValue);

    if (m_pcbTestProgressValue >= (pcbTestProgressBar->maximum() / 10.0))
    {
        checkPcbTestFinished();
    }
}

void R9PcbTesterWidget::beginPcbTest()
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

    const auto& pcbTestResultLabel = m_pUi->pcbTestResultLabel;
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

void R9PcbTesterWidget::handleSerialTxRetry()
{
    sendPcbTestSerialTx();
}

void R9PcbTesterWidget::setPcbTestEnabled(bool enable)
{
    m_pUi->pcbTestButton->setEnabled(enable);
}

void R9PcbTesterWidget::finishPcbTest(const PcbTestResult& result)
{
    assert(m_testInProgress == true);

    m_pcbTestResult = result;
}

void R9PcbTesterWidget::checkPcbTestFinished()
{
    const auto& pcbTestProgressBar = m_pUi->pcbTestProgressBar;
    const auto& pcbTestResultLabel = m_pUi->pcbTestResultLabel;

    bool testDone = false;

    switch (m_pcbTestResult)
    {
    case PcbTestResult::Failed_NoResponse:
        testDone = true;
        m_pStatItem->count_failed_noResponse++;
        pcbTestResultLabel->setStyleSheet("QLabel { background : red; }");
        pcbTestResultLabel->setText("FAILED");
        break;

    case PcbTestResult::Failed_WrongRx_R9:
    case PcbTestResult::Failed_WrongRx_R8:
        testDone = true;
        m_pStatItem->count_failed_wrongRx++;
        pcbTestResultLabel->setStyleSheet("QLabel { background : red; }");
        pcbTestResultLabel->setText("FAILED");
        break;

    case PcbTestResult::Passed_R9:
    case PcbTestResult::Passed_R8:
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

        m_pPcbTestTimer->stop();

        emit writeSerialDataDone();

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
                     mSerialPortName));

        saveHistoryToFile(m_pcbTestHistoryListModel);

        m_pUi->pcbTestHistoryListView->scrollToBottom();
    }
}


void R9PcbTesterWidget::clearStatisticsData()
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

void R9PcbTesterWidget::onSerialRxCompleteString(const QString& completeString)
{

    if (m_testInProgress)
    {
        // TEST PCB

        if (completeString.contains("amboot>"))
        {
            finishPcbTest(PcbTestResult::Failed_WrongRx_R9);
        }
        else if (completeString.contains("a:\\>"))
        {
            finishPcbTest(PcbTestResult::Passed_R9);
        }
        else if (completeString.contains("NVS3320#"))
        {
            finishPcbTest(PcbTestResult::Passed_R8);
        }
        else
        {
            //
        }

        m_testHasReponse = true;
    }
    else
    {
        qInfo() << "readData: m_testInProgress=false";
    }

}

void R9PcbTesterWidget::onSerialStateChanged(
    const QString& portName, SerialState serialState, const QString& extra)
{
    switch (serialState)
    {
    case SerialState::Connected:
        setPcbTestEnabled(true);

        mSerialPortName = portName;
        break;

    case SerialState::Disconnected:
        setPcbTestEnabled(false);

        mSerialPortName = "";
        break;

    case SerialState::RxError:
        break;

    case SerialState::TxError:
        if (m_testInProgress == true)
        {
            finishPcbTest(PcbTestResult::Failed_TxError);
        }
        break;

    case SerialState::DoneBytesWritten:
        if (m_testInProgress)
        {
            m_testSerialConnected = true;
        }
        break;

    case SerialState::Timeout:
        if (m_testInProgress == true)
        {
            finishPcbTest(PcbTestResult::Failed_NoResponse);
        }
        else if (m_testSerialConnected == false)
        {
            QMessageBox::warning(this, tr("Warning"), extra);
        }
        break;

    default:
        break;
    }
}

void R9PcbTesterWidget::onPutConsole(const QByteArray& data)
{
    m_pConsole->putData(data);
}

void R9PcbTesterWidget::saveHistoryToFile(QStringListModel& listModel)
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

void R9PcbTesterWidget::loadHistoryFromFile(QStringListModel& listModel)
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

    m_pUi->pcbTestHistoryListView->scrollToBottom();
}


void R9PcbTesterWidget::handlePcbTestTimeout()
{
    const QString error = tr("Pcb test timed out for port %1.\n")
                              .arg(mSerialPortName);

    if (m_testInProgress == true)
    {
        if (m_testHasReponse == true)
        {
            finishPcbTest(PcbTestResult::Failed_WrongRx_Unknown);
        }
        else
        {
            finishPcbTest(PcbTestResult::Failed_NoResponse);
        }
    }
    else if (m_testSerialConnected == false)
    {
        QMessageBox::warning(this, tr("Warning"), error);
    }
}

void R9PcbTesterWidget::sendPcbTestSerialTx()
{
    emit writeSerialData("\r\n");

    //writeData("show ptb\n");
}
