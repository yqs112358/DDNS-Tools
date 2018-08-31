#include "dialog.h"
#include "ui_dialog.h"

#define PORT 39151
#define PORT_STR "39151"
#define CONF "./cli-conf.dat"
#define DATA_VER QDataStream::Qt_5_10

#define WD "053df23ac464e7e29cef5f5e31a50d8d"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    ui->lineEdit_port->setText(PORT_STR);
    ui->tableWidget->horizontalHeader()->setHidden(true);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    readConf();

    connect(&s,SIGNAL(connected()),this,SLOT(sendAsk()));
    connect(&s,SIGNAL(disconnected()),this,SLOT(endAsk()));
    connect(&s,SIGNAL(readyRead()),this,SLOT(dataRecv()));
    connect(&s,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(errorCtrl(QAbstractSocket::SocketError)));
    setMsg(tr("【查询】等待查询。"));
}

Dialog::~Dialog()
{
    writeConf();
    delete ui;
}

QString Dialog::getStr(QDataStream *din)
{
    QString res;
    QChar c;
    while(true)
    {
        if(din->atEnd())
            break;
        (*din) >> c;
        if(c == QChar(' '))
            break;
        else
            res.append(c);
    }
    return res;
}

void Dialog::putStr(QDataStream *dout, const QString &str)
{
    foreach (QChar c, str) {
        (*dout) << c;
    }
}

void Dialog::writeConf()
{
    QSettings s(CONF,QSettings::IniFormat);
    s.beginGroup("Main");
    s.setValue("Server",ui->lineEdit_server->text());
    s.setValue("Name",ui->lineEdit_name->text());
    s.endGroup();
    s.sync();
}

void Dialog::readConf()
{
    if(!QFile::exists(CONF))
        return;
    QSettings s(CONF,QSettings::IniFormat);
    s.beginGroup("Main");
    ui->lineEdit_server->setText(s.value("Server").toString());
    ui->lineEdit_name->setText(s.value("Name").toString());
    s.endGroup();
}

void Dialog::errorCtrl(QAbstractSocket::SocketError error)
{
    if(is_quit && error == QAbstractSocket::RemoteHostClosedError)
    {
        QMessageBox::information(this,tr("提示"),tr("成功关闭服务器！"),QMessageBox::Ok,QMessageBox::Ok);
        setMsg(tr("【状态】服务器已关闭。"));
    }
    else
    {
        qDebug() << "【错误】" << s.errorString();
        QMessageBox::critical(this,tr("错误"),s.errorString(),QMessageBox::Ok,QMessageBox::Ok);
        s.abort();
        endAsk();
        setMsg(tr("【状态】等待查询。"));
    }
}

void Dialog::disableInput()
{
    ui->lineEdit_server->setEnabled(false);
    ui->lineEdit_name->setEnabled(false);
    ui->pushButton_ask->setEnabled(false);
    ui->pushButton_list->setEnabled(false);
}

void Dialog::enableInput()
{
    ui->lineEdit_server->setEnabled(true);
    ui->lineEdit_name->setEnabled(true);
    if(!(ui->lineEdit_name->text()).isEmpty())
        ui->pushButton_ask->setEnabled(true);
    ui->pushButton_list->setEnabled(true);
}

void Dialog::tClear()
{
    tCount=0;
    ui->tableWidget->clear();
}

void Dialog::tAdd(QString name, QString addr)
{
    ui->tableWidget->setItem(tCount,0,new QTableWidgetItem(name));
    ui->tableWidget->setItem(tCount,1,new QTableWidgetItem(addr));
    ++tCount;
}

void Dialog::setMsg(QString str)
{
    tClear();
    tAdd("",str);
}

void Dialog::on_lineEdit_server_textChanged(const QString &str)
{
    if(str.isEmpty())
    {
        ui->pushButton_ask->setEnabled(false);
        ui->pushButton_list->setEnabled(false);
    }
    else
    {
        ui->pushButton_list->setEnabled(true);
        ui->pushButton_ask->setEnabled(!(ui->lineEdit_name->text()).isEmpty());
    }
}

void Dialog::on_lineEdit_name_textChanged(const QString &str)
{
    ui->pushButton_ask->setEnabled(!str.isEmpty());
}

void Dialog::on_pushButton_ask_clicked()
{
    is_list=is_quit=false;
    init();
}

