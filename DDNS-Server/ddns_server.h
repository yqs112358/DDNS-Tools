#ifndef DDNS_SERVER_H
#define DDNS_SERVER_H

#include <QTcpServer>
#include <QDebug>
#include <QThread>
#include "clientsocket.h"
#include <cstdlib>

class DDNS_Server : public QTcpServer
{
    Q_OBJECT
public:
    DDNS_Server(QObject *parent=nullptr);
    ~DDNS_Server();
    //void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    QString log_path;

protected:
    void incomingConnection(qintptr sockid);

public slots:
    void exitServer();
    void errorCtrl();
};

#endif // DDNS_SERVER_H
