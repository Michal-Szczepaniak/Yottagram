/****************************************************************************
** Meta object code from reading C++ file 'yottagramvoicecallprovider.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Yottagram/YottagramVoicecallPlugin/yottagramvoicecallprovider.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'yottagramvoicecallprovider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_YottagramVoiceCallProvider_t {
    QByteArrayData data[14];
    char stringdata0[148];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_YottagramVoiceCallProvider_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_YottagramVoiceCallProvider_t qt_meta_stringdata_YottagramVoiceCallProvider = {
    {
QT_MOC_LITERAL(0, 0, 26), // "YottagramVoiceCallProvider"
QT_MOC_LITERAL(1, 27, 6), // "accept"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 7), // "discard"
QT_MOC_LITERAL(4, 43, 14), // "muteMicrophone"
QT_MOC_LITERAL(5, 58, 4), // "mute"
QT_MOC_LITERAL(6, 63, 17), // "changeSpeakerMode"
QT_MOC_LITERAL(7, 81, 11), // "loudspeaker"
QT_MOC_LITERAL(8, 93, 4), // "dial"
QT_MOC_LITERAL(9, 98, 9), // "callReady"
QT_MOC_LITERAL(10, 108, 11), // "discardCall"
QT_MOC_LITERAL(11, 120, 7), // "newCall"
QT_MOC_LITERAL(12, 128, 10), // "callerName"
QT_MOC_LITERAL(13, 139, 8) // "incoming"

    },
    "YottagramVoiceCallProvider\0accept\0\0"
    "discard\0muteMicrophone\0mute\0"
    "changeSpeakerMode\0loudspeaker\0dial\0"
    "callReady\0discardCall\0newCall\0callerName\0"
    "incoming"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_YottagramVoiceCallProvider[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x06 /* Public */,
       3,    0,   55,    2, 0x06 /* Public */,
       4,    1,   56,    2, 0x06 /* Public */,
       6,    1,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   62,    2, 0x0a /* Public */,
       9,    0,   65,    2, 0x0a /* Public */,
      10,    0,   66,    2, 0x0a /* Public */,
      11,    2,   67,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    7,

 // slots: parameters
    QMetaType::Bool, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,   12,   13,

       0        // eod
};

void YottagramVoiceCallProvider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        YottagramVoiceCallProvider *_t = static_cast<YottagramVoiceCallProvider *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->accept(); break;
        case 1: _t->discard(); break;
        case 2: _t->muteMicrophone((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->changeSpeakerMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: { bool _r = _t->dial((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 5: _t->callReady(); break;
        case 6: _t->discardCall(); break;
        case 7: _t->newCall((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (YottagramVoiceCallProvider::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&YottagramVoiceCallProvider::accept)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (YottagramVoiceCallProvider::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&YottagramVoiceCallProvider::discard)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (YottagramVoiceCallProvider::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&YottagramVoiceCallProvider::muteMicrophone)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (YottagramVoiceCallProvider::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&YottagramVoiceCallProvider::changeSpeakerMode)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject YottagramVoiceCallProvider::staticMetaObject = {
    { &AbstractVoiceCallProvider::staticMetaObject, qt_meta_stringdata_YottagramVoiceCallProvider.data,
      qt_meta_data_YottagramVoiceCallProvider,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *YottagramVoiceCallProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *YottagramVoiceCallProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_YottagramVoiceCallProvider.stringdata0))
        return static_cast<void*>(const_cast< YottagramVoiceCallProvider*>(this));
    return AbstractVoiceCallProvider::qt_metacast(_clname);
}

int YottagramVoiceCallProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractVoiceCallProvider::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void YottagramVoiceCallProvider::accept()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void YottagramVoiceCallProvider::discard()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void YottagramVoiceCallProvider::muteMicrophone(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void YottagramVoiceCallProvider::changeSpeakerMode(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
