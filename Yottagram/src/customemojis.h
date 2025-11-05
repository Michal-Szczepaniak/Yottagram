#ifndef CUSTOMEMOJIS_H
#define CUSTOMEMOJIS_H

#include <QObject>
#include "core/telegrammanager.h"
#include "files/files.h"

class CustomEmojis : public QObject
{
    Q_OBJECT
public:
    struct CustomEmoji {
        int64_t emojiId{};
        int64_t stickerId{};
        int64_t thumbnailId{};
        QString emoji{};
    };

    explicit CustomEmojis(QObject *parent = nullptr);
    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setFiles(shared_ptr<Files> files);

    int32_t getCustomEmojiSticker(int64_t emojiId);
    int32_t getCustomEmojiThumbnail(int64_t emojiId);
    CustomEmoji getCustomEmoji(int64_t emojiId);

signals:
    void messageUpdated(int64_t chatId, int64_t messageId, QVector<int32_t> fileIds);

public slots:
    void onGotCustomEmojiStickers(int64_t chatId, int64_t messageId, td_api::stickers *stickers);
    void addCustomEmoji(int64_t emojiId, int32_t stickerId, int32_t thumbnailId, QString emoji);

private:
    std::shared_ptr<TelegramManager> _manager;
    std::shared_ptr<Files> _files;
    QHash<int64_t, int32_t> _customEmojiStickers;
    QHash<int64_t, int32_t> _customEmojiThumbnails;
    QHash<int64_t, CustomEmoji> _customEmojis;
};

#endif // CUSTOMEMOJIS_H
