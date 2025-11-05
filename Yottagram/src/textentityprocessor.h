#ifndef TEXTENTITYPROCESSOR_H
#define TEXTENTITYPROCESSOR_H

#include <QDomNode>
#include <QObject>
#include <customemojis.h>
#include <users.h>
#include <td/telegram/Client.h>

using namespace td;

class TextEntityProcessor : public QObject
{
    Q_OBJECT
public:
    explicit TextEntityProcessor(QObject *parent = nullptr);

    void setUsers(shared_ptr<Users> users);
    void setFiles(shared_ptr<Files> files);
    void setCustomEmojis(shared_ptr<CustomEmojis> customEmojis);

    td_api::object_ptr<td_api::formattedText> parseHTML(QString html);
    QString formatTextAsHTML(td_api::formattedText* formattedText, bool forEdit = false);

private:
    void parseNode(QDomNode node, QString &out, td_api::array<td_api::object_ptr<td_api::textEntity>> &entities);
    QString getHTMLEntityForIndex(int index, td_api::textEntity *entity, QString originalText, bool startTag, int &skip, bool forEdit = false);

signals:

private:
    shared_ptr<Users> _users;
    shared_ptr<Files> _files;
    shared_ptr<CustomEmojis> _customEmojis;
};

#endif // TEXTENTITYPROCESSOR_H
