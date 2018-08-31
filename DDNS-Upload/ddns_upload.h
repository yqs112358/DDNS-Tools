#ifndef DDNS_UPLOAD_H
#define DDNS_UPLOAD_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QSettings>
#include <QString>
#include <QFile>
#include <QThread>
#include <QTimer>
#include <cstdlib>

class DDNS_Upload : public QObject
{
    Q_OBJECT
public:
    explicit DDNS_Upload(QObject *parent = nullptr);
    ~DDNS_Upload();
    QTcpSocket s;
    QTimer t;

private:
    void putStr(QDataStream *din,const QString &str);

signals:

public slots:
    void sendData();
    void errorCtrl(QAbstractSocket::SocketError err);
};

#endif // DDNS_UPLOAD_H
