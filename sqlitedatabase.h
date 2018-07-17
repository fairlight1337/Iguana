#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H


#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QVariant>
#include <QDateTime>

#include <iostream>

#include <database.h>


class SQLiteDatabase : public Database
{
public:
    SQLiteDatabase(const QString& databaseFile);
    ~SQLiteDatabase() override = default;

    bool Initialize() override;

    void RegisterFile(const QString& fileName, const QString& hash) override;
    void SetTagForFile(const QString& fileName, const QString& tag, bool isSet) override;
    bool FileHasTag(const QString& fileName, const QString& tag) override;
    bool FileWithHashIsRegistered(const QString& hash) override;
    bool FileWithFileNameIsRegistered(const QString& fileName) override;
    QStringList GetFilesAfterFile(const QString& fileName, int amount) override;
    QStringList GetFilesBeforeFile(const QString& fileName, int amount) override;
    QDateTime GetDateForFile(const QString& fileName) override;
    QString GetFirstImage() override;

    void AddTag(const QString& tag) override;
    void DeleteTag(const QString& tag) override;
    QStringList GetTags() override;

private:
    QSqlDatabase mSqlDatabase;
};

#endif // SQLITEDATABASE_H
