#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mBaseUrl(QUrl("http://boards.4chan.org/wg/")),
    mPageIndex(9),
    mDownloadManager(new DownloadManager()),
    mDownloadFlag(false),
    mDatabase(nullptr)
{
    ui->setupUi(this);

    cout << "Application directory path: " << QApplication::applicationDirPath().toUtf8().constData() << endl;

    mTagsListModel = new CheckableStringListModel(this);

    QGraphicsView* view = findChild<QGraphicsView*>("imageDisplay");
    mScene = new QGraphicsScene(view);
    view->setScene(mScene);

    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPushButton* downloadButton = findChild<QPushButton*>("downloadButton");
    downloadButton->setCheckable(true);
    connect(downloadButton, SIGNAL(toggled(bool)), this, SLOT(toggleDownload(bool)));

    connect(mDownloadManager, SIGNAL(downloadProgress(QUrl, qint64, qint64)), this, SLOT(downloadProgress(QUrl, qint64, qint64)));

    QPushButton* trainButton = findChild<QPushButton*>("trainButton");
    connect(trainButton, SIGNAL(clicked()), this, SLOT(startTraining()));

    QString databaseFile = QApplication::applicationDirPath() + "/database.sql";
    mDatabase = new SQLiteDatabase(databaseFile);
    mDatabase->Initialize();

    mTagsListModel->setStringList(mDatabase->GetTags());
    QListView* tagsList = findChild<QListView*>("tagsList");
    tagsList->setModel(mTagsListModel);

    tagsList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tagsList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTagsListContextMenu(const QPoint&)));

    if(!QDir(QApplication::applicationDirPath() + "/storage").exists())
    {
        QDir().mkdir(QApplication::applicationDirPath() + "/storage");
    }

    QShortcut* shortcutSelectPreviousImage = new QShortcut(QKeySequence::MoveToPreviousChar, this);
    QObject::connect(shortcutSelectPreviousImage, SIGNAL(activated()), this, SLOT(selectPreviousImage()));

    QShortcut* shortcutSelectNextImage = new QShortcut(QKeySequence::MoveToNextChar, this);
    QObject::connect(shortcutSelectNextImage, SIGNAL(activated()), this, SLOT(selectNextImage()));

    connect(mTagsListModel, SIGNAL(checkboxActivated(QString)), this, SLOT(tagActivated(QString)));
    connect(mTagsListModel, SIGNAL(checkboxDeactivated(QString)), this, SLOT(tagDeactivated(QString)));
}

MainWindow::~MainWindow()
{
    delete mDatabase;
    delete mDownloadManager;
    delete ui;
}

void MainWindow::showEvent(QShowEvent* ev)
{
    QMainWindow::showEvent(ev);

    selectFirstImage();
}

void MainWindow::showTagsListContextMenu(const QPoint& point)
{
    QListView* tagsList = findChild<QListView*>("tagsList");

    QMenu contextMenu(tr("Tag Actions"), tagsList);

    QAction actionAddTag("Add new tag", tagsList);
    connect(&actionAddTag, SIGNAL(triggered()), this, SLOT(addNewTag()));
    contextMenu.addAction(&actionAddTag);

    QAction actionRemoveTags("Remove selected tag(s)", tagsList);
    connect(&actionRemoveTags, SIGNAL(triggered()), this, SLOT(removeSelectedTags()));

    if(tagsList->selectionModel()->selectedIndexes().size() > 0)
    {
        contextMenu.addAction(&actionRemoveTags);
    }

    contextMenu.exec(tagsList->mapToGlobal(point));
}

void MainWindow::addNewTag()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add new tag"), tr("Tag name:"), QLineEdit::Normal, "", &ok);

    if(ok && !text.isEmpty())
    {
        mDatabase->AddTag(text);

        QStringList currentStringList = mTagsListModel->stringList();
        currentStringList.append(text);
        currentStringList.sort();

        mTagsListModel->setStringList(currentStringList);
    }
}

void MainWindow::removeSelectedTags()
{
    QListView* tagsList = findChild<QListView*>("tagsList");
    QModelIndexList selectedIndexes = tagsList->selectionModel()->selectedIndexes();

    for(const QModelIndex& idx : selectedIndexes)
    {
        mDatabase->DeleteTag(mTagsListModel->stringList().at(idx.row()));
        mTagsListModel->removeRow(idx.row());
    }
}

void MainWindow::toggleDownload(bool download)
{
    mDownloadFlag = download;

    if(download)
    {
        QTimer::singleShot(0, this, SLOT(cycleDownload()));
    }
}

