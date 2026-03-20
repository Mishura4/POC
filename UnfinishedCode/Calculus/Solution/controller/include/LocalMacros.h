
#include "Macros.h"

// wstring to xstring
#define wtox(PARAM) WTXS(PARAM)
//#define wtox(PARAM) ToXString<wchar_t>(PARAM)

// qstring to xstring
#define qtox(PARAM) wtox((wchar_t*)(PARAM.utf16()))

#define AddQmlClass(_ObjType_, _Obj_, _QmlObjName_) \
    if(_Obj_##Ptr) { HostDelete(_Obj_##Ptr); } \
    _Obj_##Ptr = new _ObjType_(); \
    MoEngine.rootContext()->setContextProperty(_QmlObjName_, _Obj_##Ptr); \
    qRegisterMetaType<_ObjType_*>("const " #_ObjType_ "*");
