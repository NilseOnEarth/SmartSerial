#ifndef SMARTSERIALWINDOW_H
#define SMARTSERIALWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QFile>


namespace Ui {
class SmartSerialWindow;
}

class SmartSerialWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SmartSerialWindow(QWidget *parent = 0);
    ~SmartSerialWindow();

private slots:
    void on_aboutAction();
    void on_pushButton_refreshports_clicked();
    void on_pushButton_openport_clicked();
    void onRecvMsg();
    void slot_baudrateIndexChange(int idx);
    void on_pushButton_clearmsg_clicked();
    void onSendTimerTimeOut(void);
    void action_checkalgorithm_group(QAction *);

    void on_actionOpenPort_triggered();

    void on_pushButton_send_clicked();

    void on_pushButton_clearsend_clicked();

    void on_checkBox_disp_txrx_stateChanged(int arg1);

    void on_actionSaveDisplay_triggered();

    void on_checkBox_sendashex_toggled(bool checked);

    void on_pushButton_hex2int_clicked();

    void on_pushButton_int2hex_clicked();

    void on_pushButton_hex2float_clicked();

    void on_pushButton_float2hex_clicked();

    void on_pushButton_hex2double_clicked();

    void on_pushButton_double2hex_clicked();

    void on_pushButton_calccheck_clicked();

    void on_pushButton_utf82hex_clicked();

    void on_pushButton_hex2utf8_clicked();

    void on_pushButton_local2hex_clicked();

    void on_pushButton_hex2local_clicked();


    void on_actionClear_Tx_Rx_Cnt_triggered();

    void on_actionStartSave_triggered();

private:
    Ui::SmartSerialWindow *ui;
    int m_PortOpen_flg;
    QString m_PortName;
    QSerialPort *m_serialPort;
    QLabel *m_statusmsg, *m_status_txnum, *m_status_rxnum, *m_status_info;
    QString *m_pDispBuff[2];
    int m_DispBuffIdx;
    QString m_DispBuffer;
    bool m_bAutoSend;
    QTimer *m_sendtimer;
    int m_checkAlgorithm;
    int m_sendTimes;
    bool m_enableSendtimes;
    bool m_startSave;
    QFile *m_pSaveFile;
    int m_recvWaitTime_ms;
    int m_sendWaitTime_ms;
    //
    unsigned int m_tx_bytes;
    unsigned int m_rx_bytes;
    //
    void display(const QByteArray &line, int type, int disp);
    void send(void);
    void stopAutoSend(void);
};

#endif // SMARTSERIALWINDOW_H
