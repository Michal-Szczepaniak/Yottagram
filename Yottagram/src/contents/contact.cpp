#include "contact.h"

Contact::Contact(QObject *parent) : QObject(parent)
{

}

void Contact::handleContent(td_api::object_ptr<td_api::MessageContent> content)
{
    if (content->get_id() == td_api::messageContact::ID) {
        _contact = move(static_cast<td_api::messageContact*>(content.release())->contact_);

        emit contactChanged();
    }
}

void Contact::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;
}

void Contact::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

int64_t Contact::userId()
{
    return _contact->user_id_;
}

QString Contact::name()
{
    return QString::fromStdString(_contact->first_name_ + _contact->last_name_);
}

QString Contact::phoneNumber()
{
    return QString::fromStdString(_contact->phone_number_);
}
