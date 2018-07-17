#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H


#include <QtCore>
#include <QtNetwork>

#include <cstdio>
#include <iostream>

class QSslError;

using namespace std;


class DownloadManager: public QObject
{
    Q_OBJECT
    QNetworkAccessManager manager;
    QVector<QNetworkReply*> currentDownloads;

public:
    DownloadManager();
    void doDownload(const QUrl &url);
    //static QString saveFileName(const QUrl &url);
    bool saveToDisk(const QString &filename, QIODevice *data);
    static bool isHttpRedirect(QNetworkReply *reply);

public slots:
    void execute();
    void downloadFinished(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);
    void downloadProgressed(qint64 bytesRead, qint64 bytesTotal);

signals:
    void downloadDone(QUrl url, QByteArray data);
    void downloadProgress(QUrl url, qint64 bytesRead, qint64 bytesTotal);
};


#endif // DOWNLOADMANAGER_H
