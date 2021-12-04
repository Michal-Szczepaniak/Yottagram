#ifndef CONTENTINTERFACE_H
#define CONTENTINTERFACE_H

#include "core/telegrammanager.h"

class ContentInterface {
public:
    virtual void handleContent(td_api::object_ptr<td_api::MessageContent> messageContent) = 0;
};

#endif // CONTENTINTERFACE_H
