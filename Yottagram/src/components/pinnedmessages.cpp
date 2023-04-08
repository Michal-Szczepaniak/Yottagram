#include "pinnedmessages.h"

PinnedMessages::PinnedMessages(QObject *parent): QObject(parent), _currentMessage(0), _chatId(0)
{

}

PinnedMessages::~PinnedMessages()
{
    for (Message *m : _messages) delete m;
    _messages.clear();
}

void PinnedMessages::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::gotSearchChatMessagesFilterPinned, this, &PinnedMessages::onGotSearchChatMessagesFilterPinned);
    connect(_manager.get(), &TelegramManager::updateMessageIsPinned, this, &PinnedMessages::onUpdateMessageIsPinned);

    fetchMessages();
}

void PinnedMessages::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

void PinnedMessages::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

int64_t PinnedMessages::chatId() const
{
    return _chatId;
}

void PinnedMessages::setChatId(int64_t chatId)
{
    _chatId = chatId;

    emit chatIdChanged();
}

int64_t PinnedMessages::id()
{
    if (_currentMessage >= _messages.size()) return 0;

    return _messages[_currentMessage]->getId();
}

int64_t PinnedMessages::senderUserId()
{
    if (_currentMessage >= _messages.size()) return 0;

    return _messages[_currentMessage]->getSenderUserId();
}

QString PinnedMessages::type()
{
    if (_currentMessage >= _messages.size()) return "";

    return _messages[_currentMessage]->getType();
}

QString PinnedMessages::text()
{
    if (_currentMessage >= _messages.size()) return "";

    return _messages[_currentMessage]->getText();
}

void PinnedMessages::cycleMessage()
{
    if (_currentMessage+1 >= _messages.size()) _currentMessage = 0;
    else _currentMessage++;
    emit idChanged();
}

void PinnedMessages::cycleMessageBack()
{
    if (_currentMessage-1 < 0) _currentMessage = _messages.size()-1;
    else _currentMessage--;
    emit idChanged();
}

bool PinnedMessages::contains(int64_t messageId)
{
    std::vector<Message*>::iterator result = find_if(_messages.begin(), _messages.end(), [messageId](Message* m){ return m->getId() == messageId; });
    return result != _messages.end();
}

void PinnedMessages::fetchMessages(int64_t fromMessage)
{
    td_api::searchChatMessages *searchMessages = new td_api::searchChatMessages();
    searchMessages->chat_id_ = _chatId;
    searchMessages->filter_ = td_api::make_object<td_api::searchMessagesFilterPinned>();
    searchMessages->from_message_id_ = fromMessage;
    searchMessages->limit_ = 2;

    _manager->sendQueryWithResponse(_chatId, td_api::searchChatMessages::ID, td_api::searchMessagesFilterPinned::ID, searchMessages);
}

void PinnedMessages::onGotSearchChatMessagesFilterPinned(int64_t chatId, td_api::messages *messages)
{
    if (chatId != _chatId) return;

    for (td_api::object_ptr<td_api::message> &m : messages->messages_) {
        Message *n = new Message();
        n->setTelegramManager(_manager);
        n->setUsers(_users);
        n->setFiles(_files);
        n->setChatId(_chatId);
        n->setMessage(m.release());

        _messages.emplace_back(n);
    }

    if (messages->messages_.size() > 0) fetchMessages(_messages.back()->getId());

    emit idChanged();
}

void PinnedMessages::onUpdateMessageIsPinned(td_api::updateMessageIsPinned *updateMessageIsPinned)
{
    if (updateMessageIsPinned->chat_id_ != _chatId) return;

    for (Message *m : _messages) delete m;
    _messages.clear();

    fetchMessages();
}
