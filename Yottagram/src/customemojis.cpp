#include "customemojis.h"

CustomEmojis::CustomEmojis(QObject *parent) : QObject(parent)
{

}

void CustomEmojis::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::gotCustomEmojiStickers, this, &CustomEmojis::onGotCustomEmojiStickers);
}

void CustomEmojis::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

int32_t CustomEmojis::getCustomEmojiSticker(int64_t emojiId)
{
    if (!_customEmojiStickers.contains(emojiId)) {
        return 0;
    }

    return _customEmojiStickers[emojiId];
}

int32_t CustomEmojis::getCustomEmojiThumbnail(int64_t emojiId)
{
    if (!_customEmojiThumbnails.contains(emojiId)) {
        return 0;
    }

    return _customEmojiThumbnails[emojiId];
}

CustomEmojis::CustomEmoji CustomEmojis::getCustomEmoji(int64_t emojiId)
{
    if (!_customEmojis.contains(emojiId)) {
        qDebug() << "Missing emoji: " << emojiId;
        return {};
    }

    return _customEmojis[emojiId];
}

void CustomEmojis::onGotCustomEmojiStickers(int64_t chatId, int64_t messageId, td_api::stickers *stickers)
{
    QVector<int32_t> fileIds;

    for (td_api::object_ptr<td_api::sticker> &sticker: stickers->stickers_) {
        if (sticker->full_type_->get_id() != td_api::stickerFullTypeCustomEmoji::ID ||
            _customEmojiStickers.contains(static_cast<td_api::stickerFullTypeCustomEmoji*>(sticker->full_type_.get())->custom_emoji_id_)) {
            continue;
        }

        int64_t emojiId = static_cast<td_api::stickerFullTypeCustomEmoji*>(sticker->full_type_.get())->custom_emoji_id_;

        addCustomEmoji(emojiId, sticker->sticker_->id_, sticker->thumbnail_->file_->id_, QString::fromStdString(sticker->emoji_));

        _files->appendFile(move(sticker->sticker_), "sticker");
        _files->appendFile(move(sticker->thumbnail_->file_), "thumbnail");

        fileIds.append(_customEmojiStickers[emojiId]);
    }

    emit messageUpdated(chatId, messageId, fileIds);
}

void CustomEmojis::addCustomEmoji(int64_t emojiId, int32_t stickerId, int32_t thumbnailId, QString emoji)
{
    _customEmojis[emojiId] = {
        .emojiId = emojiId,
        .stickerId = stickerId,
        .thumbnailId = thumbnailId,
        .emoji = emoji,
    };

    _customEmojiStickers[emojiId] = stickerId;
    _customEmojiThumbnails[emojiId] = thumbnailId;

}
