#include "smartserialwindow.h"
#include "ui_smartserialwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QTextStream>
#include "checkalgorithm.h"
#include <QCryptographicHash>


#define CONTINIUS_DISPLAY 0
#define NEW_LINE_DISPLAY 1

#define DISPLAY_RECV 0
#define DISPLAY_SEND 1


SmartSerialWindow::SmartSerialWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SmartSerialWindow)
{
    ui->setupUi(this);
    //
    m_pSaveFile = new QFile();
    m_serialPort = new QSerialPort;
    m_statusmsg = new QLabel(this);
    //m_statusmsg->setFrameStyle(QFrame::Box|QFrame::Sunken);
    m_statusmsg->setText("<font color=red>Port not open</font>");
    ui->statusBar->addWidget(m_statusmsg);
    m_status_txnum = new QLabel(this);
    m_status_rxnum = new QLabel(this);
    m_status_info = new QLabel(this);
    m_status_txnum->setText("<font color=blue>Tx:0</font>");
    m_status_rxnum->setText("<font color=blue>Rx:0</font>");
    m_status_info->setText("-");
    ui->statusBar->addWidget(m_status_txnum);
    ui->statusBar->addWidget(m_status_rxnum);
    ui->statusBar->addWidget(m_status_info);
    //
    QActionGroup* checkalgorithm_action_group = new QActionGroup(this);
    checkalgorithm_action_group->addAction(ui->actionCheckSum);
    checkalgorithm_action_group->addAction(ui->actionCheckXOR);
    checkalgorithm_action_group->addAction(ui->actionCheckMD5_32);
    checkalgorithm_action_group->addAction(ui->actionCheckMD5_16);
    checkalgorithm_action_group->addAction(ui->actionCRC_Modbus);
    checkalgorithm_action_group->addAction(ui->actionCheckLRC);
    checkalgorithm_action_group->setExclusive(true);
    //
    //
    m_sendtimer = new QTimer;
    //
    connect(ui->actionAbout,&QAction::triggered,this,&SmartSerialWindow::on_aboutAction);
    connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(onRecvMsg()), Qt::DirectConnection);
    connect(ui->comboBox_baud, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_baudrateIndexChange(int)));
    connect(m_sendtimer,SIGNAL(timeout()),this,SLOT(onSendTimerTimeOut()));
    connect(checkalgorithm_action_group, SIGNAL(triggered(QAction *)), this, SLOT(action_checkalgorithm_group(QAction *)));
    //
    on_pushButton_refreshports_clicked();
    ui->actionCRC_Modbus->setChecked(true);
    m_checkAlgorithm = 4;
    //
    m_PortOpen_flg = 0;
    m_DispBuffIdx = 0;
    m_pDispBuff[0] = new QString("");
    m_pDispBuff[1] = new QString("");
    m_tx_bytes = 0;
    m_rx_bytes = 0;
    m_bAutoSend = false;
    m_startSave = false;
    m_recvWaitTime_ms = 0;
    m_sendWaitTime_ms = 0;
    //
    ui->comboBox_baud->addItem(QStringLiteral("1200"), QSerialPort::Baud1200);
    ui->comboBox_baud->addItem(QStringLiteral("2400"), QSerialPort::Baud2400);
    ui->comboBox_baud->addItem(QStringLiteral("4800"), QSerialPort::Baud4800);
    ui->comboBox_baud->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->comboBox_baud->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->comboBox_baud->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->comboBox_baud->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    ui->comboBox_baud->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->comboBox_baud->addItem(tr("Custom"));
    ui->comboBox_baud->setCurrentText(tr("9600"));
    //
    ui->comboBox_databit->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->comboBox_databit->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->comboBox_databit->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->comboBox_databit->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->comboBox_databit->setCurrentText(tr("8"));
    //
    ui->comboBox_parity->addItem(tr("None"), QSerialPort::NoParity);
    ui->comboBox_parity->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->comboBox_parity->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->comboBox_parity->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->comboBox_parity->addItem(tr("Space"), QSerialPort::SpaceParity);
    ui->comboBox_parity->setCurrentText(tr("None"));
    //
    ui->comboBox_stopbit->addItem(QStringLiteral("1"), QSerialPort::OneStop);
    ui->comboBox_stopbit->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
    ui->comboBox_stopbit->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    ui->comboBox_stopbit->setCurrentText(tr("1"));
    //
    ui->comboBox_flowctl->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->comboBox_flowctl->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->comboBox_flowctl->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
    ui->comboBox_flowctl->setCurrentText(tr("None"));
    //
    ui->actionOpenPort->setIcon(QIcon(":/images/png/portopen.png"));
    ui->actionOpenPort->setToolTip("Open Port");
    ui->lineEdit_sendtimeout->setText("5000");
    ui->lineEdit_recvtimeout->setText("0");
    ui->checkBox_switchtab->setChecked(true);
    ui->checkBox_linefeed->setChecked(true);
    ui->checkBox_disp_txrx->setChecked(true);
    ui->checkBox_hexdisp->setChecked(true);
    ui->checkBox_sendashex->setChecked(true);
    ui->lineEdit_sendcycletime->setText("0");
    ui->lineEdit_sendtimes->setText("0");
    m_sendTimes = 0;
    m_enableSendtimes = false;
