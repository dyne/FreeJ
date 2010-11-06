/****************************************************************************
** Meta object code from reading C++ file 'QqComboFilter.h'
**
** Created: Sat Nov 6 00:03:58 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "QqComboFilter.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QqComboFilter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QqComboFilter[] = {

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
      15,   14,   14,   14, 0x0a,
      34,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QqComboFilter[] = {
    "QqComboFilter\0\0addFilter(QString)\0"
    "chgParam(double)\0"
};

const QMetaObject QqComboFilter::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QqComboFilter,
      qt_meta_data_QqComboFilter, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QqComboFilter::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QqComboFilter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QqComboFilter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QqComboFilter))
        return static_cast<void*>(const_cast< QqComboFilter*>(this));
    return QWidget::qt_metacast(_clname);
}

int QqComboFilter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: addFilter((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: chgParam((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
