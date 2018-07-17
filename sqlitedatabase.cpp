#include "sqlitedatabase.h"


SQLiteDatabase::SQLiteDatabase(const QString& databaseFile)
    : mSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"))
{
    mSqlDatabase.setDatabaseName(databaseFile);
}

bool SQLiteDatabase::Initialize()
{
    bool success = false;

    if(mSqlDatabase.open())
    {
        if(!mSqlDatabase.tables().contains(QLatin1String("Files")))
        {
            QSqlQuery query(mSqlDatabase);
            query.exec("CREATE TABLE Files (fileName TEXT PRIMARY KEY, hash TEXT, date TEXT)");
            query.exec("CREATE TABLE Tags (tagName TEXT PRIMARY KEY)");
            query.exec("CREATE TABLE FileTags (fileName TEXT, tagName TEXT)");
        }

        success = true;
    }

    return success;
}

void SQLiteDatabase::RegisterFile(const QString& fileName, const QString& hash)
{
    QString date = QDateTime::currentDateTime().toString(Qt::ISODateWithMs); // ISO 8601

    QSqlQuery query(mSqlDatabase);
    query.prepare("INSERT INTO Files (fileName, hash, date) VALUES (:fileName, :hash, :date)");
    query.bindValue(":fileName", QVariant(fileName));
    query.bindValue(":hash", QVariant(hash));
    query.bindValue(":date", QVariant(date));
    query.exec();
}

void SQLiteDatabase::SetTagForFile(const QString& fileName, const QString& tag, bool isSet)
{
    QSqlQuery query(mSqlDatabase);
    query.prepare("DELETE FROM FileTags WHERE fileName = (:fileName) AND tagName = (:tagName)");
    query.bindValue(":fileName", QVariant(fileName));
    query.bindValue(":tagName", QVariant(tag));
    query.exec();

    if(isSet)
    {
        query.prepare("INSERT INTO FileTags (fileName, tagName) VALUES (:fileName, :tagName)");
        query.bindValue(":fileName", QVariant(fileName));
        query.bindValue(":tagName", QVariant(tag));
        query.exec();

        std::cout << query.lastQuery().toUtf8().constData() << std::endl;
    }
}

bool SQLiteDatabase::FileHasTag(const QString& fileName, const QString& tag)
{
    QSqlQuery query(mSqlDatabase);
    query.prepare("SELECT * FROM FileTags WHERE fileName = (:fileName) AND tagName = (:tagName)");
    query.bindValue(":fileName", QVariant(fileName));
    query.bindValue(":tagName", QVariant(tag));
    query.exec();

    return query.first();
}

bool SQLiteDatabase::FileWithHashIsRegistered(const QString& hash)
{
    QSqlQuery query(mSqlDatabase);
    query.prepare("SELECT * FROM Files WHERE hash = (:hash)");
    query.bindValue(":hash", QVariant(hash));
    query.exec();

    return query.first();
}

bool SQLiteDatabase::FileWithFileNameIsRegistered(const QString& fileName)
{
    QSqlQuery query(mSqlDatabase);
    query.prepare("SELECT * FROM Files WHERE fileName = (:fileName)");
    query.bindValue(":fileName", QVariant(fileName));
    query.exec();

    return query.first();
}

QStringList SQLiteDatabase::GetFilesAfterFile(const QString& fileName, int amount)
{
    QDateTime fileDate = GetDateForFile(fileName);

    QSqlQuery query(mSqlDatabase);
    query.prepare("SELECT fileName FROM Files WHERE date > (:date) ORDER BY date ASC LIMIT 0, :amount");
    query.bindValue(":date", QVariant(fileDate.toString(Qt::DateFormat::ISODateWithMs)));
    query.bindValue(":amount", QVariant(amount));
    query.exec();

    QStringList files;
    while(query.next())
    {
        files.append(query.value(0).toString());
    }

    return files;
}

QStringList SQLiteDatabase::GetFilesBeforeFile(const QString& fileName, int amount)
{
    QDateTime fileDate = GetDateForFile(fileName);

    QSqlQuery query(mSqlDatabase);
    query.prepare("SELECT fileName FROM Files WHERE date < (:date) ORDER BY date DESC LIMIT 0, :amount");
    query.bindValue(":date", QVariant(fileDate.toString(Qt::DateFormat::ISODateWithMs)));
    query.bindValue(":amount", QVariant(amount));
    query.exec();

    std::cout << fileDate.toString(Qt::DateFormat::ISODateWithMs).toUtf8().constData() << std::endl;
    std::cout << query.lastQuery().toUtf8().constData() << std::endl;

    QStringList files;
    while(query.next())
    {
        files.append(query.value(0).toString());
    }

    return files;
}

QDateTime SQLiteDatabase::GetDateForFile(const QString& fileName)
{
    QDateTime fileDate;

    QSqlQuery query(mSqlDatabase);
    query.prepare("SELECT date FROM Files WHERE fileName = (:fileName)");
    query.bindValue(":fileName", QVariant(fileName));
    query.exec();

    if(query.first())
    {
        fileDate = QDateTime::fromString(query.record().field(0).value().toString(), Qt::DateFormat::ISODateWithMs);
    }

    return fileDate;
}

QString SQLiteDatabase::GetFirstImage()
{
    QSqlQuery query(mSqlDatabase);
    query.prepare("SELECT fileName FROM Files LIMIT 0, 1");
    query.exec();

    QString fileName;

    if(query.first())
    {
        fileName = query.record().field(0).value().toString();
    }

    return fileName;
}

void SQLiteDatabase::AddTag(const QString& tag)
{
    QSqlQuery query(mSqlDatabase);
    query.prepare("INSERT INTO Tags (tagName) VALUES (:tagName)");
    query.bindValue(":tagName", QVariant(tag));
    query.exec();
}

void SQLiteDatabase::DeleteTag(const QString& tag)
{
    QSqlQuery query(mSqlDatabase);
    query.prepare("DELETE FROM Tags WHERE tagName = (:tagName)");
    query.bindValue(":tagName", QVariant(tag));
    query.exec();

    query.prepare("DELETE FROM FileTags WHERE tagName = (:tagName)");
    query.bindValue(":tagName", QVariant(tag));
    query.exec();
}

QStringList SQLiteDatabase::GetTags()
{
    QStringList tags;

    QSqlQuery query(mSqlDatabase);
    query.prepare("SELECT * FROM Tags");
    query.exec();

    int idxTagName = query.record().indexOf("tagName");
    while(query.next())
    {
        tags.append(query.value(idxTagName).toString());
    }

    return tags;
}
