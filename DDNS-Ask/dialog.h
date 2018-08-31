#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QSettings>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QDataStream>
#include <QByteArray>
#include <QMessageBox>
#include <QHostAddress>
#include <QKeyEvent>
#include <QInputDialog>
#include <QCryptographicHash>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::Dialog *ui;

    QTcpSocket s;
    bool is_list,is_quit;
    quint32 dataSize;
    QByteArray dat;
    int tCount=0;
    bool enableClose=false;

    void readConf();
    void writeConf();
    void disableInput();
    void enableInput();
    void tClear();
    void tAdd(QString name,QString addr);
    void setMsg(QString str);
    void init();

    void putStr(QDataStream *din,const QString &str);
    QString getStr(QDataStream *din);

private slots:
    void errorCtrl(QAbstractSocket::SocketError error);
    void dataRecv();
    void sendAsk();
    void endAsk();
    void on_lineEdit_server_textChanged(const QString &str);
    void on_lineEdit_name_textChanged(const QString &str);
    void on_pushButton_ask_clicked();
    void on_pushButton_list_clicked();
    void on_pushButton_close_clicked();
};

#endif // DIALOG_H