//    ui->tabWidget->setCurrentIndex(0);
    //
    //
}

SmartSerialWindow::~SmartSerialWindow()
{
    if(m_PortOpen_flg)
    {
        m_serialPort->close();
        m_PortOpen_flg = 0;
    }
    delete ui;
}

void SmartSerialWindow::on_aboutAction()
{
    QMessageBox::about(NULL, "About", "<center><b>lanyunhai (C) 2022/03/01</b></center>欢迎提出宝贵建议! <br>Email: <font color='blue'><a href='mailto:lan_yunhai@163.com'>lan_yunhai@163.com</a></font>");
}

void SmartSerialWindow::on_pushButton_refreshports_clicked()
{
    int intID = 0;
    ui->comboBox_ports->clear();
    foreach(QSerialPortInfo portInfo, QSerialPortInfo::availablePorts())
    {
        if(!portInfo.isBusy())
        {
            QString strComboShow = (portInfo.portName() + "：" + portInfo.description());
            ui->comboBox_ports->insertItem(intID++,strComboShow,portInfo.portName());
        }
    }
}

void SmartSerialWindow::on_pushButton_openport_clicked()
{
    if(0==m_PortOpen_flg)
    {
        m_PortName = ui->comboBox_ports->currentData().toString();
        qint32  baudrate;
        if(8==ui->comboBox_baud->currentIndex())
        {
            baudrate = ui->comboBox_baud->currentText().toInt();
            if(0==baudrate)
            {
                QMessageBox::warning(this,"Warning",QString("Wrong Baudrate!"));
                return;
            }
        }
        else
        {
            baudrate = ui->comboBox_baud->currentData().toInt();
        }
        QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(ui->comboBox_databit->currentData().toInt());
        QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(ui->comboBox_parity->currentData().toInt());
        QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(ui->comboBox_stopbit->currentData().toInt());
        QSerialPort::FlowControl flowControl = static_cast<QSerialPort::FlowControl>(ui->comboBox_flowctl->currentData().toInt());
        //
        m_serialPort->setPortName(m_PortName);
        m_serialPort->setBaudRate(baudrate);
        m_serialPort->setParity(parity);
        m_serialPort->setDataBits(dataBits);
        m_serialPort->setStopBits(stopBits);
        m_serialPort->setFlowControl(flowControl);
        //
        m_sendWaitTime_ms = ui->lineEdit_sendtimeout->text().toInt();
        m_recvWaitTime_ms = ui->lineEdit_recvtimeout->text().toInt();
        //
        if(!m_serialPort->open(QIODevice::ReadWrite))
        {
            QMessageBox::warning(this,"Error",QString("Can not open port: ")+m_PortName+QString(" !"));
            return;
        }
        //
        m_PortOpen_flg = 1;
        ui->pushButton_openport->setText("Close Port");
        ui->actionOpenPort->setIcon(QIcon(":/images/png/portclose.png"));
        ui->actionOpenPort->setToolTip("Close Port");
        //ui->statusBar->showMessage(m_PortName+QString(":%1-").arg(baudrate)+QString("%1-").arg(dataBits)+(ui->comboBox_parity->currentText())+QString("-%1").arg(stopBits));
        m_statusmsg->setText(QString("<font color=green>")+m_PortName+QString(":%1bps-").arg(baudrate)+QString("%1-").arg(dataBits)+(ui->comboBox_parity->currentText())+QString("-%1</font>").arg(stopBits));
        if(ui->checkBox_switchtab->checkState())
        {
            ui->tabWidget->setCurrentIndex(1);
        }
    }
    else
    {
        m_serialPort->close();
        m_PortOpen_flg = 0;
        ui->pushButton_openport->setText("Open Port");
        ui->actionOpenPort->setIcon(QIcon(":/images/png/portopen.png"));
        ui->actionOpenPort->setToolTip("Open Port");
        //ui->statusBar->showMessage("<font color=red>Port not open</font>");
        m_statusmsg->setText("<font color=red>Port closed</font>");
    }
}

