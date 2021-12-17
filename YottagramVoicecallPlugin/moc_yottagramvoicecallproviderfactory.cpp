/****************************************************************************
** Meta object code from reading C++ file 'yottagramvoicecallproviderfactory.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Yottagram/YottagramVoicecallPlugin/yottagramvoicecallproviderfactory.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qplugin.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'yottagramvoicecallproviderfactory.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_YottagramVoiceCallProviderFactory_t {
    QByteArrayData data[10];
    char stringdata0[121];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_YottagramVoiceCallProviderFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_YottagramVoiceCallProviderFactory_t qt_meta_stringdata_YottagramVoiceCallProviderFactory = {
    {
QT_MOC_LITERAL(0, 0, 33), // "YottagramVoiceCallProviderFac..."
QT_MOC_LITERAL(1, 34, 10), // "initialize"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 9), // "configure"
QT_MOC_LITERAL(4, 56, 26), // "VoiceCallManagerInterface*"
QT_MOC_LITERAL(5, 83, 7), // "manager"
QT_MOC_LITERAL(6, 91, 5), // "start"
QT_MOC_LITERAL(7, 97, 7), // "suspend"
QT_MOC_LITERAL(8, 105, 6), // "resume"
QT_MOC_LITERAL(9, 112, 8) // "finalize"

    },
    "YottagramVoiceCallProviderFactory\0"
    "initialize\0\0configure\0VoiceCallManagerInterface*\0"
    "manager\0start\0suspend\0resume\0finalize"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_YottagramVoiceCallProviderFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x0a /* Public */,
       3,    1,   45,    2, 0x0a /* Public */,
       6,    0,   48,    2, 0x0a /* Public */,
       7,    0,   49,    2, 0x0a /* Public */,
       8,    0,   50,    2, 0x0a /* Public */,
       9,    0,   51,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Bool,
    QMetaType::Bool, 0x80000000 | 4,    5,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Void,

       0        // eod
};

void YottagramVoiceCallProviderFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        YottagramVoiceCallProviderFactory *_t = static_cast<YottagramVoiceCallProviderFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: { bool _r = _t->initialize();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 1: { bool _r = _t->configure((*reinterpret_cast< VoiceCallManagerInterface*(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 2: { bool _r = _t->start();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 3: { bool _r = _t->suspend();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 4: { bool _r = _t->resume();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 5: _t->finalize(); break;
        default: ;
        }
    }
}

const QMetaObject YottagramVoiceCallProviderFactory::staticMetaObject = {
    { &AbstractVoiceCallManagerPlugin::staticMetaObject, qt_meta_stringdata_YottagramVoiceCallProviderFactory.data,
      qt_meta_data_YottagramVoiceCallProviderFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *YottagramVoiceCallProviderFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *YottagramVoiceCallProviderFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_YottagramVoiceCallProviderFactory.stringdata0))
        return static_cast<void*>(const_cast< YottagramVoiceCallProviderFactory*>(this));
    if (!strcmp(_clname, "org.nemomobile.voicecall.ManagerPlugin/1.0"))
        return static_cast< AbstractVoiceCallManagerPlugin*>(const_cast< YottagramVoiceCallProviderFactory*>(this));
    return AbstractVoiceCallManagerPlugin::qt_metacast(_clname);
}

int YottagramVoiceCallProviderFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractVoiceCallManagerPlugin::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

QT_PLUGIN_METADATA_SECTION const uint qt_section_alignment_dummy = 42;

#ifdef QT_NO_DEBUG

QT_PLUGIN_METADATA_SECTION
static const unsigned char qt_pluginMetaData[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', ' ',
    'q',  'b',  'j',  's',  0x01, 0x00, 0x00, 0x00,
    0xc0, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
    0xac, 0x00, 0x00, 0x00, 0x1b, 0x03, 0x00, 0x00,
    0x03, 0x00, 'I',  'I',  'D',  0x00, 0x00, 0x00,
    '$',  0x00, 'c',  'o',  'm',  '.',  'v',  'e', 
    'r',  'd',  'a',  'n',  'd',  'i',  't',  'e', 
    'a',  'm',  '.',  'v',  'o',  'i',  'c',  'e', 
    'c',  'a',  'l',  'l',  '.',  'y',  'o',  't', 
    't',  'a',  'g',  'r',  'a',  'm',  0x00, 0x00,
    0x1b, 0x0a, 0x00, 0x00, 0x09, 0x00, 'c',  'l', 
    'a',  's',  's',  'N',  'a',  'm',  'e',  0x00,
    '!',  0x00, 'Y',  'o',  't',  't',  'a',  'g', 
    'r',  'a',  'm',  'V',  'o',  'i',  'c',  'e', 
    'C',  'a',  'l',  'l',  'P',  'r',  'o',  'v', 
    'i',  'd',  'e',  'r',  'F',  'a',  'c',  't', 
    'o',  'r',  'y',  0x00, 'z',  0xc0, 0xa0, 0x00,
    0x07, 0x00, 'v',  'e',  'r',  's',  'i',  'o', 
    'n',  0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
    0x05, 0x00, 'd',  'e',  'b',  'u',  'g',  0x00,
    0x15, 0x14, 0x00, 0x00, 0x08, 0x00, 'M',  'e', 
    't',  'a',  'D',  'a',  't',  'a',  0x00, 0x00,
    0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
    0x90, 0x00, 0x00, 0x00, '@',  0x00, 0x00, 0x00,
    0x84, 0x00, 0x00, 0x00, 't',  0x00, 0x00, 0x00
};

#else // QT_NO_DEBUG

QT_PLUGIN_METADATA_SECTION
static const unsigned char qt_pluginMetaData[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', ' ',
    'q',  'b',  'j',  's',  0x01, 0x00, 0x00, 0x00,
    0xc0, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
    0xac, 0x00, 0x00, 0x00, 0x1b, 0x03, 0x00, 0x00,
    0x03, 0x00, 'I',  'I',  'D',  0x00, 0x00, 0x00,
    '$',  0x00, 'c',  'o',  'm',  '.',  'v',  'e', 
    'r',  'd',  'a',  'n',  'd',  'i',  't',  'e', 
    'a',  'm',  '.',  'v',  'o',  'i',  'c',  'e', 
    'c',  'a',  'l',  'l',  '.',  'y',  'o',  't', 
    't',  'a',  'g',  'r',  'a',  'm',  0x00, 0x00,
    0x15, 0x0a, 0x00, 0x00, 0x08, 0x00, 'M',  'e', 
    't',  'a',  'D',  'a',  't',  'a',  0x00, 0x00,
    0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9b, 0x0d, 0x00, 0x00,
    0x09, 0x00, 'c',  'l',  'a',  's',  's',  'N', 
    'a',  'm',  'e',  0x00, '!',  0x00, 'Y',  'o', 
    't',  't',  'a',  'g',  'r',  'a',  'm',  'V', 
    'o',  'i',  'c',  'e',  'C',  'a',  'l',  'l', 
    'P',  'r',  'o',  'v',  'i',  'd',  'e',  'r', 
    'F',  'a',  'c',  't',  'o',  'r',  'y',  0x00,
    '1',  0x00, 0x00, 0x00, 0x05, 0x00, 'd',  'e', 
    'b',  'u',  'g',  0x00, 'z',  0xc0, 0xa0, 0x00,
    0x07, 0x00, 'v',  'e',  'r',  's',  'i',  'o', 
    'n',  0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
    '@',  0x00, 0x00, 0x00, '\\', 0x00, 0x00, 0x00,
    0x90, 0x00, 0x00, 0x00, 0x9c, 0x00, 0x00, 0x00
};
#endif // QT_NO_DEBUG

QT_MOC_EXPORT_PLUGIN(YottagramVoiceCallProviderFactory, YottagramVoiceCallProviderFactory)

QT_END_MOC_NAMESPACE
