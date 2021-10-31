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

#ifndef SCOPENOTIFICATIONSETTINGS_H
#define SCOPENOTIFICATIONSETTINGS_H

#include <QObject>
#include "../core/telegrammanager.h"

class ScopeNotificationSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int32_t muteFor READ getMuteFor WRITE setMuteFor NOTIFY scopeNotificationSettingsChanged)
    Q_PROPERTY(bool showPreview READ getShowPreview WRITE setShowPreview NOTIFY scopeNotificationSettingsChanged)
    Q_PROPERTY(bool disablePinnedMessageNotifications READ getDisablePinnedMessageNotifications WRITE setDisablePinnedMessageNotifications NOTIFY scopeNotificationSettingsChanged)
    Q_PROPERTY(bool disableMentionNotifications READ getDisableMentionNotifications WRITE setDisableMentionNotifications NOTIFY scopeNotificationSettingsChanged)
public:
    explicit ScopeNotificationSettings(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setScopeNotificationSettings(td_api::object_ptr<td_api::scopeNotificationSettings> scopeNotificationSettings, QString scope);
    td_api::scopeNotificationSettings* getScopeNotificationSettings();

    td_api::object_ptr<td_api::NotificationSettingsScope> getScope() const;
    int32_t getMuteFor() const;
    void setMuteFor(int32_t muteFor);
    bool getShowPreview() const;
    void setShowPreview(bool showPreview);
    bool getDisablePinnedMessageNotifications() const;
    void setDisablePinnedMessageNotifications(bool disablePinnedMessageNotifications);
    bool getDisableMentionNotifications() const;
    void setDisableMentionNotifications(bool disableMentionNotifications);
signals:
    void scopeNotificationSettingsChanged();

public slots:

private:
    QString _scope;
    std::shared_ptr<TelegramManager> _manager;
    td_api::object_ptr<td_api::scopeNotificationSettings> _scopeNotificationSettings;
};
Q_DECLARE_METATYPE(ScopeNotificationSettings*)

#endif // SCOPENOTIFICATIONSETTINGS_H
