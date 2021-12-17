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

#include "poll.h"
#include <QDebug>

Poll::Poll(QAbstractListModel *parent) : QAbstractListModel(parent), _poll(nullptr)
{
}

void Poll::handleContent(td_api::object_ptr<td_api::MessageContent> content)
{
    if (content->get_id() == td_api::messagePoll::ID) {
        _poll = move(static_cast<td_api::messagePoll*>(content.release())->poll_);

        emit pollChanged();
    }
}

int Poll::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (_poll != nullptr) {
        return static_cast<int>(_poll->options_.size());
    }

    return 0;
}

QVariant Poll::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() < 0 || index.row() >= rowCount()) return QVariant();

    auto& option = _poll->options_[index.row()];
    switch (role) {
    case PollRoles::TextRole:
        return QString::fromStdString(option->text_);
    case PollRoles::VoteCountRole:
        return option->voter_count_;
    case PollRoles::VotePercentageRole:
        return option->vote_percentage_;
    case PollRoles::IsChosenRole:
        return option->is_chosen_;
    case PollRoles::IsBeingChosenRole:
        return option->is_being_chosen_;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> Poll::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TextRole]   = "optionText";
    roles[VoteCountRole]   = "voteCount";
    roles[VotePercentageRole]   = "votePercentage";
    roles[IsChosenRole]   = "isChosen";
    roles[IsBeingChosenRole]   = "isBeingChosen";
    return roles;
}

int64_t Poll::getId() const
{
    return _poll->id_;
}

QString Poll::getQuestion() const
{
    return QString::fromStdString(_poll->question_);
}

int32_t Poll::getTotalVoterCount() const
{
    return _poll->total_voter_count_;
}

std::vector<int64_t> Poll::getRecentVoterUserIds() const
{
    return _poll->recent_voter_user_ids_;
}

bool Poll::isAnonymous() const
{
    return _poll->is_anonymous_;
}

int Poll::getType() const
{
    switch (_poll->type_->get_id()) {
    case td_api::pollTypeQuiz::ID:
        return PollTypes::Quiz;
    case td_api::pollTypeRegular::ID:
    default:
        return PollTypes::Regular;
    }
}

QString Poll::getTypeName() const
{
    switch (getType()) {
    case PollTypes::Quiz:
        return "quiz";
    case PollTypes::Regular:
    default:
        return "regular";
    }
}

bool Poll::isClosed() const
{
    return _poll->is_closed_;
}

bool Poll::getAllowMultipleAnswers() const
{
    if (getType() == PollTypes::Regular) {
        return static_cast<const td_api::pollTypeRegular &>(*_poll->type_).allow_multiple_answers_;
    }

    return false;
}

int32_t Poll::getCorrectOptionId() const
{
    if (getType() == PollTypes::Quiz) {
        return static_cast<const td_api::pollTypeQuiz &>(*_poll->type_).correct_option_id_;
    }

    return -1;
}

bool Poll::isAnswered() const
{
    for(auto& option: _poll->options_) {
        if (option->is_chosen_) return true;
    }

    return false;
}

QString Poll::getRecentVotersString() const
{
    QStringList voters;

    for (auto& voter: getRecentVoterUserIds()) {
        voters << QString::number(voter);
    }

    return voters.join(',');
}
