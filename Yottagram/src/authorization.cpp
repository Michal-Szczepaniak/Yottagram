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

#include "authorization.h"
#include "overloaded.h"
#include <systemsettings/aboutsettings.h>
#include <systemsettings/deviceinfo.h>
#include <QDebug>

#define STR(x) #x
#define STRINGIFY(x) STR(x)

Authorization::Authorization(QObject *parent) : QObject(parent)
{

}

void Authorization::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), SIGNAL(onMessageReceived(uint64_t,td_api::Object*)), this, SLOT(messageReceived(uint64_t,td_api::Object*)));
}

void Authorization::updateAuthorizationState(td_api::updateAuthorizationState &updateAuthorizationState)
{
    _authorizationState = std::move(updateAuthorizationState.authorization_state_);
    td_api::downcast_call(
        *_authorizationState,
        overloaded(
            [this](td_api::authorizationStateReady &) {
                authorizationStateReady();
            },
            [this](td_api::authorizationStateLoggingOut &) {
                authorizationStateLoggingOut();
//                are_authorized_ = false;
//                std::cerr << "Logging out" << std::endl;
            },
            [this](td_api::authorizationStateClosing &) {
                authorizationStateClosing();
//                std::cerr << "Closing" << std::endl;
            },
            [this](td_api::authorizationStateClosed &) {
                authorizationStateClosed();
//                are_authorized_ = false;
//                need_restart_ = true;
//                std::cerr << "Terminated" << std::endl;
            },
            [this](td_api::authorizationStateWaitEmailAddress &) {

            },
            [this](td_api::authorizationStateWaitEmailCode &) {

            },
            [this](td_api::authorizationStateWaitCode &) {
                authorizationStateWaitCode();
//                std::string first_name;
//                std::string last_name;
//                if (!wait_code.is_registered_) {
//                    std::cerr << "Enter your first name: ";
//                    std::cin >> first_name;
//                    std::cerr << "Enter your last name: ";
//                    std::cin >> last_name;
//                }
//                std::cerr << "Enter authentication code: ";
//                std::string code;
//                std::cin >> code;
//                send_query(td_api::make_object<td_api::checkAuthenticationCode>(code, first_name, last_name),
//                    create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitRegistration &) {
//                std::string first_name;
//                std::string last_name;
//                std::cerr << "Enter your first name: ";
//                std::cin >> first_name;
//                std::cerr << "Enter your last name: ";
//                std::cin >> last_name;
//                send_query(td_api::make_object<td_api::registerUser>(first_name, last_name),
//                           create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitOtherDeviceConfirmation &) {
            },
            [this](td_api::authorizationStateWaitPassword &) {
                authorizationStateWaitPassword();
//              std::cerr << "Enter authentication password: ";
//              std::string password;
//              std::cin >> password;
//              send_query(td_api::make_object<td_api::checkAuthenticationPassword>(password),
//                         create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitPhoneNumber &) {
                authorizationStateWaitPhoneNumber();
            },
            [this](td_api::authorizationStateWaitTdlibParameters &) {
                authorizationStateWaitTdlibParameters();
            }
        )
    );
}

bool Authorization::isAuthorized()
{
    return _isAuthorized;
}

void Authorization::setIsAuthorized(bool isAuthorized)
{
    _isAuthorized = isAuthorized;
    emit isAuthorizedChanged(_isAuthorized);
}

void Authorization::sendNumber(QString number)
{
    _manager->sendQuery(new td_api::setAuthenticationPhoneNumber(number.toStdString(), nullptr));
}

void Authorization::sendCode(QString code)
{
    _manager->sendQuery(new td_api::checkAuthenticationCode(code.toStdString()));
}

void Authorization::sendPassword(QString password)
{
    _manager->sendQuery(new td_api::checkAuthenticationPassword(password.toStdString()));
}

void Authorization::messageReceived(uint64_t id, td_api::Object *object)
{
    Q_UNUSED(id)
    downcast_call(
        *object, overloaded(
            [this](td_api::updateAuthorizationState &update_authorization_state) {
                this->updateAuthorizationState(update_authorization_state);
            },
            [](auto &update) { Q_UNUSED(update) }
        )
    );
}

void Authorization::authorizationStateReady()
{
    setIsAuthorized(true);
//    _manager->sendQuery(new td_api::getInstalledStickerSets(false));
    qDebug() << "authorizationStateReady";
}

void Authorization::authorizationStateLoggingOut()
{
    qDebug()<<"authorizationStateLoggingOut";

}

void Authorization::authorizationStateClosing()
{
    qDebug()<<"authorizationStateClosing";

}

void Authorization::authorizationStateClosed()
{
    qDebug()<<"authorizationStateClosed";
    setIsAuthorized(false);
}

void Authorization::authorizationStateWaitCode()
{
    qDebug()<<"authorizationStateWaitCode";
    emit waitingForCode();
}

void Authorization::authorizationStateWaitPassword()
{
    qDebug()<<"authorizationStateWaitPassword";
    emit waitingForPassword();
}

void Authorization::authorizationStateWaitPhoneNumber()
{
    qDebug()<<"authorizationStateWaitPhoneNumber";
    emit waitingForPhoneNumber();
}

void Authorization::authorizationStateWaitTdlibParameters()
{
    qDebug()<<"authorizationStateWaitTdlibParameters";

    AboutSettings aboutSettings;
    DeviceInfo deviceInfo;

    auto parameters = new td_api::setTdlibParameters();
    parameters->database_directory_ = QDir::homePath().toStdString() + "/.local/share/Yottagram";
    parameters->files_directory_ = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation).toStdString() + "/Yottagram";
    parameters->use_message_database_ = true;
    parameters->use_chat_info_database_ = true;
    parameters->use_file_database_ = true;
    parameters->use_secret_chats_ = true;
    parameters->use_test_dc_ = false;
    parameters->api_id_ = APP_TD_API_ID; // API KEY
    parameters->api_hash_ = STRINGIFY(APP_TD_API_HASH); // API HASH
    parameters->system_language_code_ = "en";
    parameters->device_model_ = (deviceInfo.manufacturer() + " " + deviceInfo.prettyName()).toStdString();
    parameters->system_version_ = (aboutSettings.localizedOperatingSystemName() + " " + aboutSettings.localizedSoftwareVersion()).toStdString();
    parameters->application_version_ = "0.4.1";
    parameters->enable_storage_optimizer_ = true;
    _manager->sendQuery(parameters);
}
