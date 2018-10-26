#include "ddns_server.h"

#define PORT 39151
#define WAIT 1

DDNS_Server::DDNS_Server(QObject *parent)
    :QTcpServer(parent)
{
    qDebug() << "服务器启动中...";
    QThread::sleep(WAIT);

    connect(this,SIGNAL(acceptError(QAbstractSocket::SocketError)),this,SLOT(errorCtrl()));
    if(!listen(QHostAddress::AnyIPv4,PORT))
    {
        qDebug() << "【错误】在端口" << PORT << "启动失败！";
        exitServer();
        return;
    }
    qDebug() << "启动成功！\n";
}

DDNS_Server::~DDNS_Server()
{
    qDebug() << "服务器已关闭。";
}

void DDNS_Server::incomingConnection(qintptr sockid)
{
    qDebug() << "【新连接】";
    ClientSocket *cli=new ClientSocket(this);
    connect(cli,SIGNAL(exitServer()),this,SLOT(exitServer()));
    connect(cli,SIGNAL(disconnected()),cli,SLOT(deleteLater()));

    if(!cli->setSocketDescriptor(sockid))
    {
        qDebug() << "【错误】连接" << sockid << "建立失败！";
        return;
    }
    emit newConnection();
    qDebug() << "新连接配置完成。";
}

void DDNS_Server::errorCtrl()
{
    qDebug() << "【错误】服务端发生错误：\n"+errorString();
}

void DDNS_Server::exitServer()
{
    ::exit(0);
}
