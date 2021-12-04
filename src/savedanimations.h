#ifndef SAVEDANIMATIONS_H
#define SAVEDANIMATIONS_H

#include <QAbstractListModel>
#include "core/telegrammanager.h"
#include "files/files.h"

class SavedAnimations : public QAbstractListModel
{
    Q_OBJECT
public:
    enum AnimationRoles {
        IdRole = Qt::UserRole + 1,
        FileRole,
        ThumbnailRole,
        VideoThumbnailRole,
        WidthRole,
        HeightRole
    };

    struct SavedAnimation {
        int32_t id;
        int32_t width;
        int32_t height;
        int32_t thumbnailId;
        bool videoThumbnail;
    };

    explicit SavedAnimations(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setFiles(shared_ptr<Files> files);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = IdRole) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    void onUpdateSavedAnimations(td_api::updateSavedAnimations *updateSavedAnimations);
    void onGotSavedAnimations(td_api::animations* animations);
    void onIsAuthorizedChanged(bool isAuthorized);

private:
    std::shared_ptr<TelegramManager> _manager;
    std::shared_ptr<Files> _files;
    QVector<int32_t> _savedAnimationsId;
    QMap<int32_t, SavedAnimation> _savedAnimations;
};

#endif // SAVEDANIMATIONS_H
