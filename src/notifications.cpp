/*

This file is part of Yottagram.
Copyright 2020, Michał Szczepaniak <m.szczepaniak.000@gmail.com>

Yottagram is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Yottagram is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Yottagram. If not, see <http://www.gnu.org/licenses/>.

*/

#include "notifications.h"

Notifications::Notifications(QObject *parent) : QObject(parent)
{

}

void Notifications::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(manager.get(), SIGNAL(updateNotification(td_api::updateNotification*)), this, SLOT(updateNotification(td_api::updateNotification*)));
    connect(manager.get(), SIGNAL(updateNotificationGroup(td_api::updateNotificationGroup*)), this, SLOT(updateNotificationGroup(td_api::updateNotificationGroup*)));
}

void Notifications::setChatList(shared_ptr<ChatList> chatList)
{
    _chatList = chatList;
}

void Notifications::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

void Notifications::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

QString Notifications::getGroupTypeAsString(td_api::NotificationGroupType *notificationGroupType) const
{
    switch (notificationGroupType->get_id()) {
    case td_api::notificationGroupTypeMessages::ID:
        return tr("Messages");
    case td_api::notificationGroupTypeMentions::ID:
        return tr("Mentions");
    case td_api::notificationGroupTypeCalls::ID:
        return tr("Calls");
    case td_api::notificationGroupTypeSecretChat::ID:
        return tr("Secret chat");
    }
}

void Notifications::updateActiveNotifications(td_api::updateActiveNotifications *updateActiveNotifications)
{
    Q_UNUSED(updateActiveNotifications);
}

void Notifications::updateNotificationGroup(td_api::updateNotificationGroup *updateNotificationGroup)
{
    Chat* chat = _chatList->getChat(updateNotificationGroup->chat_id_);

    for (auto& notification : updateNotificationGroup->added_notifications_) {
        if (chat->isOpen() || notification->is_silent_) continue;
        switch (notification->type_.get()->get_id()) {
        case td_api::notificationTypeNewMessage::ID:
        {
            td_api::notificationTypeNewMessage* newMessage = static_cast<td_api::notificationTypeNewMessage*>(notification->type_.get());
            Message message;
            message.setFiles(_files);
            message.setUsers(_users);
            message.setMessage(newMessage->message_.get());
            shared_ptr<User> user = _users->getUser(message.getSenderUserId());
            if (user == nullptr) continue;

            QString textFrom = "";
            if (chat->getChatType() == "group" || chat->getChatType() == "supergroup") {
                textFrom = user->getName() + ": ";
            }

            Notification* newNotification = new Notification;
            newNotification->setCategory("x-verdanditeam.yottagram.im");
            newNotification->setAppName("Yottagram");
            newNotification->setPreviewSummary(tr("New message from %1").arg(chat->getTitle()));
            newNotification->setPreviewBody(textFrom + message.getText().left(30));
            newNotification->setSummary(tr("New message from %1").arg(chat->getTitle()));
            newNotification->setBody(textFrom + message.getText());
            newNotification->setReplacesId(static_cast<quint32>(notification->id_));
            newNotification->setTimestamp(QDateTime::fromTime_t(static_cast<uint>(notification->date_)));
            QVariantList arguments;
            arguments.append(chat->getId());
            QVariantList actions;
            actions.append(Notification::remoteAction("default", "openChat", "com.verdanditeam.yottagram", "/", "com.verdanditeam.yottagram", "openChat", arguments));
            newNotification->setRemoteActions(actions);
            newNotification->publish();
            _notifications[notification->id_] = newNotification;
        }
            break;
        case td_api::notificationTypeNewSecretChat::ID:
        {
            td_api::notificationTypeNewSecretChat* newSecretChat = static_cast<td_api::notificationTypeNewSecretChat*>(notification->type_.get());
            auto chat = _chatList->getChat(updateNotificationGroup->chat_id_);
            if (chat == nullptr) continue;

            Notification* newNotification = new Notification;
            newNotification->setCategory("x-verdanditeam.yottagram.im");
            newNotification->setAppName("Yottagram");
            newNotification->setPreviewSummary(tr("New secret chat"));
            newNotification->setPreviewBody(chat->getTitle());
            newNotification->setSummary(tr("New secret chat"));
            newNotification->setBody(chat->getTitle());
            newNotification->setReplacesId(static_cast<quint32>(notification->id_));
            newNotification->setTimestamp(QDateTime::fromTime_t(static_cast<uint>(notification->date_)));
            QVariantList arguments;
            arguments.append(chat->getId());
            QVariantList actions;
            actions.append(Notification::remoteAction("default", "openChat", "com.verdanditeam.yottagram", "/", "com.verdanditeam.yottagram", "openChat", arguments));
            newNotification->publish();
            _notifications[notification->id_] = newNotification;
        }
        }
    }

    for (qint32 notificationId : updateNotificationGroup->removed_notification_ids_) {
        if (_notifications.contains(notificationId)) {
            Notification* notification = _notifications.take(notificationId);
            notification->close();
            delete notification;
        }
    }
}

void Notifications::updateNotification(td_api::updateNotification *updateNotification)
{
    qDebug() << __PRETTY_FUNCTION__;
//    switch (updateNotification->notification_->type_.get()->get_id()) {
//    case td_api::notificationTypeNewMessage::ID:
//    {
//        qDebug() << __PRETTY_FUNCTION__;
//        td_api::notificationTypeNewMessage* newMessage = static_cast<td_api::notificationTypeNewMessage*>(updateNotification->notification_->type_.get());

//        Notification* notification = new Notification;
//        Message message;
//        message.setMessage(newMessage->message_.get());
//        notification->setCategory("x-nemo.messaging.sms");
//        notification->setAppIcon("/usr/share/icons/hicolor/256x256/apps/Yottagram.png");
//        notification->setAppName("Yottagram");
//        notification->setPreviewSummary("New message");
//        notification->setPreviewBody("New message");
//        notification->setSummary("New message");
////        notification->setExpireTimeout(0);
//        notification->setBody(message.getText());
//        notification->publish();
//    }
//    }
}
