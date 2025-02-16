#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    tableWidget(new QTableWidget(this)),
    paramStorage(new ParamStorage(this)),    //this == MainWindow
    mavlink(new MAVlinkUDP(this)),
    dataSaved(false)
{
    setWindowTitle("Test Task");

    // Налаштування таблиці
    tableWidget->setRowCount(0);
    tableWidget->setColumnCount(2);
    tableWidget->setHorizontalHeaderLabels(QStringList() << "Parameter" << "Value");
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Створення основного лейауту
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Поля для IP і порту для MAVLink
    ipAddressInput = new QLineEdit(this);
    ipAddressInput->setPlaceholderText("Enter IP Address");
    ipAddressInput->setText("127.0.0.1"); // Значення за замовчуванням

    portInput = new QLineEdit(this);
    portInput->setPlaceholderText("Enter Port");
    portInput->setText("14550"); // Значення за замовчуванням

    // Кнопка підключення
    connectButton = new QPushButton("Connect", this);
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::connectToMAVlink);/*[this]() {
        QString ip = ipAddressInput->text();
        int port = portInput->text().toInt();
        mavlink->connectUDP(ip, port);
        connectButton->setEnabled(false);
        disconnectButton->setEnabled(true);
        qDebug() << "Connected to MAVlink at " << ip << ":" << port;
    });*/

    // Кнопка відключення
    disconnectButton = new QPushButton("Disconnect", this);
    disconnectButton->setEnabled(false); // Спочатку вимкнена
    connect(disconnectButton, &QPushButton::clicked, this, &MainWindow::disconnectFromMAVlink);/*[this]() {
        mavlink->disconnectUDP();
        connectButton->setEnabled(true);
        disconnectButton->setEnabled(false);
        qDebug() << "Disconnected from MAVlink.";
    });*/

    // Поле для пошуку параметра
    paramSearchInput = new QLineEdit(this);
    paramSearchInput->setPlaceholderText("Search for parameter");

    // Кнопка пошуку
    QPushButton *searchButton = new QPushButton("Search Parameter", this);
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::searchParameter);

    // Кнопка для запиту параметрів з борту
    requestParamsButton = new QPushButton("Request Parameters", this);
    connect(requestParamsButton, &QPushButton::clicked, this, &MainWindow::requestParamsFromBoard);

    // Кнопка для надсилання параметрів на борт
    sendParamsButton = new QPushButton("Send Parameter", this);
    connect(sendParamsButton, &QPushButton::clicked, this, &MainWindow::sendParamsToBoard);

    // Додаємо поля вводу IP і порту в лейаут, і кнопки Connect/Disconnect
    mainLayout->addWidget(ipAddressInput);
    mainLayout->addWidget(portInput);
    mainLayout->addWidget(connectButton);
    mainLayout->addWidget(disconnectButton);

    //Додаємо кнопки для запиту параметрів і відправки на борт
    mainLayout->addWidget(requestParamsButton);
    mainLayout->addWidget(sendParamsButton);

    // Додаємо таблицю з параметрами
    mainLayout->addWidget(tableWidget);

    // Поле для введення параметра
    QLineEdit *paramKeyInput = new QLineEdit(this);
    paramKeyInput->setPlaceholderText("Enter Parameter Key");

    // Поле для введення значення
    QLineEdit *paramValueInput = new QLineEdit(this);
    paramValueInput->setPlaceholderText("Enter Parameter Value");

    // Кнопка для додавання параметру із значенням
    QPushButton *addButton = new QPushButton("Add Parameter", this);
    connect(addButton, &QPushButton::clicked, this, [this, paramKeyInput, paramValueInput]() {
        // Логіка для додавання параметра
        QString key = paramKeyInput->text();
        QString value = paramValueInput->text();
        if (!key.isEmpty() && !value.isEmpty()) {
            paramStorage->addParam(key, value);
            paramKeyInput->clear();
            paramValueInput->clear();
        }
    });

    QPushButton *clearButton = new QPushButton("Clear Table", this);
    connect(clearButton, &QPushButton::clicked, this, [this]() { // Очистка всього table
        paramStorage->clearParams();
    });

    QPushButton *removeButton = new QPushButton("Remove Selected", this);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeSelectedParam); // Видалення обраного параметра

    // Кнопка для збереження параметрів у файл
    QPushButton *saveButton = new QPushButton("Save Config File", this);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveParamsToFile);

    // Кнопка для завантаження параметрів з файлу
    QPushButton *loadButton = new QPushButton("Load Сonfig File", this);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadParamsFromFile);

    // Додаємо поля для введення/видалення/пошуку параметра/ів та відповідні кнопки
    mainLayout->addWidget(paramKeyInput);
    mainLayout->addWidget(paramValueInput);
    mainLayout->addWidget(paramSearchInput);
    mainLayout->addWidget(addButton);
    mainLayout->addWidget(removeButton);
    mainLayout->addWidget(searchButton);
    mainLayout->addWidget(clearButton);

    // Додаємо кнопки для збереження і завантаження
    mainLayout->addWidget(saveButton);  // Кнопка збереження
    mainLayout->addWidget(loadButton);  // Кнопка завантаження

    // Створюємо кнопку для виходу
    QPushButton *exitButton = new QPushButton("Exit", this);
    mainLayout->addWidget(exitButton);
    connect(exitButton, &QPushButton::clicked, this, &QWidget::close);

    // Центральний віджет
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Підключення сигналів від сховища параметрів
    connect(paramStorage, &ParamStorage::paramChanged, this, &MainWindow::onParamChanged);
    connect(paramStorage, &ParamStorage::paramsCleared, this, &MainWindow::onParamsCleared);

    // Зв'язуємо сигнал про отримання параметра від MAVlink із нашим слотом
    connect(mavlink, &MAVlinkUDP::parameterUpdated, this, &MainWindow::onMAVLinkMessageReceived);
}