void SmartSerialWindow::onRecvMsg()
{
    //QMessageBox::information(NULL,"info","ok recv");
    m_serialPort->waitForBytesWritten(5);
    if(m_recvWaitTime_ms>0)
    {
        m_serialPort->waitForReadyRead(m_recvWaitTime_ms);
    }
    QByteArray buffer = m_serialPort->readAll();
    //buffer = m_serialPort->readAll();
    //while (m_serialPort->waitForReadyRead(10))
    //{
    //    buffer += m_serialPort->readAll();
    //}
    //QMessageBox::information(NULL,"",QString("%1").arg(buffer.size()));
    if(buffer.size()<1) return;
    m_rx_bytes += buffer.size();
    m_status_rxnum->setText(QString("<font color=blue>Rx:%1</font>").arg(m_rx_bytes));
    QString sRx;
    if(ui->checkBox_disp_txrx->checkState())
    {
        if(ui->checkBox_timestamp->checkState())
        {
            QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
            sRx = "["+time+" Rx]";
        }
        else
        {
            sRx = "[Rx]";
        }
    }
    else
    {
        sRx = "";
    }
    QByteArray sline;
    bool hexdisp = ui->checkBox_hexdisp->checkState();
    if(hexdisp)
    {
        sline = QString(sRx+" ").toLatin1()+buffer.toHex(' ').toUpper();
    }
    else
    {
        sline = QString(sRx).toLatin1()+buffer;
    }
    if(ui->checkBox_linefeed->checkState())
    {
        display( sline ,DISPLAY_RECV , NEW_LINE_DISPLAY);
    }
    else
    {
        display( sline ,DISPLAY_RECV , CONTINIUS_DISPLAY);
    }
}

void SmartSerialWindow::display(const QByteArray &line, int type, int disp)
{
    QString sprefix,ssuffix;
    QByteArray saveline;
    switch(type)
    {
    case DISPLAY_RECV:
        sprefix = "<font color=red>";
        break;
    case DISPLAY_SEND:
        sprefix = "<font color=blue>";
        break;
    default:
        sprefix = "";
    }
    switch(disp)
    {
    case CONTINIUS_DISPLAY:
        ssuffix = "</font>";
        saveline = "";
        break;
    case NEW_LINE_DISPLAY:
        ssuffix = "</font><br>";
        saveline = "\r\n";
        break;
    default:
        ssuffix = "";
    }
    //
    if(m_startSave)
    {
        saveline = line+saveline;
        m_pSaveFile->write(saveline);
        //QMessageBox::information(NULL,"",saveline);
    }
    //
    *(m_pDispBuff[m_DispBuffIdx]) += (sprefix+line+ssuffix);
    int lastidx = (0==m_DispBuffIdx)?1:0;
    //
    //
    m_DispBuffer = *(m_pDispBuff[lastidx]) + *(m_pDispBuff[m_DispBuffIdx]);
    ui->textEdit_message->setText(m_DispBuffer);
    //
    QTextCursor cursor = ui->textEdit_message->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textEdit_message->setTextCursor(cursor);
    //
    if(m_pDispBuff[m_DispBuffIdx]->size()>10240) //maximum display buffer size
    {
        m_DispBuffIdx = lastidx;
        m_pDispBuff[m_DispBuffIdx]->clear();
    }
    //
    m_status_info->setText(m_serialPort->errorString());
}

