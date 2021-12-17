#include "call.h"

Call::Call(QObject *parent) : QObject(parent)
{

}

void Call::handleContent(td_api::object_ptr<td_api::MessageContent> content)
{
    if (content->get_id() == td_api::messageCall::ID) {
        _call = td_api::move_object_as<td_api::messageCall>(content);

        emit callChanged();
    }
}

bool Call::duration() const
{
    return _call->duration_;
}
