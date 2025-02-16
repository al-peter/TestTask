#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include "paramstorage.h"
#include "mavlinkudp.h"

class MainWindow : public QMainWindow // Клас для відображення параметрів у вигляді таблиці (UI)
{
    Q_OBJECT // Макрос (MOC)

public:
    explicit MainWindow(QWidget *parent = nullptr); // Конструктор
    ~MainWindow(); // Деструктор

private slots:
    void onParamChanged(const QString &key, const QVariant &value); // Слот до сигналу paramChanged
    void onParamsCleared();  // Слот до сигналу paramsCleared()
    void removeSelectedParam(); // Слот для видалення параметру
    void saveParamsToFile();  // Новий слот для збереження
    void loadParamsFromFile(); // Новий слот для завантаження

    void connectToMAVlink(); // Слот для підключення до MAVLink
    void disconnectFromMAVlink(); // Слот для відключення від MAVLink
    void onMAVLinkMessageReceived(const QString &paramId, const QVariant &value); // Обробник MAVLink повідомлень
    void sendParamsToBoard(); // Слот для надсилання параметрів на борт
    void requestParamsFromBoard(); // Слот для запиту параметрів з борту
    void searchParameter(); // Слот для пошуку параметра в таблиці

private:
    QTableWidget *tableWidget; // Відображення параметрів
    ParamStorage *paramStorage; // Сховище параметрів
    MAVlinkUDP *mavlink; // Об'єкт для з'єднання через MAVLinkUDP

    //елементи для введення IP та порту
    QLineEdit *ipAddressInput;
    QLineEdit *portInput;
    QLineEdit *paramSearchInput; // Поле для пошуку параметра
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QPushButton *requestParamsButton; // Кнопка запиту параметрів від борту
    QPushButton *sendParamsButton;    // Кнопка надсилання параметрів на борт

    bool dataSaved = false; // Змінна для відстеження збереження даних
    void closeEvent(QCloseEvent *event) override; // Обробник закриття вікна

    void updateUIWithParams();  // Оголошення методу оновлення UI з параметрами
};
#endif // MAINWINDOW_H
