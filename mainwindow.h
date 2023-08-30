#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QString>
#include <QtGui/QStandardItemModel>
#include <QtSerialPort/QSerialPort>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtCore/QMetaObject>


#include "settingsdialog.h"
#include "SerialState.h"


QT_BEGIN_NAMESPACE

class QLabel;
class QTimer;
class QStackedWidget;

namespace Ui
{
class MainWindow;
}

QT_END_NAMESPACE

class R9PcbTesterWidget;
class Pi5009TestWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();


private:
    void initActionsConnections();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void handleBytesWritten(qint64 bytes);
    void handleWriteTimeout();

    void writeSerialData(const QByteArray& data);
    void writeSerialDataDone();

    void about();

private:
    void changeProductType(SettingsDialog::ProductType productType);
    void showStatusMessage(const QString& message);


Q_SIGNALS:
    void serialRxComplete(const QString& str);
    void serialStateChanged(const QString& portName, SerialState serialState, const QString& extra="");
    void putConsole(const QByteArray& data);


private:
    Ui::MainWindow* m_pMainWindow = nullptr;
    QStackedWidget* m_pMainWindowContentsStack = nullptr;
    R9PcbTesterWidget* m_pR9PcbTesterWidget = nullptr;
    Pi5009TestWidget* m_pPi5009TestWidget = nullptr;

    QLabel* m_pStatusLabel = nullptr;
    QLabel* m_pTestResultLabel = nullptr;

    SettingsDialog* m_pSettingsDialog = nullptr;
    SettingsDialog::Settings mCurrentSettings;

    QTimer* m_pSerialResponseTimer = nullptr;
    QSerialPort* m_pSerialPort = nullptr;

    QPushButton* m_pTestButton = nullptr;

    qint64 m_bytesToWrite = 0;
    QByteArray m_readBuffer;

    QMetaObject::Connection m_serialStateChangedConnection;
    QMetaObject::Connection m_serialRxCompleteConnection;
    QMetaObject::Connection m_clearStatConnection;
    QMetaObject::Connection m_writeSerialDataConnection;
    QMetaObject::Connection m_writeSerialDataDoneConnection;
    QMetaObject::Connection m_putConsoleConnection;

};
#endif // MAINWINDOW_H
