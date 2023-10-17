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

#include "scopenotificationsettings.h"

ScopeNotificationSettings::ScopeNotificationSettings(QObject *parent) : QObject(parent)
{

}

void ScopeNotificationSettings::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;
}

void ScopeNotificationSettings::setScopeNotificationSettings(td_api::object_ptr<td_api::scopeNotificationSettings> scopeNotificationSettings, QString scope)
{
    _scope = scope;
    _scopeNotificationSettings = move(scopeNotificationSettings);
}

td_api::scopeNotificationSettings *ScopeNotificationSettings::getScopeNotificationSettings()
{
    return _scopeNotificationSettings.get();
}

td_api::object_ptr<td_api::NotificationSettingsScope> ScopeNotificationSettings::getScope() const
{
    if (_scope == "private") return td_api::make_object<td_api::notificationSettingsScopePrivateChats>();
    if (_scope == "group") return td_api::make_object<td_api::notificationSettingsScopeGroupChats>();
    if (_scope == "channel") return td_api::make_object<td_api::notificationSettingsScopeChannelChats>();
    return nullptr;
}

int32_t ScopeNotificationSettings::getMuteFor() const
{
    if (_scopeNotificationSettings) {
        return _scopeNotificationSettings->mute_for_;
    } else {
        return -1;
    }
}

void ScopeNotificationSettings::setMuteFor(int32_t muteFor)
{
    auto settings = td_api::make_object<td_api::scopeNotificationSettings>(
        _scopeNotificationSettings->mute_for_,
        _scopeNotificationSettings->sound_id_,
        _scopeNotificationSettings->show_preview_,
        _scopeNotificationSettings->use_default_mute_stories_,
        _scopeNotificationSettings->mute_stories_,
        _scopeNotificationSettings->story_sound_id_,
        _scopeNotificationSettings->show_story_sender_,
        _scopeNotificationSettings->disable_pinned_message_notifications_,
        _scopeNotificationSettings->disable_mention_notifications_
    );
    settings->mute_for_ = muteFor;
    _manager->sendQuery(new td_api::setScopeNotificationSettings(getScope(), move(settings)));
}

bool ScopeNotificationSettings::getShowPreview() const
{
    if (_scopeNotificationSettings) {
        return _scopeNotificationSettings->show_preview_;
    } else {
        return false;
    }
}

void ScopeNotificationSettings::setShowPreview(bool showPreview)
{
    auto settings = td_api::make_object<td_api::scopeNotificationSettings>(
        _scopeNotificationSettings->mute_for_,
        _scopeNotificationSettings->sound_id_,
        _scopeNotificationSettings->show_preview_,
        _scopeNotificationSettings->use_default_mute_stories_,
        _scopeNotificationSettings->mute_stories_,
        _scopeNotificationSettings->story_sound_id_,
        _scopeNotificationSettings->show_story_sender_,
        _scopeNotificationSettings->disable_pinned_message_notifications_,
        _scopeNotificationSettings->disable_mention_notifications_
    );
    settings->show_preview_ = showPreview;
    _manager->sendQuery(new td_api::setScopeNotificationSettings(getScope(), move(settings)));
}

bool ScopeNotificationSettings::getUseDefaultMuteStories() const
{
    if (_scopeNotificationSettings) {
        return _scopeNotificationSettings->use_default_mute_stories_;
    } else {
        return false;
    }
}

void ScopeNotificationSettings::setUseDefaultMuteStories(bool useDefaultMuteStories)
{
    auto settings = td_api::make_object<td_api::scopeNotificationSettings>(
        _scopeNotificationSettings->mute_for_,
        _scopeNotificationSettings->sound_id_,
        _scopeNotificationSettings->show_preview_,
        _scopeNotificationSettings->use_default_mute_stories_,
        _scopeNotificationSettings->mute_stories_,
        _scopeNotificationSettings->story_sound_id_,
        _scopeNotificationSettings->show_story_sender_,
        _scopeNotificationSettings->disable_pinned_message_notifications_,
        _scopeNotificationSettings->disable_mention_notifications_
        );
    settings->use_default_mute_stories_ = useDefaultMuteStories;
    _manager->sendQuery(new td_api::setScopeNotificationSettings(getScope(), move(settings)));
}

bool ScopeNotificationSettings::getMuteStories() const
{
    if (_scopeNotificationSettings) {
        return _scopeNotificationSettings->mute_stories_;
    } else {
        return false;
    }
}

