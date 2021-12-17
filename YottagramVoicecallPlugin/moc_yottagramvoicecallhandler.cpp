/****************************************************************************
** Meta object code from reading C++ file 'yottagramvoicecallhandler.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Yottagram/YottagramVoicecallPlugin/yottagramvoicecallhandler.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'yottagramvoicecallhandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_YottagramVoiceCallHandler_t {
    QByteArrayData data[9];
    char stringdata0[75];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_YottagramVoiceCallHandler_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_YottagramVoiceCallHandler_t qt_meta_stringdata_YottagramVoiceCallHandler = {
    {
QT_MOC_LITERAL(0, 0, 25), // "YottagramVoiceCallHandler"
QT_MOC_LITERAL(1, 26, 6), // "answer"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 6), // "hangup"
QT_MOC_LITERAL(4, 41, 4), // "hold"
QT_MOC_LITERAL(5, 46, 7), // "deflect"
QT_MOC_LITERAL(6, 54, 8), // "sendDtmf"
QT_MOC_LITERAL(7, 63, 5), // "merge"
QT_MOC_LITERAL(8, 69, 5) // "split"

    },
    "YottagramVoiceCallHandler\0answer\0\0"
    "hangup\0hold\0deflect\0sendDtmf\0merge\0"
    "split"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_YottagramVoiceCallHandler[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x0a /* Public */,
       3,    0,   50,    2, 0x0a /* Public */,
       4,    1,   51,    2, 0x0a /* Public */,
       5,    1,   54,    2, 0x0a /* Public */,
       6,    1,   57,    2, 0x0a /* Public */,
       7,    1,   60,    2, 0x0a /* Public */,
       8,    0,   63,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,

       0        // eod
};

void YottagramVoiceCallHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        YottagramVoiceCallHandler *_t = static_cast<YottagramVoiceCallHandler *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->answer(); break;
        case 1: _t->hangup(); break;
        case 2: _t->hold((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->deflect((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->sendDtmf((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->merge((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->split(); break;
        default: ;
        }
    }
}

const QMetaObject YottagramVoiceCallHandler::staticMetaObject = {
    { &AbstractVoiceCallHandler::staticMetaObject, qt_meta_stringdata_YottagramVoiceCallHandler.data,
      qt_meta_data_YottagramVoiceCallHandler,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *YottagramVoiceCallHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *YottagramVoiceCallHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_YottagramVoiceCallHandler.stringdata0))
        return static_cast<void*>(const_cast< YottagramVoiceCallHandler*>(this));
    return AbstractVoiceCallHandler::qt_metacast(_clname);
}

int YottagramVoiceCallHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractVoiceCallHandler::qt_metacall(_c, _id, _a);
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
QT_END_MOC_NAMESPACE
