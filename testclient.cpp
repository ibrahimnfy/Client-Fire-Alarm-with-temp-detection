#include "testclient.h"
#include <QDebug>
#include <QDateTime>
#include <iostream>

#include <QProtobufSerializer> // bunu da unutma
QElapsedTimer elapsedtimer;
TestClient::TestClient(QString name, QtProtobuf::int64 typeID, QObject *parent)
    : QObject(parent)
    , m_name{name}
    , m_typeID{typeID}
{
    elapsedtimer.start();
    socket = new QTcpSocket(this);
    serializer = new QProtobufSerializer();// BU SATIR EKLENMELİ
    connect(socket, &QTcpSocket::connected, this, &TestClient::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &TestClient::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &TestClient::onDisconnected);
    connect(&m_timer, &QTimer::timeout, this, &TestClient::onTimeout);
}


void TestClient::connectAndSend(const QString &host, quint16 port)
{
    socket->connectToHost(host, port);
}

void TestClient::onConnected()
{
    qDebug() << "[Client] Connected to server.";
    // LOGIN
    chat::Login login;
    login.setName(m_name);
    login.setTypeID(m_typeID);
    login.setTime(5000);

    chat::Wrapper loginWrapper;
    loginWrapper.setLogin(login);

    QByteArray loginBytes = loginWrapper.serialize(serializer);
    quint32 loginLen = loginBytes.size();
    qDebug()<<"11.1";
    QByteArray loginPacket;
    QDataStream stream(&loginPacket, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << loginLen;
    loginPacket.append(loginBytes);

    socket->write(loginPacket);
    socket->flush();
    m_timer.setInterval(1000);
    m_timer.setSingleShot(true);
    m_timer.start();


}

void TestClient::onReadyRead()
{
    buffer.append(socket->readAll());
    qDebug()<< "Buffer : "<< buffer.toHex(' ');

    qDebug() << "11";
    if (onetimebufferremove) {
        if (buffer.size() < 4) return;

        QDataStream stream(buffer);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> expectedSize;

        buffer.remove(0, 4);
        onetimebufferremove = false;

        qDebug() << "Expected size size:" << expectedSize;
    }
    qDebug() << "20";

    while(buffer.size()<expectedSize)  {
        //qDebug()<< buffer.size() << "    "<< expectedSize;
        buffer.append(socket->readAll());
        // qDebug()<< "Buffer : "<< buffer.toHex(' ');

    }
    qDebug() << "21";

    QByteArray data = buffer.left(expectedSize);
    qDebug() << "16";

    buffer.remove(0, expectedSize);
    qDebug() << "17";
    onetimebufferremove=true;




    chat::Wrapper wrapper;
    qDebug() << "19";

    if (wrapper.deserialize(serializer, data)) {
        qDebug() << "10";
        if (wrapper.hasChat()) {
            auto sensor = wrapper.chat().sensors();
            qDebug() << "[Server Message]:" << wrapper.chat().sender() << ":" << sensor.iD()<< QString::number(sensor.value()) << ":"<< QString::number(sensor.typeID());

            // if(wrapper.chat().hasHumi()){
            //     qDebug() << "[Server Message]:" << wrapper.chat().sender() << ":" << wrapper.chat().humi().iD()<< wrapper.chat().humi().value();
            // }
            // else if(wrapper.chat().hasTemp()){
            //     qDebug() << "[Server Message]:" << wrapper.chat().sender() << ":" << wrapper.chat().temp().iD()<< wrapper.chat().temp().value();

            // }
        }
    }

}

void TestClient::onDisconnected()
{
    qDebug()<<"client disconnected from host";
    if(m_timer.isActive()){
        m_timer.stop();
    }
}

void TestClient::onTimeout()
{
    if(counter==100){
        return;
    }
    // CHAT
    chat::Chat chatMsg;
    chatMsg.setSender(m_name);
    chat::Sensor s;
    s.setID("1");
    s.setTypeID(m_typeID);
    qint64 v = sinValue(elapsedtimer);
    s.setValue(v);
    chatMsg.setSensors(s);
    // if(m_typeID==2){
    //     chat::Temperature t;
    //     t.setID("1");
    //     t.setValue(90);
    //     chatMsg.setTemp(t);
    // }
    // else if(m_typeID==3){
    //     chat::Humidity h;
    //     h.setID("1");
    //     h.setValue(90);
    //     chatMsg.setHumi(h);
    // }


    chat::Wrapper chatWrapper;
    chatWrapper.setChat(chatMsg);

    QByteArray chatBytes = chatWrapper.serialize(serializer);
    quint32 chatLen = chatBytes.size();
    qDebug()<<"11.2";;
    QByteArray chatPacket;

    QDataStream stream(&chatPacket, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << chatLen;

    chatPacket.append(chatBytes);

    socket->write(chatPacket);
    socket->flush();
    counter++;
    m_value++;
    m_timer.start();

}
qint64 TestClient::sinValue(const QElapsedTimer& timer,
                double freq, double amp, double phase)
{
    const double t = timer.elapsed() / 1000.0;           // saniye
    const double angle = 2 * M_PI * freq * t + phase;    // rad
    return static_cast<qint64>(amp * qSin(angle));
}

