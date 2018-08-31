#include "clientsocket.h"

#define DB_PATH "./record.dat"
#define DATA_VER QDataStream::Qt_5_10
#define SOCK_ID socketDescriptor()

ClientSocket::ClientSocket(QObject *parent)
    :QTcpSocket(parent)
{
    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(errorCtrl(QAbstractSocket::SocketError)));
    connect(this,SIGNAL(readyRead()),this,SLOT(dataRecv()));
}

ClientSocket::~ClientSocket()
{
    if(successEnd)
        qDebug() << "【连接处理完毕】成功结束\n";
    else
        qDebug() << "【连接处理完毕】非正常退出!\n";
}

void ClientSocket::errorCtrl(SocketError err)
{
    if(err == QAbstractSocket::RemoteHostClosedError)
        qDebug() << "【消息】对方连接已关闭";
    else
        qDebug() << "【错误】连接" << SOCK_ID << "发生错误：\n" << errorString();
}

QString ClientSocket::getStr(QDataStream *din)
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

void ClientSocket::putStr(QDataStream *dout, const QString &str)
{
    foreach (QChar c, str) {
        (*dout) << c;
    }
}

int ClientSocket::queryRowCount(QSqlQuery &query)
{
    int lastPos=query.at();

    int pos=0;
    if (query.last())
        pos=query.at()+1;
    else
        pos=0;

    query.seek(lastPos);
    return pos;
}

void ClientSocket::dataRecv()
{
    if(dataSize == 0)
    {
        if(bytesAvailable() < (int)sizeof(quint32))
            return;
        QDataStream din(this);
        din.setVersion(DATA_VER);
        din >> dataSize;
    }

    if(bytesAvailable() < dataSize)
        return;

    dataBuf=this->readAll();
    qDebug() << "连接" << SOCK_ID << "接收数据完毕。开始处理...";

    QDataStream din(&dataBuf,QIODevice::ReadOnly);
    QChar todo;
    din >> todo;

    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
        db = QSqlDatabase::database("qt_sql_default_connection");
    else
        db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(DB_PATH);

    try
    {
        if(todo == QChar('Q'))
        {
            qDebug() << "连接" << SOCK_ID << "要求关闭服务器（Q）";
            successEnd=true;
            emit exitServer();
            disconnectFromHost();
            return;
        }

        else
        {
            if(!db.open())
                throw "打开数据库失败！";

            QDataStream dout(&res,QIODevice::WriteOnly);
            dout.setVersion(DATA_VER);
            dout << (quint32)0;

            qDebug() << "预处理：统计总行数";
            QSqlQuery query;
            if(!query.exec("select * from record"))
                throw "预处理select失败！";
            quint32 qRecordSize=queryRowCount(query);
            qDebug() << "预处理完成。总行数：" <<qRecordSize;

            if(todo == QChar('L'))
            {
                qDebug() << "连接" << SOCK_ID << "进入列表模式（T）";

                dout << QChar('T') << (quint32)qRecordSize;
                while(query.next())
                    putStr(&dout,query.value("name").toString() + QChar(' ') + query.value("addr").toString() + QChar(' '));
            }

            else
            {
                qDebug() << "连接" << SOCK_ID << "未请求全部数据。开始筛选...";
                QString name(getStr(&din));

                query.prepare("select * from record where name=:name");
                query.bindValue(":name",name);
                if(!query.exec())
                    throw "预处理select失败！";

                bool is_exist=query.next();
                qDebug() << "连接" << SOCK_ID << "预处理完成。开始更新数据...";

                if(todo == QChar('S'))
                {
                    if(!is_exist)       //new
                    {
                        qDebug() << "连接" << SOCK_ID << "进入插入模式（S）";

                        QString sql="insert into record values("+QVariant(qRecordSize+1).toString()+",:name,:addr)";
                        query.finish();
                        query.prepare(sql);
                        query.bindValue(":name",name);
                        query.bindValue(":addr",peerAddress().toString());

                        if(!query.exec())
                            throw "更新insert失败！";
                    }
                    else            //update
                    {
                        qDebug() << "连接" << SOCK_ID << "进入更新模式（S）";
                        QSqlQuery q;
                        q.prepare("update record set addr=:addr where name=:name");
                        q.bindValue(":name",name);
                        q.bindValue(":addr",peerAddress().toString());

                        if(!q.exec())
                            throw "更新update失败！";
                    }
                    //dout << QChar('D');
                    successEnd=true;
                    db.close();
                    disconnectFromHost();
                    return;
                }

                else if(todo == QChar('A'))
                {
                    qDebug() << "连接" << SOCK_ID << "进入查询模式（A）";
                    if(!is_exist)
                        dout << QChar('N');
                    else
                    {
                        dout << QChar('R');
                        putStr(&dout,query.value("addr").toString());
                    }
                }

                else
                {
                    throw "收到未知请求！";
                }
            }

            dout.device()->seek(0);
            dout << (quint32)(res.size()-sizeof(quint32));

            if(!write(res))
                throw "返回数据失败！";
            flush();

            successEnd=true;
            qDebug() << "连接" << SOCK_ID << "成功返回数据。";
        }
    }
    catch(const char *err_str)
    {
        qDebug() << "【错误】连接" << SOCK_ID << QString(err_str);
        disconnectFromHost();
    }
    catch(...)
    {
        qDebug() << "【错误】连接" << SOCK_ID << "遭遇未知错误！";
        disconnectFromHost();
    }

    if(db.isOpen())
        db.close();
}
