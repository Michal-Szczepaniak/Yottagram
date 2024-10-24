#include "datamanager.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

DataManager::DataManager(QObject *parent) : QObject(parent)
{
    _newPath = getOldPath();
}

QString DataManager::getNewPath()
{
    return _newPath;
}

void DataManager::setNewPath(QString newPath)
{
    _newPath = newPath;

    emit newPathChanged();
    emit newPathExistsChanged();
    emit noPermissionChanged();
}

QString DataManager::getOldPath()
{
    return QSettings().value("files_directory", QDir::homePath() + "/.local/share/Yottagram/data").toString();
}

bool DataManager::getNewPathExists()
{
    return QFile::exists(getNewPath());
}

bool DataManager::getNoPermission()
{
    if (getNewPathExists()) return false;

    QString path = getNewPath().left(getNewPath().lastIndexOf('/'));
    qDebug() << "Path: " << path;
    QFileInfo dest(path);

    return !dest.isDir() || !dest.isWritable();
}

void DataManager::commitNewPath()
{
    QSettings().setValue("new_files_directory", _newPath);
}

QString DataManager::savePhoto(QString path)
{
    const QString savePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/Yottagram/";
    const QString targetPath = savePath + generateFileName(path);
    if (!QDir(savePath).exists()) {
        QDir().mkdir(savePath);
    }

    QFile::copy(path, targetPath);
    return targetPath;
}

QString DataManager::saveVideo(QString path)
{
    const QString savePath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/Yottagram/";
    const QString targetPath = savePath + generateFileName(path);
    if (!QDir(savePath).exists()) {
        QDir().mkdir(savePath);
    }

    QFile::copy(path, targetPath);
    return targetPath;
}

QString DataManager::saveAudio(QString path)
{
    const QString savePath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/Yottagram/";
    const QString targetPath = savePath + generateFileName(path);
    if (!QDir(savePath).exists()) {
        QDir().mkdir(savePath);
    }

    QFile::copy(path, targetPath);
    return targetPath;
}

QString DataManager::saveDocument(QString path)
{
    const QString savePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Yottagram/";
    const QString targetPath = savePath + generateFileName(path);
    if (!QDir(savePath).exists()) {
        QDir().mkdir(savePath);
    }

    QFile::copy(path, targetPath);
    return targetPath;
}

QString DataManager::generateFileName(QString path)
{
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    int rnd = qrand() % 100;

    QString filename = path.split('/').last();
    QString extension = "";
    if (filename.split('.').size() >= 2) {
        extension = '.' + filename.split('.').last();
    }

    return date + QString::number(rnd) + extension;
}
