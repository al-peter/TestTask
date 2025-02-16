#ifndef PARAMSTORAGE_H
#define PARAMSTORAGE_H

#include <QObject>
#include <QMap>
#include <QString>

class ParamStorage : public QObject // Клас для зберігання та управління параметрами
{
    Q_OBJECT // Макрос

public:
    explicit ParamStorage(QObject *parent = nullptr); // Конструктор
    ~ParamStorage(); // Деструктор

    void addParam(const QString &key, const QVariant &value); // Додати параметр (сеттер)
    void removeParam(const QString &key);  // Видалити параметр
    void clearParams(); // Очистка всіх полей параметрів

    QMap<QString, QVariant> getParams() const;

    void saveToFile(const QString &filename); // Збереження параметрів у файл
    void loadFromFile(const QString &filename); // Завантаження параметрів файлу


signals:
    void paramChanged(const QString &key, const QVariant &value); // Сигнал зі зміною параметру
    void paramsCleared(); // // Сигнал очищення параметрів

private:
    QMap<QString, QVariant> params; // Словник параметрів

#endif // PARAMSTORAGE_H
};
