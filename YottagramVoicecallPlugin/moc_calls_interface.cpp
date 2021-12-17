/****************************************************************************
** Meta object code from reading C++ file 'calls_interface.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "calls_interface.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'calls_interface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ComVerdanditeamYottagramCallsInterface_t {
    QByteArrayData data[14];
    char stringdata0[175];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ComVerdanditeamYottagramCallsInterface_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ComVerdanditeamYottagramCallsInterface_t qt_meta_stringdata_ComVerdanditeamYottagramCallsInterface = {
    {
QT_MOC_LITERAL(0, 0, 38), // "ComVerdanditeamYottagramCalls..."
QT_MOC_LITERAL(1, 39, 6), // "accept"
QT_MOC_LITERAL(2, 46, 0), // ""
QT_MOC_LITERAL(3, 47, 17), // "changeSpeakerMode"
QT_MOC_LITERAL(4, 65, 11), // "loudspeaker"
QT_MOC_LITERAL(5, 77, 7), // "discard"
QT_MOC_LITERAL(6, 85, 14), // "muteMicrophone"
QT_MOC_LITERAL(7, 100, 4), // "mute"
QT_MOC_LITERAL(8, 105, 9), // "callReady"
QT_MOC_LITERAL(9, 115, 19), // "QDBusPendingReply<>"
QT_MOC_LITERAL(10, 135, 11), // "discardCall"
QT_MOC_LITERAL(11, 147, 7), // "newCall"
QT_MOC_LITERAL(12, 155, 10), // "callerName"
QT_MOC_LITERAL(13, 166, 8) // "incoming"

    },
    "ComVerdanditeamYottagramCallsInterface\0"
    "accept\0\0changeSpeakerMode\0loudspeaker\0"
    "discard\0muteMicrophone\0mute\0callReady\0"
    "QDBusPendingReply<>\0discardCall\0newCall\0"
    "callerName\0incoming"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ComVerdanditeamYottagramCallsInterface[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x06 /* Public */,
       3,    1,   50,    2, 0x06 /* Public */,
       5,    0,   53,    2, 0x06 /* Public */,
       6,    1,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   57,    2, 0x0a /* Public */,
      10,    0,   58,    2, 0x0a /* Public */,
      11,    2,   59,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    7,

 // slots: parameters
    0x80000000 | 9,
    0x80000000 | 9,
    0x80000000 | 9, QMetaType::QString, QMetaType::Bool,   12,   13,

       0        // eod
};

void ComVerdanditeamYottagramCallsInterface::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ComVerdanditeamYottagramCallsInterface *_t = static_cast<ComVerdanditeamYottagramCallsInterface *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->accept(); break;
        case 1: _t->changeSpeakerMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->discard(); break;
        case 3: _t->muteMicrophone((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: { QDBusPendingReply<> _r = _t->callReady();
            if (_a[0]) *reinterpret_cast< QDBusPendingReply<>*>(_a[0]) = _r; }  break;
        case 5: { QDBusPendingReply<> _r = _t->discardCall();
            if (_a[0]) *reinterpret_cast< QDBusPendingReply<>*>(_a[0]) = _r; }  break;
        case 6: { QDBusPendingReply<> _r = _t->newCall((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QDBusPendingReply<>*>(_a[0]) = _r; }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ComVerdanditeamYottagramCallsInterface::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ComVerdanditeamYottagramCallsInterface::accept)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (ComVerdanditeamYottagramCallsInterface::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ComVerdanditeamYottagramCallsInterface::changeSpeakerMode)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (ComVerdanditeamYottagramCallsInterface::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ComVerdanditeamYottagramCallsInterface::discard)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (ComVerdanditeamYottagramCallsInterface::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ComVerdanditeamYottagramCallsInterface::muteMicrophone)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject ComVerdanditeamYottagramCallsInterface::staticMetaObject = {
    { &QDBusAbstractInterface::staticMetaObject, qt_meta_stringdata_ComVerdanditeamYottagramCallsInterface.data,
      qt_meta_data_ComVerdanditeamYottagramCallsInterface,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ComVerdanditeamYottagramCallsInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ComVerdanditeamYottagramCallsInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ComVerdanditeamYottagramCallsInterface.stringdata0))
        return static_cast<void*>(const_cast< ComVerdanditeamYottagramCallsInterface*>(this));
    return QDBusAbstractInterface::qt_metacast(_clname);
}

int ComVerdanditeamYottagramCallsInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractInterface::qt_metacall(_c, _id, _a);
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
void ComVerdanditeamYottagramCallsInterface::accept()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void ComVerdanditeamYottagramCallsInterface::changeSpeakerMode(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ComVerdanditeamYottagramCallsInterface::discard()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void ComVerdanditeamYottagramCallsInterface::muteMicrophone(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
