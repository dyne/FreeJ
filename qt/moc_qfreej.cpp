/****************************************************************************
** Meta object code from reading C++ file 'qfreej.h'
**
** Created: Sat Nov 6 01:55:27 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qfreej.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qfreej.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Qfreej[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x0a,
      19,    7,    7,    7, 0x0a,
      37,    7,    7,    7, 0x0a,
      52,    7,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Qfreej[] = {
    "Qfreej\0\0addLayer()\0updateInterface()\0"
    "addTextLayer()\0startStreaming()\0"
};

const QMetaObject Qfreej::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Qfreej,
      qt_meta_data_Qfreej, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Qfreej::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Qfreej::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Qfreej::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Qfreej))
        return static_cast<void*>(const_cast< Qfreej*>(this));
    return QWidget::qt_metacast(_clname);
}

int Qfreej::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: addLayer(); break;
        case 1: updateInterface(); break;
        case 2: addTextLayer(); break;
        case 3: startStreaming(); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
