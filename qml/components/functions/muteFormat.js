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

function formatMute(muteFor) {
    if (muteFor == 0) return "";
    if (muteFor > 0 && muteFor < 60) return qsTr("%L1 second(s)").arg(muteFor);
    var minuteMuteFor = Math.floor(muteFor/60);
    if (minuteMuteFor < 60) return qsTr("%L1 minute(s)").arg(minuteMuteFor);
    var hourMuteFor = Math.floor(minuteMuteFor/60);
    if (hourMuteFor < 24) return qsTr("%L1 hour(s)").arg(hourMuteFor);
    var dayMuteFor = Math.floor(hourMuteFor/24);
    if (dayMuteFor > 2) return ""
    return qsTr("%L1 day(s)").arg(dayMuteFor);
}

function appendSpace(value) {
    if (value !== "") return value + " "
    return value
}

function formatAutoDestruct(value) {
    if (value == 0) return "";
    if (value > 0 && value < 60) return qsTr("%L1").arg(value);
    var minuteDestruct = Math.floor(value/60);
    if (minuteDestruct < 60) return qsTr("%L1m").arg(minuteDestruct);
    var hourDestruct = Math.floor(minuteDestruct/60);
    if (hourDestruct < 24) return qsTr("%L1h").arg(hourDestruct);
    var dayDestruct = Math.floor(hourDestruct/24);
    if (dayDestruct >= 7)
        return qsTr("%L1w").arg(Math.floor(dayDestruct/7));
}
