#ifndef MAVLINKUDP_H
#define MAVLINKUDP_H

#include "mavlink.h"
#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMap>
#include <QString>
#include <QVariant>

class MAVlinkUDP : public QObject // Клас для роботи з MAVLink по UDP
{
    Q_OBJECT // Макрос
public:

    explicit MAVlinkUDP(QObject *parent = nullptr); // Конструктор
    ~MAVlinkUDP(); // Деструктор

    bool connectUDP(const QString &ipAddress, quint16 port); // Підключення по UDP
    void disconnectUDP(); // Відключення від UDP
    void requestParameters(); // Запит списку параметрів PARAM_REQUEST_LIST (21)
    void sendParameterToBoard(const QString &paramId, const QVariant &value); // Надсилання параметра на борт PARAM_SET (23)

signals:
    void parameterUpdated(const QString &paramId, const QVariant &value); // Сигнал оновлення параметра
    void connected(); // Сигнал підключення по UDP
    void disconnected(); // Сигнал відключення від UDP
    //void parametersCompleted();

private slots:
    void readPendingDatagrams(); // Обробляє вхідні UDP-датаграми - зчитує всі доступні датаграми

private:
    void processMavlinkMessage(const mavlink_message_t &msg); // Обробка MAVLink-повідомлення

    QUdpSocket *udpSocket;
    QHostAddress sendAddress;
    quint16  sendPort;
    bool socketConnected; // Статус з'єднання
    QMap<QString, QVariant> parameters;
};

#endif // MAVLINKUDP_H
