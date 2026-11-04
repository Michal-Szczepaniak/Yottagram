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

#ifndef SLACKLIBRARY_H
#define SLACKLIBRARY_H

// C ABI exported by vendor/yottaslack/main.go
struct SlackLibrary {
    int (*init)(char *dataDir) = nullptr;
    void (*send)(char *request) = nullptr;
    char *(*receive)(double timeout) = nullptr;
    void (*free)(char *string) = nullptr;
    void (*close)() = nullptr;

    bool isLoaded() const { return init && send && receive && free && close; }
};

#endif // SLACKLIBRARY_H
