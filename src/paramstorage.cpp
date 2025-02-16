#include "paramstorage.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringList>
#include <QVariant>

ParamStorage::ParamStorage(QObject *parent)
    : QObject(parent)
{
}

ParamStorage::~ParamStorage()
{
    //clearParams(); // очищення всіх параметрів
    qDebug() << "ParamStorage destructor called";
}

void ParamStorage::addParam(const QString &key, const QVariant &value)
{
    params[key] = value; // Додавання параметру
    emit paramChanged(key, value); // Сигнал про зміну (додавання) параметра
    qDebug() << "Added parameter: " << key << "=" << value;
}

void ParamStorage::removeParam(const QString &key)
{
    if(params.contains(key))
    {
        params.remove(key); // Видалення параметру
        emit paramChanged(key, QVariant()); // Сигнал про зміну (видалення) параметра
        qDebug() << "Removed parameter: " << key;
    }
}

void ParamStorage::clearParams()
{
    params.clear(); // Очищення всіх параметрів
    qDebug() << "params after clear:" << params; // Перевірка чи пуст QMap
    emit paramsCleared(); // Сигнал про видалення всіх параметрів

    //saveToFile("params.ini"); // одразу зберігаємо порожній файл
    //emit paramChanged("", ""); // сигнал про очищення

    //qDebug() << "Усі параметри були очищені";
}

QMap<QString, QVariant> ParamStorage::getParams() const
{
    return params;
}

/*QString ParamStorage::getParam(const QString &key) const
{
    return params.value(key, "");  // повернення значення параметра або порожній рядок
}
*/

void ParamStorage::saveToFile(const QString &filename)
{
    qDebug() << "Saving parameters to file. Current params:" << params;
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {  // Перевірка на можливість відкриття файлу для запису
        qDebug() << "Error opening file for writing: " << filename;
        return;
    }

    qDebug() << "Saving parameters to file: " << filename;
    QTextStream out(&file);
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        out << it.key() << "=" << it.value().toString() << "\n";  // Фактичне збереження у файл
        qDebug() << "Saved parameter: " << it.key() << "=" << it.value();
    }
    file.close();
    qDebug() << "Parameters saved to file successfully.";
}

void ParamStorage::loadFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {  // Перевірка на можливість відкриття файлу для читання
        qDebug() << "Error opening " << filename << " for reading";
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList param = line.split("=");
        if (param.size() == 2)
        {
            addParam(param[0], param[1]);  // Ініціалізація/додавання параметра у сховище
        } else
        {
            qDebug() << "Invalid string format: " << line;
        }
    }
    file.close();
}
