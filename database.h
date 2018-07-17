#ifndef DATABASE_H
#define DATABASE_H


#include <QString>
#include <QVector>
#include <QDateTime>


class Database
{
public:
    Database() = default;
    virtual ~Database() = default;

    virtual bool Initialize() = 0;

    virtual void RegisterFile(const QString& fileName, const QString& hash) = 0;
    virtual void SetTagForFile(const QString& fileName, const QString& tag, bool isSet) = 0;
    virtual bool FileHasTag(const QString& fileName, const QString& tag) = 0;
    virtual bool FileWithHashIsRegistered(const QString& hash) = 0;
    virtual bool FileWithFileNameIsRegistered(const QString& fileName) = 0;
    virtual QStringList GetFilesAfterFile(const QString& fileName, int amount) = 0;
    virtual QStringList GetFilesBeforeFile(const QString& fileName, int amount) = 0;
    virtual QDateTime GetDateForFile(const QString& fileName) = 0;
    virtual QString GetFirstImage() = 0;

    virtual void AddTag(const QString& tag) = 0;
    virtual void DeleteTag(const QString& tag) = 0;
    virtual QStringList GetTags() = 0;
};


#endif // DATABASE_H