void SmartSerialWindow::on_pushButton_clearmsg_clicked()
{
    m_pDispBuff[0]->clear();
    m_pDispBuff[1]->clear();
    ui->textEdit_message->clear();
    //
    m_serialPort->readAll();
}

void SmartSerialWindow::slot_baudrateIndexChange(int idx)
{
    //QMessageBox::information(NULL,"",QString("%1").arg(idx));
    //QMessageBox::information(NULL,"",text);
    if(8==idx)
    {
        //QLineEdit *lineEdit = new QLineEdit(this);
        //ui->comboBox_baud->setLineEdit(lineEdit);
        ui->comboBox_baud->setEditable(true);
        ui->comboBox_baud->lineEdit()->setText("300");
        ui->comboBox_baud->lineEdit()->selectAll();
    }
    else
    {
        ui->comboBox_baud->setEditable(false);
    }
}

void SmartSerialWindow::on_actionOpenPort_triggered()
{
    on_pushButton_openport_clicked();
}

void SmartSerialWindow::send(void)
{
    if(!m_PortOpen_flg)
    {
        //QMessageBox::information(NULL,"Cannot Send","Port not open!");
        return;
    }
    QString text, sTx;
    text = ui->lineEdit_sendhead->text();
    text += ui->lineEdit_send->text();
    text += ui->lineEdit_sendtail->text();
    text += ui->lineEdit_sendtail1->text();
    if(0==text.size())
    {
        //QMessageBox::information(NULL,"Cannot Send","Empty to send!");
        return;
    }
    bool sendashex = ui->checkBox_sendashex->checkState();
    QByteArray hex;
    if(sendashex)
    {
        hex = QByteArray::fromHex(text.toLatin1());
    }
    else //send as ascii
    {
        if(ui->checkBox_esc->checkState())
        {
            text.replace("\\n","\n");
            text.replace("\\r","\r");
            text.replace("\\t","\t");
            text.replace("\\v","\v");
        }
        hex = text.toLocal8Bit(); //text.toLatin1();
    }
    int sendbytes = m_serialPort->write(hex,hex.size());
    if(m_sendWaitTime_ms>0)
    {
        m_serialPort->waitForBytesWritten(m_sendWaitTime_ms);
    }
    if(sendbytes>0)
    {
        m_tx_bytes += sendbytes;
        m_status_txnum->setText(QString("<font color=blue>Tx:%1</font>").arg(m_tx_bytes));
    }
    else
    {
        QMessageBox::warning(this,"error","Error to Send to serial port!");
        return;
    }
    QByteArray sendline;
    if(ui->checkBox_disp_txrx->checkState())
    {
        if(ui->checkBox_timestamp->checkState())
        {
            QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
            sTx = "["+time+" Tx]";
        }
        else
        {
            sTx = "[Tx]";
        }
        if(ui->checkBox_hexdisp->checkState())
        {
            sendline = QString(sTx+" ").toLatin1()+hex.toHex(' ').toUpper();
        }
        else
        {
            sendline = QString(sTx+" ").toLatin1()+hex;
        }

        if(ui->checkBox_linefeed->checkState())
        {
            display( sendline ,DISPLAY_SEND , NEW_LINE_DISPLAY);
        }
        else
        {
            display( sendline ,DISPLAY_SEND , CONTINIUS_DISPLAY);
        }
    }
    else
    {
        sTx = "";
    }
    //QMessageBox::information(NULL,"",text);
    //QMessageBox::information(NULL,"",hex.toHex(' '));
}

