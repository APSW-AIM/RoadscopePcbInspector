#ifndef R9PCBTESTERWIDGET_H
#define R9PCBTESTERWIDGET_H

#include <QtWidgets/QWidget>
#include <QtCore/QElapsedTimer>

#include "ui_R9PcbTesterWidget.h"

#include "customstringlistmodel.h"
#include "SerialState.h"

//namespace Ui {
//class R9PcbTesterWidget;
//}


QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE



class MainWindow;

class Console;
class HorizontalBarWidget;
class StatItem;

class R9PcbTesterWidget : public QWidget
{
    Q_OBJECT

public:
    enum PcbTestResult
    {
        None = 0,
        Passed_R9,
        Passed_R8,
        Failed_NoResponse,
        Failed_WrongRx_R9,
        Failed_WrongRx_R8,
        Failed_WrongRx_Unknown,
        Failed_TxError,
        NotTested
    };
    Q_ENUM(PcbTestResult)

public:
    explicit R9PcbTesterWidget(QWidget* parent = nullptr);
    ~R9PcbTesterWidget();

Q_SIGNALS:
    void writeSerialData(const QByteArray& data);
    void writeSerialDataDone();

public slots:
    void clearStatisticsData();
    void onSerialRxCompleteString(const QString& completeString);
    void onSerialStateChanged(const QString& portName, SerialState serialState, const QString& extra);
    void onPutConsole(const QByteArray& data);



private slots:
    void handlePcbTestTimeout();
    void handleSerialTxRetry();
    void updatePcbTestProgress();
    void beginPcbTest();


//    void onSerialConnected();
//    void onSerialDisconnected();
//    void onSerialTxError();

private:
    void saveHistoryToFile(QStringListModel& listModel);
    void loadHistoryFromFile(QStringListModel& listModel);

    void setPcbTestEnabled(bool);

    void finishPcbTest(const PcbTestResult& result);

    void checkPcbTestFinished();

    void sendPcbTestSerialTx();

private:
    Ui::R9PcbTesterWidget* m_pUi = nullptr;
    Console* m_pConsole = nullptr;
    CustomStringListModel m_pcbTestHistoryListModel;

    qint64 m_pcbTestCount = 0;


    double m_pcbTestProgressValue = 0;
    PcbTestResult m_pcbTestResult = PcbTestResult::None;

    bool m_testInProgress = false;
    bool m_testHasReponse = false;
    bool m_testSerialConnected = false;


    QTimer* m_pPcbTestTimer = nullptr;
    QTimer* m_pUiUpdateTimer = nullptr;
    QTimer* m_pSerialTxRetryTimer = nullptr;

    QElapsedTimer m_pcbTestElapsedTimer;
    HorizontalBarWidget* m_pPcbTestStatsWidget;
    StatItem* m_pStatItem;

    QString mSerialPortName;
};

#endif // R9PCBTESTERWIDGET_H
