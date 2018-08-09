/****************************************************************************
** Meta object code from reading C++ file 'CCVG_gui_ProjectSettings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Include/CCVG_gui_ProjectSettings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CCVG_gui_ProjectSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ccvg__gui__CCVGGUIProjectSettings_t {
    QByteArrayData data[8];
    char stringdata0[201];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ccvg__gui__CCVGGUIProjectSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ccvg__gui__CCVGGUIProjectSettings_t qt_meta_stringdata_ccvg__gui__CCVGGUIProjectSettings = {
    {
QT_MOC_LITERAL(0, 0, 33), // "ccvg::gui::CCVGGUIProjectSett..."
QT_MOC_LITERAL(1, 34, 29), // "OnClickBtnGetCCVGResultFolder"
QT_MOC_LITERAL(2, 64, 0), // ""
QT_MOC_LITERAL(3, 65, 29), // "OnClickBtnGetCCVGSourceFolder"
QT_MOC_LITERAL(4, 95, 29), // "OnClickBtnGetCCVGDriverFolder"
QT_MOC_LITERAL(5, 125, 30), // "OnClickGrpBxProjectEnabledFlag"
QT_MOC_LITERAL(6, 156, 9), // "vbChecked"
QT_MOC_LITERAL(7, 166, 34) // "OnClickBtnGetCCVGExecuteableF..."

    },
    "ccvg::gui::CCVGGUIProjectSettings\0"
    "OnClickBtnGetCCVGResultFolder\0\0"
    "OnClickBtnGetCCVGSourceFolder\0"
    "OnClickBtnGetCCVGDriverFolder\0"
    "OnClickGrpBxProjectEnabledFlag\0vbChecked\0"
    "OnClickBtnGetCCVGExecuteableFolder"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ccvg__gui__CCVGGUIProjectSettings[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    0,   41,    2, 0x08 /* Private */,
       5,    1,   42,    2, 0x08 /* Private */,
       7,    0,   45,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void,

       0        // eod
};

void ccvg::gui::CCVGGUIProjectSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CCVGGUIProjectSettings *_t = static_cast<CCVGGUIProjectSettings *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->OnClickBtnGetCCVGResultFolder(); break;
        case 1: _t->OnClickBtnGetCCVGSourceFolder(); break;
        case 2: _t->OnClickBtnGetCCVGDriverFolder(); break;
        case 3: _t->OnClickGrpBxProjectEnabledFlag((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->OnClickBtnGetCCVGExecuteableFolder(); break;
        default: ;
        }
    }
}

const QMetaObject ccvg::gui::CCVGGUIProjectSettings::staticMetaObject = {
    { &afProjectSettingsExtension::staticMetaObject, qt_meta_stringdata_ccvg__gui__CCVGGUIProjectSettings.data,
      qt_meta_data_ccvg__gui__CCVGGUIProjectSettings,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ccvg::gui::CCVGGUIProjectSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ccvg::gui::CCVGGUIProjectSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ccvg__gui__CCVGGUIProjectSettings.stringdata0))
        return static_cast<void*>(const_cast< CCVGGUIProjectSettings*>(this));
    if (!strcmp(_clname, "CCVGClassCmnBase"))
        return static_cast< CCVGClassCmnBase*>(const_cast< CCVGGUIProjectSettings*>(this));
    return afProjectSettingsExtension::qt_metacast(_clname);
}

int ccvg::gui::CCVGGUIProjectSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = afProjectSettingsExtension::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
