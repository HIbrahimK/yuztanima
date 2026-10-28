/****************************************************************************
** Meta object code from reading C++ file 'ImagePreviewWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/gui/ImagePreviewWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ImagePreviewWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN22InteractiveImageCanvasE_t {};
} // unnamed namespace

template <> constexpr inline auto InteractiveImageCanvas::qt_create_metaobjectdata<qt_meta_tag_ZN22InteractiveImageCanvasE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "InteractiveImageCanvas",
        "facesChanged",
        "",
        "QList<FaceItem>",
        "faces",
        "scaleChanged",
        "newScale",
        "cropRectChanged",
        "QRect",
        "cropRect",
        "zoomStepRequested",
        "steps",
        "QPoint",
        "mousePos",
        "panRequested",
        "dx",
        "dy"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'facesChanged'
        QtMocHelpers::SignalData<void(const QList<FaceItem> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'scaleChanged'
        QtMocHelpers::SignalData<void(double)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 6 },
        }}),
        // Signal 'cropRectChanged'
        QtMocHelpers::SignalData<void(const QRect &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Signal 'zoomStepRequested'
        QtMocHelpers::SignalData<void(int, const QPoint &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 11 }, { 0x80000000 | 12, 13 },
        }}),
        // Signal 'panRequested'
        QtMocHelpers::SignalData<void(int, int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 }, { QMetaType::Int, 16 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<InteractiveImageCanvas, qt_meta_tag_ZN22InteractiveImageCanvasE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject InteractiveImageCanvas::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN22InteractiveImageCanvasE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN22InteractiveImageCanvasE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN22InteractiveImageCanvasE_t>.metaTypes,
    nullptr
} };

void InteractiveImageCanvas::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<InteractiveImageCanvas *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->facesChanged((*reinterpret_cast<std::add_pointer_t<QList<FaceItem>>>(_a[1]))); break;
        case 1: _t->scaleChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 2: _t->cropRectChanged((*reinterpret_cast<std::add_pointer_t<QRect>>(_a[1]))); break;
        case 3: _t->zoomStepRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[2]))); break;
        case 4: _t->panRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<FaceItem> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (InteractiveImageCanvas::*)(const QList<FaceItem> & )>(_a, &InteractiveImageCanvas::facesChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (InteractiveImageCanvas::*)(double )>(_a, &InteractiveImageCanvas::scaleChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (InteractiveImageCanvas::*)(const QRect & )>(_a, &InteractiveImageCanvas::cropRectChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (InteractiveImageCanvas::*)(int , const QPoint & )>(_a, &InteractiveImageCanvas::zoomStepRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (InteractiveImageCanvas::*)(int , int )>(_a, &InteractiveImageCanvas::panRequested, 4))
            return;
    }
}

const QMetaObject *InteractiveImageCanvas::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *InteractiveImageCanvas::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN22InteractiveImageCanvasE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int InteractiveImageCanvas::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void InteractiveImageCanvas::facesChanged(const QList<FaceItem> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void InteractiveImageCanvas::scaleChanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void InteractiveImageCanvas::cropRectChanged(const QRect & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void InteractiveImageCanvas::zoomStepRequested(int _t1, const QPoint & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void InteractiveImageCanvas::panRequested(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}
namespace {
struct qt_meta_tag_ZN18ImagePreviewWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto ImagePreviewWidget::qt_create_metaobjectdata<qt_meta_tag_ZN18ImagePreviewWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ImagePreviewWidget",
        "facesModified",
        "",
        "filePath",
        "QList<FaceItem>",
        "faces",
        "saveSingleRequested",
        "resizePresetChanged",
        "ResizeMode",
        "mode",
        "customSizeModified",
        "width",
        "height",
        "maintainAspect",
        "setZoomFactor",
        "factor",
        "zoomIn",
        "zoomOut",
        "zoomFit",
        "zoomOriginal",
        "onResetFaces",
        "onApplyCrop",
        "onRevertCrop",
        "openCustomSizeDialog",
        "onSliderValueChanged",
        "val",
        "onCanvasFacesChanged",
        "onCropRectSelected",
        "QRect",
        "cropRect",
        "onPresetComboChanged",
        "index",
        "onZoomStepRequested",
        "steps",
        "QPoint",
        "mousePos",
        "onPanRequested",
        "dx",
        "dy"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'facesModified'
        QtMocHelpers::SignalData<void(const QString &, const QList<FaceItem> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { 0x80000000 | 4, 5 },
        }}),
        // Signal 'saveSingleRequested'
        QtMocHelpers::SignalData<void(const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'resizePresetChanged'
        QtMocHelpers::SignalData<void(ResizeMode)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Signal 'customSizeModified'
        QtMocHelpers::SignalData<void(int, int, bool)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 11 }, { QMetaType::Int, 12 }, { QMetaType::Bool, 13 },
        }}),
        // Slot 'setZoomFactor'
        QtMocHelpers::SlotData<void(double)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 15 },
        }}),
        // Slot 'zoomIn'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'zoomOut'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'zoomFit'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'zoomOriginal'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onResetFaces'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onApplyCrop'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onRevertCrop'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'openCustomSizeDialog'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onSliderValueChanged'
        QtMocHelpers::SlotData<void(int)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 25 },
        }}),
        // Slot 'onCanvasFacesChanged'
        QtMocHelpers::SlotData<void(const QList<FaceItem> &)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Slot 'onCropRectSelected'
        QtMocHelpers::SlotData<void(const QRect &)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 28, 29 },
        }}),
        // Slot 'onPresetComboChanged'
        QtMocHelpers::SlotData<void(int)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 31 },
        }}),
        // Slot 'onZoomStepRequested'
        QtMocHelpers::SlotData<void(int, const QPoint &)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 33 }, { 0x80000000 | 34, 35 },
        }}),
        // Slot 'onPanRequested'
        QtMocHelpers::SlotData<void(int, int)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 37 }, { QMetaType::Int, 38 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ImagePreviewWidget, qt_meta_tag_ZN18ImagePreviewWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ImagePreviewWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ImagePreviewWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ImagePreviewWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18ImagePreviewWidgetE_t>.metaTypes,
    nullptr
} };

void ImagePreviewWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ImagePreviewWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->facesModified((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<FaceItem>>>(_a[2]))); break;
        case 1: _t->saveSingleRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->resizePresetChanged((*reinterpret_cast<std::add_pointer_t<ResizeMode>>(_a[1]))); break;
        case 3: _t->customSizeModified((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3]))); break;
        case 4: _t->setZoomFactor((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 5: _t->zoomIn(); break;
        case 6: _t->zoomOut(); break;
        case 7: _t->zoomFit(); break;
        case 8: _t->zoomOriginal(); break;
        case 9: _t->onResetFaces(); break;
        case 10: _t->onApplyCrop(); break;
        case 11: _t->onRevertCrop(); break;
        case 12: _t->openCustomSizeDialog(); break;
        case 13: _t->onSliderValueChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->onCanvasFacesChanged((*reinterpret_cast<std::add_pointer_t<QList<FaceItem>>>(_a[1]))); break;
        case 15: _t->onCropRectSelected((*reinterpret_cast<std::add_pointer_t<QRect>>(_a[1]))); break;
        case 16: _t->onPresetComboChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->onZoomStepRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[2]))); break;
        case 18: _t->onPanRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<FaceItem> >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< ResizeMode >(); break;
            }
            break;
        case 14:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<FaceItem> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ImagePreviewWidget::*)(const QString & , const QList<FaceItem> & )>(_a, &ImagePreviewWidget::facesModified, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ImagePreviewWidget::*)(const QString & )>(_a, &ImagePreviewWidget::saveSingleRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ImagePreviewWidget::*)(ResizeMode )>(_a, &ImagePreviewWidget::resizePresetChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ImagePreviewWidget::*)(int , int , bool )>(_a, &ImagePreviewWidget::customSizeModified, 3))
            return;
    }
}

const QMetaObject *ImagePreviewWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ImagePreviewWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ImagePreviewWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ImagePreviewWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void ImagePreviewWidget::facesModified(const QString & _t1, const QList<FaceItem> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void ImagePreviewWidget::saveSingleRequested(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ImagePreviewWidget::resizePresetChanged(ResizeMode _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void ImagePreviewWidget::customSizeModified(int _t1, int _t2, bool _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP
