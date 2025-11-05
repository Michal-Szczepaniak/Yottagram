#include "textentityprocessor.h"

#include <QtXml/QDomDocument>
#include <libsailfishsilica/silicatheme.h>
#include <QDebug>

TextEntityProcessor::TextEntityProcessor(QObject *parent) : QObject(parent)
{

}

void TextEntityProcessor::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

void TextEntityProcessor::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

void TextEntityProcessor::setCustomEmojis(shared_ptr<CustomEmojis> customEmojis)
{
    _customEmojis = customEmojis;
}

td_api::object_ptr<td_api::formattedText> TextEntityProcessor::parseHTML(QString html)
{
    QDomDocument doc;
    if (!doc.setContent(html)) {
        qWarning() << "Error parsing message html";
        return {};
    }

    QDomElement rootElement = doc.documentElement();
    QDomNode body = rootElement.namedItem("body");
    if (body.isNull()) {
        qWarning() << "Couldn't find body";
        return {};
    }

    QString output;
    td_api::array<td_api::object_ptr<td_api::textEntity>> entities;
    parseNode(body, output, entities);

    td_api::object_ptr<td_api::formattedText> result = td_api::make_object<td_api::formattedText>();
    result->text_ = output.toStdString();
    result->entities_ = move(entities);
    return result;
}

void TextEntityProcessor::parseNode(QDomNode node, QString &out, td_api::array<td_api::object_ptr<td_api::textEntity>> &entities)
{
    if (node.isNull()) return;

    if (node.isText()) {
        QString text = node.toText().data();

        QDomElement parent = node.parentNode().toElement();
        if (parent.tagName() == "a" || parent.parentNode().toElement().tagName() == "a") return;
        QString style = parent.attribute("style");

        if (style.contains("font-weight:600;")) {
            entities.push_back(td_api::make_object<td_api::textEntity>(out.length(), text.length(), td_api::make_object<td_api::textEntityTypeBold>()));
        }
        if (style.contains("font-style:italic;")) {
            entities.push_back(td_api::make_object<td_api::textEntity>(out.length(), text.length(), td_api::make_object<td_api::textEntityTypeItalic>()));
        }
        if (style.contains("text-decoration: underline;")) {
            entities.push_back(td_api::make_object<td_api::textEntity>(out.length(), text.length(), td_api::make_object<td_api::textEntityTypeUnderline>()));
        }
        if (style.contains("text-decoration: line-through;")) {
            entities.push_back(td_api::make_object<td_api::textEntity>(out.length(), text.length(), td_api::make_object<td_api::textEntityTypeStrikethrough>()));
        }
        if (style.contains("font-family:'monospace';")) {
            entities.push_back(td_api::make_object<td_api::textEntity>(out.length(), text.length(), td_api::make_object<td_api::textEntityTypeCode>()));
        }

        out.append(text);

        return;
    }

    if (!node.isElement()) {
        qDebug() << "Not an element: " << node.nodeType();
        return;
    }

    QString tag = node.toElement().tagName();
    if (tag == "img") {
        QDomElement parent = node.parentNode().toElement();
        if (parent.tagName() != "a") {
            qWarning() << "No parent a element";
        } else {
            QString href = parent.attribute("href");
            if (href.startsWith("emoji://")) {
                QString emoji = _customEmojis->getCustomEmoji(href.mid(8).toLongLong()).emoji;
                td_api::textEntity *entity = new td_api::textEntity(out.length(), emoji.length(), td_api::make_object<td_api::textEntityTypeCustomEmoji>(href.mid(8).toLongLong()));
                entities.push_back(td_api::object_ptr<td_api::textEntity>(entity));
                out.append(emoji);
            }
        }
    } else if (tag == "a") {
        QString href = node.toElement().attribute("href");
        if (href.startsWith("userid://")) {
            QString username = node.toElement().text();

            td_api::textEntity *entity = nullptr;
            if (username.startsWith("@")) {
                entity = new td_api::textEntity(out.length(), username.length(), td_api::make_object<td_api::textEntityTypeMention>());
            } else {
                entity = new td_api::textEntity(out.length(), username.length(), td_api::make_object<td_api::textEntityTypeMentionName>(href.mid(9).toLongLong()));
            }

            entities.push_back(td_api::object_ptr<td_api::textEntity>(entity));
            out.append(username);
        }
    }

    if (node.hasChildNodes()) {
        QDomNodeList children = node.childNodes();
        for (int i = 0; i < children.size(); i++) {
            parseNode(children.at(i), out, entities);
        }
    }

    if (tag == "p") {
        out.append("\n");
    }
}

QString TextEntityProcessor::formatTextAsHTML(td_api::formattedText *formattedText, bool forEdit)
{
    QString originalText = QString::fromStdString(formattedText->text_);
    QString modifiedText = "";

    for (int i = 0; i < originalText.length(); i++) {
        int skip = 0;

        for (td_api::object_ptr<td_api::textEntity> &entity : formattedText->entities_) {
            modifiedText += getHTMLEntityForIndex(i, entity.get(), originalText, true, skip, forEdit);
        }

        if (skip == 0) {
            modifiedText += QString(originalText[i]).toHtmlEscaped();
        }

        for (auto it = formattedText->entities_.rbegin(); it != formattedText->entities_.rend(); it++) {
            modifiedText += getHTMLEntityForIndex(i, (*it).get(), originalText, false, skip, forEdit);
        }

        i += skip;
    }

    return modifiedText;
}

QString TextEntityProcessor::getHTMLEntityForIndex(int index, td_api::textEntity *entity, QString originalText, bool startTag, int &skip, bool forEdit)
{
    if (startTag && index != entity->offset_) return "";
    else if (!startTag && index != (entity->offset_ + entity->length_ - 1)) return "";

    switch (entity->type_->get_id()) {
    case td_api::textEntityTypeBankCardNumber::ID:
        return "";
    case td_api::textEntityTypeBold::ID:
        return startTag ? "<b>" : "</b>";
    case td_api::textEntityTypeBotCommand::ID:
        if (startTag) {
            return "<a href=\"command://" + originalText.mid(entity->offset_, entity->length_) + "\">";
        } else {
            return "</a>";
        }
    case td_api::textEntityTypeCashtag::ID:
        return "";
    case td_api::textEntityTypeCode::ID:
        if (forEdit) {
            return startTag ? "<code>" : "</code>";
        } else {
            return startTag ? "<pre>" : "</pre>";
        }
    case td_api::textEntityTypeCustomEmoji::ID:
    {
        if (!startTag) return "";

        int64_t emojiId = static_cast<td_api::textEntityTypeCustomEmoji*>(entity->type_.get())->custom_emoji_id_;
        int32_t stickerId = _customEmojis->getCustomEmojiThumbnail(emojiId);
        shared_ptr<File> file = _files->getFile(stickerId);
        if (stickerId != 0 && file) {
            QString localPath = file->localPath();
            if (localPath == "") {
                return "";
            }

            skip = entity->length_ - 1;
            auto theme = Silica::Theme::instance();
            QString result;
            result += forEdit ? "<a href=\"emoji://" + QString::number(emojiId) + "\">" : "";
            result += "<img width=\"" + QString::number(theme->fontSizeSmall()) + "\" height=\"" + QString::number(theme->fontSizeSmall()) + "\" align=\"middle\" src=\"" + localPath + "\"/>";
            result += forEdit ? "</a>" : "";
            return result;
        }
        return "";
    }
    case td_api::textEntityTypeEmailAddress::ID:
        if (startTag) {
            return "<a href=\"mailto:" + originalText.mid(entity->offset_, entity->length_) + "\">";
        } else {
            return "</a>";
        }
    case td_api::textEntityTypeHashtag::ID:
        return "";
    case td_api::textEntityTypeItalic::ID:
        return startTag ? "<i>" : "</i>";
    case td_api::textEntityTypeMediaTimestamp::ID:
        return "";
    case td_api::textEntityTypeMention::ID:
        if (startTag) {
            std::shared_ptr<User> user = _users->getUserByUsername(originalText.mid(entity->offset_ + 1, entity->length_ - 1));
            if (user)
                return "<a href=\"userid://" + QString::number(user->getId()) + "\">";
            else
                return "<a>";
        } else {
            return "</a>";
        }
    case td_api::textEntityTypeMentionName::ID:
        if (startTag) {
            return "<a href=\"userid://" + QString::number(static_cast<td_api::textEntityTypeMentionName*>(entity->type_.get())->user_id_) + "\">";
        } else {
            return "</a>";
        }
    case td_api::textEntityTypePhoneNumber::ID:
        if (startTag) {
            return "<a href=\"tel:" + originalText.mid(entity->offset_, entity->length_) + "\">";
        } else {
            return "</a>";
        }
    case td_api::textEntityTypePre::ID:
        return startTag ? "<pre>" : "</pre>";
    case td_api::textEntityTypePreCode::ID:
        if (forEdit) {
            return startTag ? "<pre><code>" : "</code></pre>";
        } else {
            return startTag ? "<pre>" : "</pre>";
        }
    case td_api::textEntityTypeSpoiler::ID:
        return "";
    case td_api::textEntityTypeStrikethrough::ID:
        return startTag ? "<s>" : "</s>";
    case td_api::textEntityTypeTextUrl::ID:
        if (startTag) {
            return "<a href=\"" + QString::fromStdString(static_cast<td_api::textEntityTypeTextUrl*>(entity->type_.get())->url_) + "\">";
        } else {
            return "</a>";
        }
    case td_api::textEntityTypeUnderline::ID:
        return startTag ? "<u>" : "</u>";
    case td_api::textEntityTypeUrl::ID:
        if (startTag) {
            return "<a href=\"" + originalText.mid(entity->offset_, entity->length_) + "\">";
        } else {
            return "</a>";
        }
    default:
        return "";
    }
}
