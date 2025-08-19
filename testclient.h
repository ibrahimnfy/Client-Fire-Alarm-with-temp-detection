#ifndef TESTCLIENT_H
#define TESTCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QProtobufSerializer>
#include "message.qpb.h"
#include <QTimer>
#include <QtMath>
#include <QElapsedTimer>

class TestClient : public QObject
{
    Q_OBJECT
public:
    explicit TestClient(QString name, QtProtobuf::int64 typeID, QObject *parent = nullptr);
    void connectAndSend(const QString &host, quint16 port);
    QTimer timeoutDuration;


private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onTimeout();
    qint64 sinValue(const QElapsedTimer& timer,
                    double freq=0.11, double amp=100.0, double phase=3.0);

private:
    int counter = 0;
    int m_value;
    QString m_name;
    QtProtobuf::int64 m_typeID;
    QTcpSocket *socket;
    QProtobufSerializer *serializer;
    QByteArray buffer;
    bool onetimebufferremove=true;
    quint32 expectedSize= 0;
    QTimer m_timer;
};


#endif // TESTCLIENT_H
