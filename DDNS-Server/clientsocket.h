#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QTcpSocket>
#include <QByteArray>
#include <QDebug>
#include <QDataStream>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QHostAddress>

class ClientSocket : public QTcpSocket
{
    Q_OBJECT
private:
    quint32 dataSize=0;
    QByteArray dataBuf,res;
    bool successEnd=false;

    void putStr(QDataStream *din,const QString &str);
    QString getStr(QDataStream *din);
    int queryRowCount(QSqlQuery &query);

public:
    ClientSocket(QObject *parent=nullptr);
    ~ClientSocket();

signals:
    void exitServer();

private slots:
    void dataRecv();
    void errorCtrl(QAbstractSocket::SocketError err);
};

#endif // CLIENTSOCKET_H
