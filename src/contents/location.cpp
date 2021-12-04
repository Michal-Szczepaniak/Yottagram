#include "location.h"

Location::Location(QObject *parent) : QObject(parent)
{

}

void Location::handleContent(td_api::object_ptr<td_api::MessageContent> content)
{
    if (content->get_id() == td_api::messageLocation::ID) {
        _location = td_api::move_object_as<td_api::messageLocation>(content);

        emit locationChanged();
    }
}

double Location::latitude() const
{
    return _location->location_->latitude_;
}

double Location::longitude() const
{
    return _location->location_->longitude_;
}
