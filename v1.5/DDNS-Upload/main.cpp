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
#include <QSettings>
#include <QDir>
#include "ddns_upload.h"

#define NAME "DDNS-Upload"
#define VER "1.5"

#define REG "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REG_NAME "dCHost"
#define GIT "https://github.com/yqs112358/DDNS-Tools"

#define CONF_BAK ":/backup/conf.dat"
#define CONF_PATH "./conf.dat"
#define LICENSE_BAK ":/backup/LICENSE"

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
    QCoreApplication::setApplicationName(NAME);
    QCoreApplication::setApplicationVersion(VER);

    QCommandLineParser p;
    p.setApplicationDescription("Server of DDNS Tools (By @yqs112358)");
    p.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    p.addVersionOption();
    p.addHelpOption();

    QCommandLineOption logOption(QStringList() << "o" << "output",QCoreApplication::translate("main","Output the log into <file>")
                                 ,QCoreApplication::translate("output.log","file"));
    p.addOption(logOption);

    QCommandLineOption setStartup(QStringList() << "i" << "install"
                                  ,QCoreApplication::translate("main","Enable the startup"));
    QCommandLineOption clearStartup(QStringList() << "u" << "uninstall"
                                  ,QCoreApplication::translate("main","Disable the startup"));
    p.addOption(setStartup);
    p.addOption(clearStartup);

    QCommandLineOption license(QStringList() << "l" << "license"
                                  ,QCoreApplication::translate("main","Show the LGPL-3.0 license"));
    p.addOption(license);

    QCommandLineOption git(QStringList() << "g" << "git"
                                  ,QCoreApplication::translate("main","To get source from Github"));
    p.addOption(git);

    p.process(a);

    //开始处理命令行参数
    //--output
    if(p.isSet(logOption))
    {
        log_path=p.value(logOption);
        qDebug() << "重定向输出至：" << log_path;

        QFile file(log_path);
        file.open(QIODevice::WriteOnly);
        QTextStream dout(&file);
        dout << QString("【") << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << QString("】") << "\r\n";
        file.flush();
        file.close();

        qInstallMessageHandler(outputMessage);
    }

    //--license
    if(p.isSet(license))
    {
        QFile fBak(LICENSE_BAK);
        fBak.open(QIODevice::ReadOnly);
        qDebug() << qPrintable(fBak.readAll());
        fBak.close();

        return 0;
    }

    //--install --uninstall
    bool isSet=p.isSet(setStartup),isClear=p.isSet(clearStartup);
    if(isSet || isClear)
    {
        QSettings s(REG,QSettings::NativeFormat);
        if(isSet)
        {
            qDebug() << "安装启动项...";
            QString val(QDir::toNativeSeparators(a.applicationFilePath()));
            if(!log_path.isEmpty())
                val+=" -o "+log_path;
            s.setValue(REG_NAME,val);
        }
        else if(isClear)
        {
            qDebug() << "卸载启动项...";
            s.remove(REG_NAME);
        }
        s.sync();

        switch(s.status())
        {
        case QSettings::NoError:
            qDebug() << "处理成功！";
            break;
        case QSettings::AccessError:
            qDebug() << "【错误】权限不足！";
            break;
        case QSettings::FormatError:
            qDebug() << "【错误】格式错误！";
            break;
        }

        return 0;
    }

    //--github
    if(p.isSet(git))
    {
        qDebug() << "GitHub地址：" << GIT;

        return 0;
    }

    //conf
    if(!QFile::exists(CONF_PATH))
    {
        qDebug() << "配置文件不存在！正在释放...";
        QFile fBakDB(CONF_BAK);
        QFile fReleaseDB(CONF_PATH);
        if(!fBakDB.open(QIODevice::ReadOnly))
        {
            qDebug() << "资源文件打开失败！\n启动失败！";
            return 1;
        }

        if(!fReleaseDB.open(QIODevice::WriteOnly))
        {
            qDebug() << "配置文件创建失败！\n启动失败！";
            return 1;
        }

        fReleaseDB.write(fBakDB.readAll());
        fReleaseDB.close();
        fBakDB.close();
        qDebug() << "释放成功。\n";
    }

    DDNS_Upload cli;

    return a.exec();
}
