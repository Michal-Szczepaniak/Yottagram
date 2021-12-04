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

function format(duration) {
    if (duration === 0) return "";
    var formatted = "";
    if (duration % 60) formatted = " " + qsTr("%L1 s").arg(duration);
    var minuteduration = Math.floor(duration/60);
    if (minuteduration % 60) formatted = " " + qsTr("%L1 min").arg(minuteduration) + formatted;
    var hourduration = Math.floor(minuteduration/60);
    if (hourduration) formatted = qsTr("%L1 h").arg(hourduration) + formatted;

    return formatted;
}
