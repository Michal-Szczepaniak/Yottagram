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

td_api::sticker *CustomEmojis::getCustomEmoji(int64_t emojiId)
{
    if (!_customEmojis.contains(emojiId)) {
        return nullptr;
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

        _customEmojiStickers[emojiId] = sticker->sticker_->id_;

        if (_customEmojis.contains(emojiId)) {
            delete _customEmojis[emojiId];
        }

        _files->appendFile(move(sticker->sticker_), "sticker");
        _customEmojis[emojiId] = sticker.release();

        fileIds.append(_customEmojiStickers[emojiId]);
    }

    emit messageUpdated(chatId, messageId, fileIds);
}
