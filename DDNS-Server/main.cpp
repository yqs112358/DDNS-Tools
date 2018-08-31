#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStringList>
#include <QtDebug>
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QString>
#include "ddns_server.h"

QString log_path;

void outputMessage(QtMsgType, const QMessageLogContext&, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QFile file(log_path);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream dout(&file);
    dout << msg << "\r\n";
    file.flush();
    file.close();

    mutex.unlock();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("DDNS-Server");
    QCoreApplication::setApplicationVersion("1.1");

    QCommandLineParser p;
    p.setApplicationDescription("Server of DDNS Tools (By @yqs112358)");
    p.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    p.addVersionOption();
    p.addHelpOption();
    QCommandLineOption logOption(QStringList() << "l" << "log",QCoreApplication::translate("main","Output the log into <file>")
                                 ,QCoreApplication::translate("output.log","file"));
    p.addOption(logOption);
    p.process(a);

    if(p.isSet(logOption))
    {
        log_path=p.value(logOption);

        QFile file(log_path);
        file.open(QIODevice::WriteOnly);
        QTextStream dout(&file);
        dout << QString("【") << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << QString("】") << "\r\n";
        file.flush();
        file.close();

        qInstallMessageHandler(outputMessage);
    }

    DDNS_Server server;

    return a.exec();
}