void MainWindow::cycleDownload()
{
    if(mDownloadFlag)
    {
        if(mUrlsToDownload.empty())
        {
            if(mThreadsToDownload.empty())
            {
                connect(mDownloadManager, SIGNAL(downloadDone(QUrl, QByteArray)), this, SLOT(processIndexPageDownload(QUrl, QByteArray)));

                QString indexedBaseUrl = mBaseUrl.toString();
                if(mPageIndex > 0)
                {
                    indexedBaseUrl += QString::number(mPageIndex + 1);
                    mPageIndex--;
                }
                else
                {
                    mPageIndex = 9;
                }

                cout << "Base url: " << mBaseUrl.toEncoded().constData() << endl;
                cout << "Download index page: " << indexedBaseUrl.toUtf8().constData() << endl;
                mDownloadManager->doDownload(QUrl(indexedBaseUrl));
            }
            else
            {
                connect(mDownloadManager, SIGNAL(downloadDone(QUrl, QByteArray)), this, SLOT(processThreadPageDownload(QUrl, QByteArray)));

                QString threadUrl = mThreadsToDownload.front();
                mThreadsToDownload.pop_front();

                cout << "Download thread page: " << threadUrl.toUtf8().constData() << endl;
                mDownloadManager->doDownload(QUrl(threadUrl));
            }
        }
        else
        {
            QUrl currentDownloadUrl = QUrl(mUrlsToDownload.front());
            mUrlsToDownload.pop_front();

            connect(mDownloadManager, SIGNAL(downloadDone(QUrl, QByteArray)), this, SLOT(processImageDownload(QUrl, QByteArray)));
            mDownloadManager->doDownload(currentDownloadUrl);
        }
    }
    else
    {
        mUrlsToDownload.clear();
    }
}

void MainWindow::processIndexPageDownload(QUrl url, QByteArray data)
{
    Q_UNUSED(url);

    disconnect(mDownloadManager, SIGNAL(downloadDone(QUrl, QByteArray)), this, SLOT(processIndexPageDownload(QUrl, QByteArray)));

    if(data != nullptr)
    {
        //QRegularExpression indexRegExp("<a class\\=\"fileThumb\" href\\=\"//([a-zA-Z0-9\\./]*\\.[jpg|png|bmp|gif]*)\"");
        QRegularExpression indexRegExp("href=\"/wg/thread/([0-9]*)");
        QRegularExpressionMatchIterator i = indexRegExp.globalMatch(data.constData());

        QStringList sets;
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString set = match.captured(1);

            sets << set;
        }

        for(QString set : sets)
        {
            QString threadUrl = mBaseUrl.toString() + "thread/" + set;
            if(!mThreadsToDownload.contains(threadUrl))
            {
                mThreadsToDownload << threadUrl;
            }
        }

        QTimer::singleShot(0, this, SLOT(cycleDownload()));
    }
    else
    {
        cout << "Failed to download index page." << endl;
    }
}

void MainWindow::processThreadPageDownload(QUrl url, QByteArray data)
{
    Q_UNUSED(url);

    disconnect(mDownloadManager, SIGNAL(downloadDone(QUrl, QByteArray)), this, SLOT(processThreadPageDownload(QUrl, QByteArray)));

    if(data != nullptr)
    {
        QRegularExpression indexRegExp("<a class\\=\"fileThumb\" href\\=\"//([a-zA-Z0-9\\./]*\\.[jpg|png|bmp|gif]*)\"");
        QRegularExpressionMatchIterator i = indexRegExp.globalMatch(data.constData());

        QStringList sets;
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString set = match.captured(1);

            sets << set;
        }

        for(QString set : sets)
        {
            QUrl newUrl = "http://" + set;

            QString basename = newUrl.fileName();
            if(!mDatabase->FileWithFileNameIsRegistered(basename))
            {
                mUrlsToDownload << newUrl.toString();
            }
        }

        QTimer::singleShot(0, this, SLOT(cycleDownload()));
    }
    else
    {
        cout << "Failed to download thread page." << endl;
    }
}

void MainWindow::processImageDownload(QUrl url, QByteArray data)
{
    disconnect(mDownloadManager, SIGNAL(downloadDone(QUrl, QByteArray)), this, SLOT(processImageDownload(QUrl, QByteArray)));

    if(data != nullptr)
    {
        cout << "Got image data from '" << url.toEncoded().constData() << "': " << data.size() << " bytes" << endl;

        QByteArray hash = dataChecksum(data, QCryptographicHash::Algorithm::Sha512);
        QString basename = url.fileName();

        if(!mDatabase->FileWithHashIsRegistered(hash.toHex()))
        {
            QString fullPath = saveFileName(url);
            QFile file(fullPath);

            if(file.open(QIODevice::WriteOnly))
            {
                file.write(data);
                file.close();

                mDatabase->RegisterFile(basename, hash.toHex());
            }

            QCheckBox* checkFollow = findChild<QCheckBox*>("checkFollow");
            if(checkFollow->checkState() == Qt::CheckState::Checked)
            {
                selectImage(QFileInfo(fullPath).fileName());
            }
        }
    }
    else
    {
        cout << "Image download failed." << endl;
    }

    QTimer::singleShot(100, this, SLOT(cycleDownload()));
}