void ScopeNotificationSettings::setMuteStories(bool muteStories)
{
    auto settings = td_api::make_object<td_api::scopeNotificationSettings>(
        _scopeNotificationSettings->mute_for_,
        _scopeNotificationSettings->sound_id_,
        _scopeNotificationSettings->show_preview_,
        _scopeNotificationSettings->use_default_mute_stories_,
        _scopeNotificationSettings->mute_stories_,
        _scopeNotificationSettings->story_sound_id_,
        _scopeNotificationSettings->show_story_sender_,
        _scopeNotificationSettings->disable_pinned_message_notifications_,
        _scopeNotificationSettings->disable_mention_notifications_
        );
    settings->mute_for_ = muteStories;
    _manager->sendQuery(new td_api::setScopeNotificationSettings(getScope(), move(settings)));
}

int64_t ScopeNotificationSettings::getStorySoundId() const
{
    if (_scopeNotificationSettings) {
        return _scopeNotificationSettings->story_sound_id_;
    } else {
        return false;
    }
}

void ScopeNotificationSettings::setStorySoundId(int64_t storySoundId)
{
    auto settings = td_api::make_object<td_api::scopeNotificationSettings>(
        _scopeNotificationSettings->mute_for_,
        _scopeNotificationSettings->sound_id_,
        _scopeNotificationSettings->show_preview_,
        _scopeNotificationSettings->use_default_mute_stories_,
        _scopeNotificationSettings->mute_stories_,
        _scopeNotificationSettings->story_sound_id_,
        _scopeNotificationSettings->show_story_sender_,
        _scopeNotificationSettings->disable_pinned_message_notifications_,
        _scopeNotificationSettings->disable_mention_notifications_
        );
    settings->story_sound_id_ = storySoundId;
    _manager->sendQuery(new td_api::setScopeNotificationSettings(getScope(), move(settings)));
}

bool ScopeNotificationSettings::getShowStorySender() const
{
    if (_scopeNotificationSettings) {
        return _scopeNotificationSettings->show_story_sender_;
    } else {
        return false;
    }
}

void ScopeNotificationSettings::setShowStorySender(bool showStorySender)
{
    auto settings = td_api::make_object<td_api::scopeNotificationSettings>(
        _scopeNotificationSettings->mute_for_,
        _scopeNotificationSettings->sound_id_,
        _scopeNotificationSettings->show_preview_,
        _scopeNotificationSettings->use_default_mute_stories_,
        _scopeNotificationSettings->mute_stories_,
        _scopeNotificationSettings->story_sound_id_,
        _scopeNotificationSettings->show_story_sender_,
        _scopeNotificationSettings->disable_pinned_message_notifications_,
        _scopeNotificationSettings->disable_mention_notifications_
        );
    settings->show_story_sender_ = showStorySender;
    _manager->sendQuery(new td_api::setScopeNotificationSettings(getScope(), move(settings)));
}

bool ScopeNotificationSettings::getDisablePinnedMessageNotifications() const
{
    if (_scopeNotificationSettings) {
       return _scopeNotificationSettings->disable_pinned_message_notifications_;
    } else {
        return false;
    }
}

void ScopeNotificationSettings::setDisablePinnedMessageNotifications(bool disablePinnedMessageNotifications)
{
    auto settings = td_api::make_object<td_api::scopeNotificationSettings>(
        _scopeNotificationSettings->mute_for_,
        _scopeNotificationSettings->sound_id_,
        _scopeNotificationSettings->show_preview_,
        _scopeNotificationSettings->use_default_mute_stories_,
        _scopeNotificationSettings->mute_stories_,
        _scopeNotificationSettings->story_sound_id_,
        _scopeNotificationSettings->show_story_sender_,
        _scopeNotificationSettings->disable_pinned_message_notifications_,
        _scopeNotificationSettings->disable_mention_notifications_
    );
    settings->disable_pinned_message_notifications_ = disablePinnedMessageNotifications;
    _manager->sendQuery(new td_api::setScopeNotificationSettings(getScope(), move(settings)));
}

bool ScopeNotificationSettings::getDisableMentionNotifications() const
{
    if (_scopeNotificationSettings) {
        return _scopeNotificationSettings->disable_mention_notifications_;
    } else {
        return false;
    }
}

void ScopeNotificationSettings::setDisableMentionNotifications(bool disableMentionNotifications)
{
    auto settings = td_api::make_object<td_api::scopeNotificationSettings>(
        _scopeNotificationSettings->mute_for_,
        _scopeNotificationSettings->sound_id_,
        _scopeNotificationSettings->show_preview_,
        _scopeNotificationSettings->use_default_mute_stories_,
        _scopeNotificationSettings->mute_stories_,
        _scopeNotificationSettings->story_sound_id_,
        _scopeNotificationSettings->show_story_sender_,
        _scopeNotificationSettings->disable_pinned_message_notifications_,
        _scopeNotificationSettings->disable_mention_notifications_
    );
    settings->disable_mention_notifications_ = disableMentionNotifications;
    _manager->sendQuery(new td_api::setScopeNotificationSettings(getScope(), move(settings)));
}
