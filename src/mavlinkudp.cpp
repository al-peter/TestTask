#include "MAVlinkUDP.h"
#include <QByteArray>
#include <QTimer>

MAVlinkUDP::MAVlinkUDP(QObject *parent)
    : QObject(parent),
    udpSocket(new QUdpSocket(this)),
    sendAddress(QHostAddress::LocalHost),
    sendPort(14550),
    socketConnected(false) {}

MAVlinkUDP::~MAVlinkUDP()
{
    disconnectUDP();
}

bool MAVlinkUDP::connectUDP(const QString &ipAddress, quint16 port)
{
    if (ipAddress.isEmpty())
    {
        qDebug() << "Invalid IP address.";
        return false;
    }

    sendAddress = QHostAddress(ipAddress);

    if (udpSocket)
    {
        udpSocket->close();
    }

    udpSocket = new QUdpSocket();

    if (!udpSocket->bind(QHostAddress::AnyIPv4, port))
    {
        qDebug() << "Failed to bind UDP socket:" << udpSocket->errorString();
        qDebug() << "Attempted to bind on " << sendAddress.toString() << ":" << port;
        return false;
    }

    connect(udpSocket, &QUdpSocket::readyRead, this, &MAVlinkUDP::readPendingDatagrams);

    socketConnected = true;
    qDebug() << "Connected to " << sendAddress.toString() << "on sendPort " << sendPort;

    emit connected();
    return true;
}

void MAVlinkUDP::disconnectUDP()
{
    if(udpSocket)
    {
        udpSocket->close();
        delete udpSocket;
        udpSocket = nullptr;
    }

    socketConnected = false;
    emit disconnected();
}

void MAVlinkUDP::requestParameters()
{
    if(!socketConnected)
    {
        qDebug() << "Not connected.";
        return;
    }

    mavlink_message_t msg;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];

    mavlink_msg_param_request_list_pack(1, 1, &msg, 1, 1); // Запит списку параметрів
    //mavlink_msg_param_request_read_pack(1, 1, &msg, 1, 1, "STAT_RUNTIME\0", -1);
    int len = mavlink_msg_to_send_buffer(buffer, &msg);

    udpSocket->writeDatagram(reinterpret_cast<const char*>(buffer), len, sendAddress, sendPort);
    qDebug() << "Sent PARAM_REQUEST_LIST, msg id:" << msg.msgid;
}

void MAVlinkUDP::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        //qDebug() << "Received datagram, size:" << datagram.size();

        mavlink_message_t msg;
        mavlink_status_t status;

        for (int i = 0; i < datagram.size(); i++)
        {
            if(mavlink_parse_char(MAVLINK_COMM_0, static_cast<uint8_t>(datagram[i]), &msg, &status))
            {
                //qDebug() << "MAVLink message parsed with msgid:" << msg.msgid;
                //qDebug() << "Sent PARAM_REQUEST_LIST to" << sendAddress.toString() << ":" << sendPort;
                //qDebug() << "Packet data (hex):" << datagram.toHex();
                processMavlinkMessage(msg);
            }
        }
    }
}

void MAVlinkUDP::processMavlinkMessage(const mavlink_message_t &msg)
{

    if(msg.msgid == MAVLINK_MSG_ID_PARAM_VALUE)
    {
        mavlink_param_value_t param;
        mavlink_msg_param_value_decode(&msg, &param);

        qDebug() << "Received MAVLink PARAM_VALUE:";
        qDebug() << "Param ID:" << param.param_id;
        qDebug() << "Param Value:" << param.param_value;
        qDebug() << "Param Type:" << param.param_type;
        qDebug() << "Param index:" << param.param_index;

        QString paramId = QString::fromUtf8(param.param_id, 16).trimmed();
        QVariant paramValue(param.param_value);

        parameters[paramId] = paramValue;
        emit parameterUpdated(paramId, paramValue);

        if (param.param_count > 0)
        {
            qDebug() << "Total parameters on board:" << param.param_count;
        }

        if (param.param_index + 1 < param.param_count) {
            qDebug() << "Not all parameters received yet, waiting for the next fragment...";
        } else {
            qDebug() << "All parameters received.";
            //emit parametersCompleted();
        }

    }else if(msg.msgid == 0)
    {
        qDebug() << "Got HEARTBEAT";
        qDebug() << "COMP_ID:" << msg.compid << "SYS_ID:" << msg.sysid;
    }
}

void MAVlinkUDP::sendParameterToBoard(const QString &paramId, const QVariant &value) // PARAM_SET (23)
{
    if (!socketConnected) {
        qDebug() << "Not connected.";
        return;
    }

    mavlink_message_t msg;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];

    float paramValue = value.toFloat();
    mavlink_msg_param_set_pack(255, 190, &msg, 1, 1, paramId.toUtf8().data(), paramValue, MAV_PARAM_TYPE_REAL32);
    int len = mavlink_msg_to_send_buffer(buffer, &msg);

    udpSocket->writeDatagram(reinterpret_cast<const char*>(buffer), len, sendAddress, sendPort);
    qDebug() << "Sent parameter:" << paramId << "=" << value;
}