void MainWindow::selectFirstImage()
{
    QString firstImage = mDatabase->GetFirstImage();

    if(!firstImage.isEmpty())
    {
        selectImage(firstImage);
    }
}

void MainWindow::selectImage(QString fileName)
{
    mSelectedFileName = fileName;

    QString fullPath = QApplication::applicationDirPath() + "/storage/" + fileName;

    QLabel* fileNameLabel = findChild<QLabel*>("fileNameLabel");
    fileNameLabel->setText(mSelectedFileName);

    QGraphicsView* imageDisplay = findChild<QGraphicsView*>("imageDisplay");

    while(mScene->items().size() > 0)
    {
        QGraphicsItem* item = mScene->items().at(0);
        mScene->removeItem(item);

        delete item;
    }

    QImage img(fullPath);
    mScene->addPixmap(QPixmap::fromImage(img));

    imageDisplay->fitInView(mScene->itemsBoundingRect(), Qt::KeepAspectRatio);
    imageDisplay->show();

    for(int i = 0; i < mTagsListModel->rowCount(); ++i)
    {
        //cout << mTagsListModel->stringList().at(i).toUtf8().constData() << endl;
        if(mDatabase->FileHasTag(mSelectedFileName, mTagsListModel->stringList().at(i)))
        {
            mTagsListModel->setData(mTagsListModel->index(i), Qt::CheckState::Checked, Qt::CheckStateRole);
        }
        else
        {
            mTagsListModel->setData(mTagsListModel->index(i), Qt::CheckState::Unchecked, Qt::CheckStateRole);
        }
    }

    findChild<QListView*>("tagsList")->show();
}

QByteArray MainWindow::fileChecksum(const QString& fileName, QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);

    if(f.open(QFile::ReadOnly))
    {
        QCryptographicHash hash(hashAlgorithm);

        if(hash.addData(&f))
        {
            return hash.result();
        }
    }

    return QByteArray();
}

QByteArray MainWindow::dataChecksum(QByteArray& data, QCryptographicHash::Algorithm hashAlgorithm)
{
    QCryptographicHash hash(hashAlgorithm);
    hash.addData(data);

    return hash.result();
}

QString MainWindow::saveFileName(const QUrl& url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if(basename.isEmpty())
    {
        basename = "download";
    }

    QString fullPath = QApplication::applicationDirPath() + "/storage/" + basename;

    if(QFile::exists(fullPath))
    {
        int i = 0;

        QFileInfo info(basename);
        QString extension = info.suffix();
        QString baseNameWithoutExtension = info.completeBaseName();

        while(QFile::exists(QApplication::applicationDirPath() + "/storage/" + baseNameWithoutExtension + "." + QString::number(i) + "." + extension))
        {
            ++i;
        }

        fullPath = QApplication::applicationDirPath() + "/storage/" + baseNameWithoutExtension + "." + QString::number(i) + "." + extension;
    }

    return fullPath;
}

void MainWindow::downloadProgress(QUrl url, qint64 bytesRead, qint64 bytesTotal)
{
    Q_UNUSED(url);

    QProgressBar* progressBar = findChild<QProgressBar*>("downloadProgressBar");

    progressBar->setMinimum(0);
    progressBar->setMaximum(static_cast<int>(bytesTotal));
    progressBar->setValue(static_cast<int>(bytesRead));
}

void MainWindow::selectPreviousImage()
{
    QCheckBox* checkFollow = findChild<QCheckBox*>("checkFollow");
    checkFollow->setCheckState(Qt::CheckState::Unchecked);

    if(mSelectedFileName.isEmpty())
    {
        selectFirstImage();
    }
    else
    {
        QStringList previousImages = mDatabase->GetFilesBeforeFile(mSelectedFileName, 1);

        if(!previousImages.empty())
        {
            cout << "Select " << previousImages.at(0).toUtf8().constData() << endl;
            selectImage(previousImages.at(0));
        }
    }
}

void MainWindow::selectNextImage()
{
    QCheckBox* checkFollow = findChild<QCheckBox*>("checkFollow");
    checkFollow->setCheckState(Qt::CheckState::Unchecked);

    if(mSelectedFileName.isEmpty())
    {
        selectFirstImage();
    }
    else
    {
        QStringList nextImages = mDatabase->GetFilesAfterFile(mSelectedFileName, 1);

        if(!nextImages.empty())
        {
            cout << "Select " << nextImages.at(0).toUtf8().constData() << endl;
            selectImage(nextImages.at(0));
        }
    }
}

void MainWindow::tagActivated(QString tagName)
{
    if(!mSelectedFileName.isEmpty())
    {
        mDatabase->SetTagForFile(mSelectedFileName, tagName, true);
    }
}

void MainWindow::tagDeactivated(QString tagName)
{
    if(!mSelectedFileName.isEmpty())
    {
        mDatabase->SetTagForFile(mSelectedFileName, tagName, false);
    }
}

void MainWindow::startTraining()
{
    cout << "Train" << endl;
}
