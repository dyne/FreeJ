/****************************************************************************
** Meta object code from reading C++ file 'QqComboBlit.h'
**
** Created: Tue Nov 2 16:03:40 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "QqComboBlit.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QqComboBlit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QqComboBlit[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x0a,
      30,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QqComboBlit[] = {
    "QqComboBlit\0\0addBlit(QString)\0"
    "chgParam(double)\0"
};

const QMetaObject QqComboBlit::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QqComboBlit,
      qt_meta_data_QqComboBlit, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QqComboBlit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QqComboBlit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QqComboBlit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QqComboBlit))
        return static_cast<void*>(const_cast< QqComboBlit*>(this));
    return QWidget::qt_metacast(_clname);
}

int QqComboBlit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: addBlit((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: chgParam((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