// Реалізація деструктора
MainWindow::~MainWindow()
{
    delete tableWidget; // Видалення об'єкту tableWidget і звільнення пам'яті
    //delete paramStorage; // є дочірнім об'єктом, його не потрібно видаляти окремо
    delete mavlink; // Видалення об'єкту mavlink і звільнення пам'яті
}

void MainWindow::connectToMAVlink() // Підключення до бортової системи через MAVLink
{
    QString ipAddress = ipAddressInput->text(); // Отримаємо IP із поля вводу
    quint16 port = portInput->text().toUShort(); // Отримаємо port із поля вводу

    if(mavlink->connectUDP(ipAddress, port)) // Намагаємось підключитися по UDP
    {
        connectButton->setEnabled(false);
        disconnectButton->setEnabled(true);
        qDebug() << "Connected to receive MAVlink at " << ipAddress << ":" << port;
    }
    else
    {
        QMessageBox::critical(this, "Connection Error", "Failed to connect to MAVLink.");
    }
}

void MainWindow::disconnectFromMAVlink() // Логіка для відключення від MAVLink
{
    mavlink->disconnectUDP();
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
    qDebug() << "Disconnected from MAVlink.";
}

void MainWindow::onMAVLinkMessageReceived(const QString &paramId, const QVariant &value)
{
    // Обробка повідомлення MAVLink
    qDebug() << "Received param: " << paramId << " with value: " << value;
    paramStorage->addParam(paramId, value.toString()); // Додаємо параметр до сховища
    onParamChanged(paramId, value.toString());  // Оновлюємо таблицю з параметрами
    updateUIWithParams();
}

void MainWindow::updateUIWithParams()
{
    qDebug() << "Updating UI with parameters...";
    tableWidget->setRowCount(0);  // Очищаємо таблицю перед додаванням нових даних

    // Отримуємо параметри з paramStorage
    QMap<QString, QVariant> params = paramStorage->getParams();

    // Додаємо дані до таблиці
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);  // Додаємо новий рядок
        tableWidget->setItem(row, 0, new QTableWidgetItem(it.key()));  // Ключ (ім'я параметра)
        tableWidget->setItem(row, 1, new QTableWidgetItem(it.value().toString()));  // Значення параметра
    }
}

void MainWindow::sendParamsToBoard() // Логіка для надсилання параметрів на борт
{
    QMap<QString, QVariant> params = paramStorage->getParams();
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        mavlink->sendParameterToBoard(it.key(), it.value());
    }
}