void SmartSerialWindow::on_pushButton_send_clicked()
{
    if(!m_PortOpen_flg)
    {
        QMessageBox::information(NULL,"Cannot Send","Port not open!");
        return;
    }
    QString text;
    text = ui->lineEdit_sendhead->text();
    text += ui->lineEdit_send->text();
    text += ui->lineEdit_sendtail->text();
    text += ui->lineEdit_sendtail1->text();
    if(0==text.size())
    {
        QMessageBox::information(NULL,"Cannot Send","Empty to send!");
        return;
    }
    //
    QString cycletime = ui->lineEdit_sendcycletime->text();
    m_sendTimes = (ui->lineEdit_sendtimes->text()).toInt();
    int period = cycletime.toInt();
    if(period>0&&1!=m_sendTimes)
    {
        if(!m_bAutoSend)
        {
            ui->lineEdit_sendcycletime->setEnabled(false);
            m_bAutoSend = true;
            ui->pushButton_send->setText("Stop");
            m_sendtimer->setInterval(period);
            send();
            m_sendtimer->start();
            if(m_sendTimes>1)
            {
                m_enableSendtimes=true;
                m_sendTimes-=2;
            }
        }
        else
        {
            stopAutoSend();
        }
    }
    else
    {
        send();
    }
}

void SmartSerialWindow::stopAutoSend(void)
{
    m_enableSendtimes = false;
    ui->lineEdit_sendcycletime->setEnabled(true);
    m_sendtimer->stop();
    m_bAutoSend = false;
    ui->pushButton_send->setText("Send");
}

void SmartSerialWindow::onSendTimerTimeOut(void)
{
    send();
    if(m_enableSendtimes)
    {
        if(m_sendTimes>0) m_sendTimes--;
        else stopAutoSend();
    }
}

void SmartSerialWindow::on_pushButton_clearsend_clicked()
{
    ui->lineEdit_sendhead->clear();
    ui->lineEdit_send->clear();
    ui->lineEdit_sendtail->clear();
    ui->lineEdit_sendtail1->clear();
}

void SmartSerialWindow::on_checkBox_disp_txrx_stateChanged(int arg1)
{
    //QMessageBox::information(NULL,"info",QString("%1").arg(arg1));
    if(arg1)
    {
        ui->checkBox_timestamp->setEnabled(true);
    }
    else
    {
        ui->checkBox_timestamp->setEnabled(false);
        ui->checkBox_timestamp->setChecked(false);
    }
}

void SmartSerialWindow::on_actionSaveDisplay_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
            this,
            tr("Select a file to save current display data"),
            NULL,
            tr("text file(*.txt);;All files(*.*)"));
    if (fileName.isEmpty())
    {
        //QMessageBox::warning(this, "Warning!", "Failed to open the video!");
        return;
    }
    //QMessageBox::information(NULL,"filename",fileName);
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,"error","open file failure!");
        return;
    }
    else
    {
        QTextStream textStream(&file);
        QString str = ui->textEdit_message->toPlainText();
        textStream<<str;
        file.close();
        //QMessageBox::warning(this,"tip","Save File Success!");
    }

}

void SmartSerialWindow::on_checkBox_sendashex_toggled(bool checked)
{
    //QMessageBox::information(NULL,"info",QString("%1").arg(checked));
    if(checked)
    {
        //ui->pushButton_calccheck->setEnabled(true);
        ui->checkBox_esc->setEnabled(false);
        ui->checkBox_esc->setChecked(false);
    }
    else
    {
        //ui->pushButton_calccheck->setEnabled(false);
        ui->checkBox_esc->setEnabled(true);
    }
}

void SmartSerialWindow::on_pushButton_hex2int_clicked()
{
    QString hexstr = ui->lineEdit_hex1->text();
    QByteArray hex = QByteArray::fromHex(hexstr.toLatin1());
    std::reverse(hex.begin(), hex.end());
    long long n=0;
    memcpy(&n,hex.data(),sizeof(n)<((unsigned int)(hex.size()))?sizeof(n):(hex.size()));
    for(int i=sizeof(n);i<hex.size();i++)
    {
        if(0!=hex.at(i))
        {
            ui->lineEdit_num1->setText("too large!");
            return;
        }
    }
    //
    QString numstr = QString::number(n,10);
    ui->lineEdit_num1->setText(numstr);
}

void SmartSerialWindow::on_pushButton_int2hex_clicked()
{
    QString numstr = ui->lineEdit_num1->text();
    bool ok;
    long long n = numstr.toLongLong(&ok, 10);
    //QMessageBox::information(NULL,"info",QString("%1").arg(n));
    //QMessageBox::information(NULL,"info",QString("%1").arg(ok));
    if(!ok)
    {
        ui->lineEdit_hex1->setText("---");
        return;
    }
    QString hexstr = QString::number(n,16);
    ui->lineEdit_hex1->setText(hexstr);
}

