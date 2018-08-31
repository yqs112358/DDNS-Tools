#include "ddns_upload.h"

#define PORT 39151
#define CONF "./conf.dat"
#define DATA_VER QDataStream::Qt_5_10

#define WAIT_START 60
#define WAIT_TIME 20*60*1000

DDNS_Upload::DDNS_Upload(QObject *parent) : QObject(parent)
{
    qDebug() << "上传端启动中...";
    QThread::sleep(WAIT_START);

    connect(&s,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(errorCtrl(QAbstractSocket::SocketError)));
    qDebug() << "第一次更新测试....\n";
    sendData();
    connect(&t,SIGNAL(timeout()),this,SLOT(sendData()));
    t.start(WAIT_TIME);
    qDebug() << "初始化完成！\n";
}

DDNS_Upload::~DDNS_Upload()
{
    qDebug() << "上传端已关闭。";
}

void DDNS_Upload::errorCtrl(QAbstractSocket::SocketError err)
{
    if(err == QAbstractSocket::RemoteHostClosedError)
        qDebug() << "【消息】对方连接已关闭";
    else
        qDebug() << "【错误】连接发生错误：\n" << s.errorString();
}

void DDNS_Upload::putStr(QDataStream *dout, const QString &str)
{
    foreach (QChar c, str) {
        (*dout) << c;
    }
}

void DDNS_Upload::sendData()
{
    qDebug() << "【新连接启动】";

    try
    {
        if(!QFile::exists(CONF))
        {
            qDebug() << "【错误】配置文件不存在！";
            QThread::sleep(3);
            ::exit(1);
            return;
        }

        QByteArray res;
        QDataStream dout(&res,QIODevice::WriteOnly);
        dout.setVersion(DATA_VER);
        dout << (quint32)0;

        qDebug() << "开始读取配置文件。";
        QSettings settings(CONF,QSettings::IniFormat);
        settings.beginGroup("Main");
        QString name(settings.value("N").toString()),target(settings.value("T").toString());
        settings.endGroup();

        dout << QChar('S');
        putStr(&dout,name);
        dout.device()->seek(0);
        dout << (quint32)(res.size()-sizeof(quint32));
        qDebug() << "数据准备完毕。开始发送...";

        s.abort();
        s.connectToHost(target,PORT);
        if(!s.write(res))
            throw "数据发送失败！";
    }
    catch(const char *err_str)
    {
        qDebug() << "【错误】" << QString(err_str);
        return;
    }
    catch(...)
    {
        qDebug() << "【错误】发生未知错误！";
        return;
    }

    qDebug() << "【新连接结束】处理成功\n";
}
