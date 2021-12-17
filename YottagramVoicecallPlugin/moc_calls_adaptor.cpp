/****************************************************************************
** Meta object code from reading C++ file 'calls_adaptor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "calls_adaptor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'calls_adaptor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_CallsAdaptor_t {
    QByteArrayData data[17];
    char stringdata0[803];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CallsAdaptor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CallsAdaptor_t qt_meta_stringdata_CallsAdaptor = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CallsAdaptor"
QT_MOC_LITERAL(1, 13, 15), // "D-Bus Interface"
QT_MOC_LITERAL(2, 29, 32), // "com.verdanditeam.yottagram.calls"
QT_MOC_LITERAL(3, 62, 19), // "D-Bus Introspection"
QT_MOC_LITERAL(4, 82, 604), // "  <interface name=\"com.verda..."
QT_MOC_LITERAL(5, 631, 6), // "accept"
QT_MOC_LITERAL(6, 638, 0), // ""
QT_MOC_LITERAL(7, 639, 17), // "changeSpeakerMode"
QT_MOC_LITERAL(8, 657, 11), // "loudspeaker"
QT_MOC_LITERAL(9, 669, 7), // "discard"
QT_MOC_LITERAL(10, 677, 14), // "muteMicrophone"
QT_MOC_LITERAL(11, 692, 4), // "mute"
QT_MOC_LITERAL(12, 697, 9), // "callReady"
QT_MOC_LITERAL(13, 707, 11), // "discardCall"
QT_MOC_LITERAL(14, 719, 7), // "newCall"
QT_MOC_LITERAL(15, 727, 10), // "callerName"
QT_MOC_LITERAL(16, 738, 8) // "incoming"

    },
    "CallsAdaptor\0D-Bus Interface\0"
    "com.verdanditeam.yottagram.calls\0"
    "D-Bus Introspection\0"
    "  <interface name=\"com.verdanditeam.yottagram.calls\">\n    <method n"
    "ame=\"newCall\">\n      <arg direction=\"in\" type=\"s\" name=\"caller"
    "Name\"/>\n      <arg direction=\"in\" type=\"b\" name=\"incoming\"/>\n"
    "    </method>\n    <method name=\"discardCall\"/>\n    <method name=\""
    "callReady\"/>\n    <signal name=\"discard\"/>\n    <signal name=\"acce"
    "pt\"/>\n    <signal name=\"muteMicrophone\">\n      <arg direction=\"o"
    "ut\" type=\"b\" name=\"mute\"/>\n    </signal>\n    <signal name=\"cha"
    "ngeSpeakerMode\">\n      <arg direction=\"out\" type=\"b\" name=\"loud"
    "speaker\"/>\n    </signal>\n  </interface>\n\0"
    "accept\0\0changeSpeakerMode\0loudspeaker\0"
    "discard\0muteMicrophone\0mute\0callReady\0"
    "discardCall\0newCall\0callerName\0incoming"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CallsAdaptor[] = {

 // content:
       7,       // revision
       0,       // classname
       2,   14, // classinfo
       7,   18, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // classinfo: key, value
       1,    2,
       3,    4,

 // signals: name, argc, parameters, tag, flags
       5,    0,   53,    6, 0x06 /* Public */,
       7,    1,   54,    6, 0x06 /* Public */,
       9,    0,   57,    6, 0x06 /* Public */,
      10,    1,   58,    6, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    0,   61,    6, 0x0a /* Public */,
      13,    0,   62,    6, 0x0a /* Public */,
      14,    2,   63,    6, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   11,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,   15,   16,

       0        // eod
};

void CallsAdaptor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CallsAdaptor *_t = static_cast<CallsAdaptor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->accept(); break;
        case 1: _t->changeSpeakerMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->discard(); break;
        case 3: _t->muteMicrophone((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->callReady(); break;
        case 5: _t->discardCall(); break;
        case 6: _t->newCall((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (CallsAdaptor::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CallsAdaptor::accept)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (CallsAdaptor::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CallsAdaptor::changeSpeakerMode)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (CallsAdaptor::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CallsAdaptor::discard)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (CallsAdaptor::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CallsAdaptor::muteMicrophone)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject CallsAdaptor::staticMetaObject = {
    { &QDBusAbstractAdaptor::staticMetaObject, qt_meta_stringdata_CallsAdaptor.data,
      qt_meta_data_CallsAdaptor,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *CallsAdaptor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CallsAdaptor::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_CallsAdaptor.stringdata0))
        return static_cast<void*>(const_cast< CallsAdaptor*>(this));
    return QDBusAbstractAdaptor::qt_metacast(_clname);
}

int CallsAdaptor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractAdaptor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void CallsAdaptor::accept()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void CallsAdaptor::changeSpeakerMode(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CallsAdaptor::discard()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void CallsAdaptor::muteMicrophone(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
