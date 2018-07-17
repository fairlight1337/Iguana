#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QProgressBar>
#include <QGraphicsItem>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QLineEdit>
#include <QDir>
#include <QLabel>
#include <QShortcut>
#include <QKeySequence>

#include <iostream>

#include <downloadmanager.h>
#include <database.h>
#include <sqlitedatabase.h>
#include <checkablestringlistmodel.h>


namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;

    CheckableStringListModel* mTagsListModel;

    QGraphicsScene* mScene;

    QUrl mBaseUrl;
    QStringList mUrlsToDownload;
    QStringList mThreadsToDownload;
    int mPageIndex;

    DownloadManager* mDownloadManager;
    bool mDownloadFlag;

    Database* mDatabase;

    QString mSelectedFileName;

    static QString saveFileName(const QUrl& url);
    static QByteArray fileChecksum(const QString& fileName, QCryptographicHash::Algorithm hashAlgorithm);
    QByteArray dataChecksum(QByteArray& data, QCryptographicHash::Algorithm hashAlgorithm);
    void selectImage(QString fileName);
    void selectFirstImage();

protected:
    void showEvent(QShowEvent *ev);

private slots:
    void showTagsListContextMenu(const QPoint& point);
    void addNewTag();
    void removeSelectedTags();
    void toggleDownload(bool download);
    void cycleDownload();
    void processIndexPageDownload(QUrl url, QByteArray data);
    void processThreadPageDownload(QUrl url, QByteArray data);
    void processImageDownload(QUrl url, QByteArray data);
    void downloadProgress(QUrl url, qint64 bytesRead, qint64 bytesTotal);
    void selectPreviousImage();
    void selectNextImage();
    void tagActivated(QString tagName);
    void tagDeactivated(QString tagName);
    void startTraining();
};


#endif // MAINWINDOW_H
