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

#ifndef POLL_H
#define POLL_H

#include <QAbstractListModel>
#include <QPointer>
#include "core/telegrammanager.h"

class Poll : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int64_t id READ getId NOTIFY pollChanged)
    Q_PROPERTY(QString question READ getQuestion NOTIFY pollChanged)
    Q_PROPERTY(int32_t totalVoterCount READ getTotalVoterCount NOTIFY pollChanged)
    Q_PROPERTY(bool isAnonymous READ isAnonymous NOTIFY pollChanged)
    Q_PROPERTY(int type READ getType NOTIFY pollChanged)
    Q_PROPERTY(QString typeName READ getTypeName NOTIFY pollChanged)
    Q_PROPERTY(bool isClosed READ isClosed NOTIFY pollChanged)
    Q_PROPERTY(bool allowMultipleAnswers READ getAllowMultipleAnswers NOTIFY pollChanged)
    Q_PROPERTY(int32_t correctOptionId READ getCorrectOptionId NOTIFY pollChanged)
    Q_PROPERTY(bool isAnswered READ isAnswered NOTIFY pollChanged)
    Q_PROPERTY(QString recentVoters READ getRecentVotersString NOTIFY pollChanged)
public:
    enum PollRoles {
        TextRole = Qt::UserRole + 1,
        VoteCountRole,
        VotePercentageRole,
        IsChosenRole,
        IsBeingChosenRole
    };

    enum PollTypes {
        Regular,
        Quiz
    };

    explicit Poll(QAbstractListModel *parent = nullptr);
    void setPoll(td_api::object_ptr<td_api::poll> poll);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = TextRole) const;
    QHash<int, QByteArray> roleNames() const;

    int64_t getId() const;
    QString getQuestion() const;
    int32_t getTotalVoterCount() const;
    std::vector<int32_t> getRecentVoterUserIds() const;
    bool isAnonymous() const;
    int getType() const;
    QString getTypeName() const;
    bool isClosed() const;
    bool getAllowMultipleAnswers() const;
    int32_t getCorrectOptionId() const;
    bool isAnswered() const;
    QString getRecentVotersString() const;

signals:
    void pollChanged();

private:
    td_api::object_ptr<td_api::poll> _poll;
};
Q_DECLARE_METATYPE(Poll*)

#endif // POLL_H
