#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QElapsedTimer>
#include <QMetaEnum>
#include <QString>
#include <QtGui/QStandardItemModel>
#include <QtSerialPort/QSerialPort>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>

#include "CustomStringListModel.h"

QT_BEGIN_NAMESPACE

class QLabel;
class QTimer;

namespace Ui
{
class MainWindow;
}

QT_END_NAMESPACE

class MainWindowContents;
class Console;
class SettingsDialog;
class HorizontalBarWidget;
class StatItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public:
    enum PcbTestResult
    {
        None = 0,
        Passed,
        Failed_NoResponse,
        Failed_WrongRx,
        Failed_TxError,
        NotTested
    };
    Q_ENUM(PcbTestResult)

private:
    void initActionsConnections();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray& data);
    void readData();

    void handleError(QSerialPort::SerialPortError error);
    void handleSerialTxRetry();
    void handlePcbTestTimeout();
    void handleBytesWritten(qint64 bytes);
    void handleWriteTimeout();

    void updatePcbTestProgress();

    void beginPcbTest();

    void clearStatisticsData();

    void about();

private:
    void setPcbTestEnabled(bool);

    void finishPcbTest(const PcbTestResult& result);

    void checkPcbTestFinished();
    void handlePcbTestDone(const PcbTestResult& result);

    void showStatusMessage(const QString& message);
    void showWriteError(const QString& message);

    void sendPcbTestSerialTx();

private:
    Ui::MainWindow* m_pMainWindow = nullptr;
    MainWindowContents* m_pMainWindowContents = nullptr;
    Console* m_pConsole = nullptr;
    QLabel* m_pStatusLabel = nullptr;
    QLabel* m_pTestResultLabel = nullptr;

    SettingsDialog* m_pSettingsDialog = nullptr;
    QTimer* m_pPcbTestTimer = nullptr;
    QTimer* m_pSerialResponseTimer = nullptr;
    QTimer* m_pSerialTxRetryTimer = nullptr;
    QTimer* m_pUiUpdateTimer = nullptr;

    QSerialPort* m_pSerialPort = nullptr;

    QPushButton* m_pTestButton = nullptr;

    qint64 m_bytesToWrite = 0;
    QByteArray m_readBuffer;

    qint64 m_pcbTestCount = 0;
    double m_pcbTestProgressValue = 0;
    PcbTestResult m_pcbTestResult = PcbTestResult::None;

    bool m_testInProgress = false;
    bool m_testSerialConnected = false;

    CustomStringListModel m_pcbTestHistoryListModel;
    QElapsedTimer m_pcbTestElapsedTimer;

    HorizontalBarWidget* m_pPcbTestStatsWidget;

    StatItem* m_pStatItem;
};
#endif // MAINWINDOW_H