void Dialog::on_pushButton_list_clicked()
{
    is_list=true;
    is_quit=false;
    init();
}

void Dialog::init()
{
    setMsg(tr("【状态】连接中"));
    disableInput();
    s.abort();
    dataSize=0;

    QHostAddress addr;
    if(!addr.setAddress(ui->lineEdit_server->text()))
    {
        QMessageBox::warning(this,tr("错误"),tr("无法识别输入的服务器地址！"),QMessageBox::Ok,QMessageBox::Ok);
        setMsg(tr("【状态】等待查询。"));
        return;
    }
    s.connectToHost(addr,PORT);
}

void Dialog::sendAsk()
{
    setMsg(tr("【状态】连接成功！查询中"));
    dat.clear();
    QDataStream dout(&dat,QIODevice::WriteOnly);
    dout.setVersion(DATA_VER);
    dout << (quint32)0;

    if(is_quit)
    {
        dout << QChar('Q');
    }
    else if(is_list)
    {
        dout << QChar('L');
    }
    else
    {
        dout << QChar('A');
        QString name(ui->lineEdit_name->text());
        for(int i=0;i<name.size();++i)
            if(name[i] == QChar(' '))
                name.remove(i,1);
        ui->lineEdit_name->setText(name);
        putStr(&dout,name);
    }

    dout.device()->seek(0);
    dout << (quint32)(dat.size()-sizeof(quint32));
    s.write(dat);
    s.flush();
}

void Dialog::dataRecv()
{
    if(dataSize == 0)
    {
        if(s.bytesAvailable() < (int)sizeof(quint32))
            return;
        QDataStream din(&s);
        din >> dataSize;
    }
    if(s.bytesAvailable() < dataSize)
        return;

    setMsg(tr("【状态】查询成功！接收中"));
    dat=s.readAll();

    QChar todo;
    QDataStream din(&dat,QIODevice::ReadOnly);

    din >> todo;
    if(todo == QChar('N'))
    {
        setMsg(tr("【状态】等待查询。"));
        QMessageBox::information(this,tr("提示"),tr("未查询到对应记录！"),QMessageBox::Ok,QMessageBox::Ok);
    }
    else
    {
        tClear();
        if(todo == QChar('R'))
        {
            QString addr(getStr(&din));
            tAdd(ui->lineEdit_name->text(),addr);
            tAdd("",tr("【状态】查询成功！"));
        }
        else if(todo == QChar('T'))
        {
            QString name,addr;
            quint32 count;
            din >> count;
            if(count <= 0)
            {
                setMsg(tr("【状态】等待查询。"));
                QMessageBox::information(this,tr("提示"),tr("未查询到记录！"),QMessageBox::Ok,QMessageBox::Ok);
            }
            else
            {
                if(count>10)
                    ui->tableWidget->setRowCount(count+1);
                else
                    ui->tableWidget->setRowCount(10);
                for(quint32 i=0;i<count;++i)
                {
                    name=getStr(&din);
                    addr=getStr(&din);
                    tAdd(name,addr);
                }
                tAdd("",tr("【状态】查询成功！"));
            }
        }
        else
        {
            qDebug() << "【错误】未知回应：\n" << QString(dat);
            QMessageBox::critical(this,tr("错误"),tr("收到未知回应！"),QMessageBox::Ok,QMessageBox::Ok);
        }
    }
    s.disconnectFromHost();
}

void Dialog::endAsk()
{
    enableInput();
}

void Dialog::on_pushButton_close_clicked()
{
    QString pass=QInputDialog::getText(this,tr("提示"),tr("请输入密码："),QLineEdit::Password);

    QByteArray bytePwd = pass.toLatin1();
    QByteArray bytePwdMd5 = QCryptographicHash::hash(bytePwd, QCryptographicHash::Md5);
    QString strPwdMd5 = bytePwdMd5.toHex();

    bytePwd = strPwdMd5.toLatin1();
    bytePwdMd5 = QCryptographicHash::hash(bytePwd, QCryptographicHash::Md5);
    strPwdMd5 = bytePwdMd5.toHex();

    if(strPwdMd5 == WD)
    {
        is_quit=true;
        init();
    }
}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == (Qt::ControlModifier | Qt::AltModifier) && event->key() == Qt::Key_Y)
    {
        enableClose=!enableClose;
        ui->pushButton_close->setEnabled(enableClose);
    }
}
