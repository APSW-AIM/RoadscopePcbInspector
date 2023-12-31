// Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
// Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtSerialPort/QSerialPort>
#include <QtWidgets/QDialog>

#include "ui_settingsdialog.h"

QT_BEGIN_NAMESPACE

//namespace Ui {
//class SettingsDialog;
//}

class QIntValidator;

QT_END_NAMESPACE

class MainWindow;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    enum ProductType
    {
        Roadscope9,
        PI5009
    };

public:
    struct Settings
    {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        ProductType productType;
        bool localEchoEnabled;
    };

    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

    Settings settings() const;

protected:
    void showEvent(QShowEvent* pShowEvent) override;

private slots:
    void showPortInfo(int idx);
    void apply();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);

private:
    void fillPortsParameters();
    void fillPortsInfo();
    void fillProductList();
    void updateSettings();

private:
    Ui::SettingsDialog* m_ui = nullptr;
    Settings m_currentSettings;
    QIntValidator* m_intValidator = nullptr;

    friend MainWindow;
};

#endif // SETTINGSDIALOG_H