void SmartSerialWindow::on_pushButton_hex2float_clicked()
{
    QString hexstr = ui->lineEdit_hex1->text();
    unsigned int n=0;
    sscanf( hexstr.toLatin1().data() ,"%x",&n);
    //QMessageBox::information(NULL,"info",QString::number(n,16));
    //QMessageBox::information(NULL,"info",QString("%1").arg(*((float*)(&n))));
    QString numstr = QString("%1").arg(*((float*)(&n)), 0, 'E', 8);
    ui->lineEdit_num1->setText(numstr);
}

void SmartSerialWindow::on_pushButton_float2hex_clicked()
{
    QString numstr = ui->lineEdit_num1->text();
    float f = numstr.toFloat();
    //QMessageBox::information(NULL,"info",QString("%1").arg(f));
    QString hexstr = QString::number(*((unsigned int*)&f),16);
    ui->lineEdit_hex1->setText(hexstr);
}

void SmartSerialWindow::on_pushButton_hex2double_clicked()
{
    QString hexstr = ui->lineEdit_hex1->text();
    QByteArray hex = QByteArray::fromHex(hexstr.toLatin1());
    std::reverse(hex.begin(), hex.end());
    //
    unsigned long long n=0;
    memcpy(&n,hex.data(),sizeof(n)<((unsigned int)(hex.size()))?sizeof(n):(hex.size()));
    for(int i=sizeof(n);i<hex.size();i++)
    {
        if(0!=hex.at(i))
        {
            n = 0x7ffc000000000000;//NAN
            break;
        }
    }
    //QMessageBox::information(NULL,"info",QString::number(*((unsigned long long *)hex.data()),16));
    //QMessageBox::information(NULL,"info",QString::number(n,16));
    QString numstr = QString("%1").arg(*((double*)(&n)), 0, 'E', 16);
    ui->lineEdit_num1->setText(numstr);
}

void SmartSerialWindow::on_pushButton_double2hex_clicked()
{
    QString numstr = ui->lineEdit_num1->text();
    double d = numstr.toDouble();
    //QMessageBox::information(NULL,"info",QString("%1").arg(d));
    QString hexstr = QString::number(*((unsigned long long*)&d),16);
    ui->lineEdit_hex1->setText(hexstr);
}

void SmartSerialWindow::action_checkalgorithm_group(QAction *action)
{
    if(action == ui->actionCheckSum)
    {
        m_checkAlgorithm = 0;
        //QMessageBox::information(NULL,"info","checksum");
    }
    else if(action == ui->actionCheckXOR)
    {
        m_checkAlgorithm = 1;
        //QMessageBox::information(NULL,"info","checkxor");
    }
    else if(action == ui->actionCheckMD5_32)
    {
        m_checkAlgorithm = 2;
    }
    else if(action == ui->actionCheckMD5_16)
    {
        m_checkAlgorithm = 3;
    }
    else if(action == ui->actionCRC_Modbus)
    {
        m_checkAlgorithm = 4;
    }
    else if(action == ui->actionCheckLRC)
    {
        m_checkAlgorithm = 5;
        ui->checkBox_sendashex->setChecked(false);
    }

}