void MainWindow::requestParamsFromBoard() // Логіка для запиту параметрів з борту через MAVLink
{
    qDebug() << "Requesting parameters from the board...";
    mavlink->requestParameters();  // Надсилаємо запит для отримання списку параметрів
}

void MainWindow::searchParameter()
{
    QString searchKey = paramSearchInput->text().trimmed();  // Отримаємо ключ для пошуку
    if (!searchKey.isEmpty())
    {
        bool found = false;
        tableWidget->clearSelection();

        for (int i = 0; i < tableWidget->rowCount(); ++i)
        {
            QString paramkey = tableWidget->item(i,0)->text();

            if (paramkey.contains(searchKey, Qt::CaseInsensitive))
            {
                tableWidget->item(i, 0)->setSelected(true);
                found = true;
            }
        }
        if(!found)
        {
            QMessageBox::information(this, "Search Result", "No matching parameter found.");
        }
    }
}

void MainWindow::onParamChanged(const QString &key, const QVariant &value)
{
    // Логіка оновлення таблиці

    /*if (key.isEmpty() && value.isEmpty()) {
        onParamsCleared();
        return;
    }*/

    if (!value.isValid()) { // Перевірка на пусте значення
        for (int row = 0; row < tableWidget->rowCount(); ++row)
        {
            if (tableWidget->item(row, 0)->text() == key)
            {
                tableWidget->removeRow(row);  // Видалення рядка із ключем
                return;
            }
        }
    } else
    {
    int row = tableWidget->rowCount(); //Поточна кількість рядків у tableWidget + row є індексом наступної строки для додавання даних
    tableWidget->insertRow(row); // Вставка нового пустого рядка у таблицю tableWidget на місце row
    tableWidget->setItem(row, 0, new QTableWidgetItem(key)); // Вставляє новий QTableWidgetItem із значенням key в першу колонку нової строки
    tableWidget->setItem(row, 1, new QTableWidgetItem(value.toString())); // Вставляє новий QTableWidgetItem із значенням value в другу колонку нової строки
    }
}

void MainWindow::onParamsCleared() // Очищає таблицю в UI (не торкається параметрів)
{
    //qDebug() << "onParamsCleared() called!";
    qDebug() << "Clearing all parameters...";
    tableWidget->setRowCount(0);  // Очищення таблиці
    qDebug() << "Table cleared";
}

void MainWindow::removeSelectedParam() // Видаляє параметр з таблиці в UI
{
    int row = tableWidget->currentRow();
    if (row >= 0)
    {
        QString key = tableWidget->item(row, 0)->text();
        paramStorage->removeParam(key);

        // Не викликаємо tableWidget->removeRow(row),
        // адже це зробить слот onParamChanged()
    }
}

void MainWindow::saveParamsToFile() // Викликає збереження параметрів у файл
{
    // Відкриття діалогу вибору файлу для збереження
    QString filename = QFileDialog::getSaveFileName(this, "Save Parameters", "", "Text Files (*.txt);;INI Files (*.ini)");
    if (!filename.isEmpty())
    {
        paramStorage->saveToFile(filename);  // Викликає функцію збереження з ParamStorage для запису параметрів у файл
        dataSaved = true;
    }
}

void MainWindow::loadParamsFromFile() // Викликає завантаження параметрів із файлу
{
    // Відкриття діалогу вибору файлу для завантаження
    QString filename = QFileDialog::getOpenFileName(this, "Load Parameters", "", "Text Files (*.txt);;INI Files (*.ini)");
    if (!filename.isEmpty())
    {
        paramStorage->loadFromFile(filename);  // Викликає функцію завантаження з ParamStorage для читання параметрів файлу
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!dataSaved)
    {
        // Якщо дані не збережені виконуємо далі
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Unsaved Changes", "Do you want to save changes before exiting?",
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (reply == QMessageBox::Yes)
        {
            saveParamsToFile();  // Зберігаємо дані
            //qDebug() << "Closing application...";
            event->accept();  // Закриваємо програму
        } else if (reply == QMessageBox::No)
        {
            //qDebug() << "Closing application...";
            event->accept();  // Закриваємо програму без збереження
        } else
        {
            event->ignore();  // Скасуємо закриття
        }
    } else
    {
        qDebug() << "Closing application...";
        event->accept();  // Якщо дані вже збережені - просто закриваємо програму
    }
}
