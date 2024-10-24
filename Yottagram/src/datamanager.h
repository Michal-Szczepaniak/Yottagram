#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>

class DataManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString newPath READ getNewPath WRITE setNewPath NOTIFY newPathChanged)
    Q_PROPERTY(bool newPathExists READ getNewPathExists NOTIFY newPathExistsChanged)
    Q_PROPERTY(bool noPermission READ getNoPermission NOTIFY noPermissionChanged)
public:
    explicit DataManager(QObject *parent = nullptr);

    QString getNewPath();
    void setNewPath(QString newPath);
    Q_INVOKABLE QString getOldPath();
    bool getNewPathExists();
    bool getNoPermission();
    Q_INVOKABLE void commitNewPath();
    Q_INVOKABLE QString savePhoto(QString path);
    Q_INVOKABLE QString saveVideo(QString path);
    Q_INVOKABLE QString saveAudio(QString path);
    Q_INVOKABLE QString saveDocument(QString path);

private:
    QString generateFileName(QString path);

signals:
    void newPathChanged();
    void newPathExistsChanged();
    void noPermissionChanged();

private:
    QString _newPath;
};

#endif // DATAMANAGER_H