void SmartSerialWindow::on_pushButton_calccheck_clicked()
{
    QString text = ui->lineEdit_send->text();
    bool sendashex = ui->checkBox_sendashex->checkState();
    QByteArray hex;
    if(sendashex) //as hex
    {
        hex = QByteArray::fromHex(text.toLatin1());
    }
    else //as ascii
    {
        hex = text.toLocal8Bit(); //text.toLatin1();
    }
    //
    if(0==hex.size()) return;
    QByteArray checkcode;
    unsigned char ch;
    unsigned short w;
    switch(m_checkAlgorithm)
    {
    case 0: //checksum
        ch = Alg_CheckSum((unsigned char *)(hex.data()),hex.size());
        checkcode.append(ch);
        break;
    case 1: //checkxor
        ch = Alg_CheckXOR((unsigned char *)(hex.data()),hex.size());
        checkcode.append(ch);
        break;
    case 2: //md5-32
        checkcode = QCryptographicHash::hash(hex,QCryptographicHash::Md5);
        break;
    case 3: //md5-16
        checkcode = QCryptographicHash::hash(hex,QCryptographicHash::Md5);
        checkcode = checkcode.mid(4,8);
        break;
    case 4: //crc-modbus
        w = CRC16_Modbus((unsigned char *)(hex.data()),hex.size());
        ch = (w>>8)&0xff;
        checkcode.append(ch);
        ch = w&0xff;
        checkcode.append(ch);
        //QMessageBox::information(NULL,"info",QString("%1").arg(w));
        break;
    case 5: //check lrc
        if(!sendashex) hex = QByteArray::fromHex(hex);
        ch = Alg_CheckLRC((unsigned char *)(hex.data()),hex.size());
        checkcode.append(ch);
        //if(sendashex) checkcode.append(ch);
        //else checkcode = (QByteArray::number(ch,16)).toUpper();
        break;
    default:
        break;
    }
    //
    //if(sendashex) ui->lineEdit_sendtail->setText(checkcode.toHex(' ').toUpper());
    //else ui->lineEdit_sendtail->setText(checkcode);
    ui->lineEdit_sendtail->setText(checkcode.toHex(' ').toUpper());
}



void SmartSerialWindow::on_pushButton_utf82hex_clicked()
{
    QString text = ui->plainTextEdit_txtcode->toPlainText();
    QByteArray hex = text.toUtf8();
    ui->plainTextEdit_hexcode->setPlainText(hex.toHex(' ').toUpper());
}

void SmartSerialWindow::on_pushButton_hex2utf8_clicked()
{
    QString hexstr = ui->plainTextEdit_hexcode->toPlainText();
    QByteArray hex = QByteArray::fromHex(hexstr.toLatin1());
    ui->plainTextEdit_txtcode->setPlainText(QString::fromUtf8(hex));
}

void SmartSerialWindow::on_pushButton_local2hex_clicked()
{
    QString text = ui->plainTextEdit_txtcode->toPlainText();
    QByteArray hex = text.toLocal8Bit();
    ui->plainTextEdit_hexcode->setPlainText(hex.toHex(' ').toUpper());
}

void SmartSerialWindow::on_pushButton_hex2local_clicked()
{
    QString hexstr = ui->plainTextEdit_hexcode->toPlainText();
    QByteArray hex = QByteArray::fromHex(hexstr.toLatin1());
    ui->plainTextEdit_txtcode->setPlainText(QString::fromLocal8Bit(hex));
}


void SmartSerialWindow::on_actionClear_Tx_Rx_Cnt_triggered()
{
    m_tx_bytes = 0;
    m_rx_bytes = 0;
    m_status_txnum->setText(QString("<font color=blue>Tx:%1</font>").arg(m_tx_bytes));
    m_status_rxnum->setText(QString("<font color=blue>Rx:%1</font>").arg(m_rx_bytes));
}

void SmartSerialWindow::on_actionStartSave_triggered()
{
    if(!m_startSave)
    {
        QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Select a file name to save."),
                NULL,
                tr("data file(*.dat);;All files(*.*)"));
        if (fileName.isEmpty())
        {
            //QMessageBox::warning(this, "Warning!", "Failed to open the video!");
            return;
        }
        //QMessageBox::information(NULL,"filename",fileName);
        m_pSaveFile->setFileName(fileName);
        if(!m_pSaveFile->open(QIODevice::WriteOnly))
        {
            QMessageBox::warning(this,"error","open file failure!");
            return;
        }
        //
        m_startSave = true;
        //
        ui->actionStartSave->setIcon(QIcon(":/images/png/stopsave.png"));
        ui->actionStartSave->setToolTip("Stop Continious Save");
    }
    else
    {
        m_startSave = false;
        m_pSaveFile->close();
        //
        ui->actionStartSave->setIcon(QIcon(":/images/png/startsave.png"));
        ui->actionStartSave->setToolTip("Start Continious Save");
    }
}
