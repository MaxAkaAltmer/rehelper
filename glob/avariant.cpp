/*****************************************************************************

Copyright (C) 2009 Гришин Максим Леонидович (altmer@arts-union.ru)
          (C) 2009 Maxim L. Grishin  (altmer@arts-union.ru)

        Программное обеспечение (ПО) предоставляется "как есть" - без явной и
неявной гарантии. Авторы не несут никакой ответственности за любые убытки,
связанные с использованием данного ПО, вы используете его на свой страх и риск.
        Данное ПО не предназначено для использования в странах с патентной
системой разрешающей патентование алгоритмов или программ.
        Авторы разрешают свободное использование данного ПО всем желающим,
в том числе в коммерческих целях.
        На распространение измененных версий ПО накладываются следующие ограничения:
        1. Запрещается утверждать, что это вы написали оригинальный продукт;
        2. Измененные версии не должны выдаваться за оригинальный продукт;
        3. Вам запрещается менять или удалять данное уведомление из пакетов с исходными текстами.

*****************************************************************************/

#include "avariant.h"
#include <math.h>

bool AVariant::toBool(bool *ok) const
{
    if(ok)*ok=true;
    switch(type)
    {
    case tBool:
        return data.vBool;
    case tInt:
        return data.vInt;
    case tReal:
        return __round<intx>(data.vReal);
    case tString:
        if((*data.vString)=="true")return true;
        return false;
    default:
        if(ok)*ok=false;
        break;
    };
    return false;
}

intx AVariant::toInt(bool *ok) const
{
    if(ok)*ok=true;
    switch(type)
    {
    case tBool:
        return data.vBool?1:0;
    case tInt:
        return data.vInt;
    case tReal:
        return __round<intx>(data.vReal);
    case tString:
        return (*data.vString).toInt<intx>(10,ok);
    default:
        if(ok)*ok=false;
        break;
    }    
    return 0;
}

realx AVariant::toReal(bool *ok) const
{
    if(ok)*ok=true;
    switch(type)
    {
    case tBool:
        return data.vBool?1:0;
    case tInt:
        return data.vInt;
    case tReal:
        return data.vReal;
    case tString:
        return (*data.vString).toReal<realx>();
    default:
        if(ok)*ok=false;
        break;
    }    
    return 0.0;
}

AString AVariant::toString(bool *ok) const
{
    if(ok)*ok=true;
    switch(type)
    {
    case tBool:
        if(data.vBool)return "true";
        return "false";
    case tInt:
        return AString::fromInt(data.vInt);
    case tString:
        return (*data.vString);
    case tReal:
        return AString::fromReal(data.vReal,20);
    case tData:
        return (*data.vData).toHex();
    default:
        if(ok)*ok=false;
        break;
    }    
    return AString();
}

AData AVariant::toData(bool *ok) const
{
    if(isString())
    {
        if(ok)*ok=true;
        return AData::fromHex((*data.vString)());
    }
    if(isData())
    {
        if(ok)*ok=true;
        return *data.vData;
    }
    if(ok)*ok=false;
    return AData();
}

ATHash<AString,AVariant> AVariant::toHash(bool *ok) const
{
    if(isHash())
    {
        if(ok)*ok=true;
        return (*data.vHash);
    }
    if(ok)*ok=false;
    return ATHash<AString,AVariant>();
}

ATArray<AVariant> AVariant::toArray(bool *ok) const
{
    if(isArray())
    {
        if(ok)*ok=true;
        return (*data.vArray);
    }
    if(ok)*ok=false;
    return ATArray<AVariant>();
}

ATArray<AVariant>* AVariant::pointArray()
{
    if(isArray())return data.vArray;
    return NULL;
}

void* AVariant::toPointer(bool *ok) const
{
    if(isPointer())
    {
        if(ok)*ok=true;
        return (data.vPointer);
    }
    else if(isData())
    {
        if(ok)*ok=true;
        return (*data.vData)();
    }
    else if(isString())
    {
        if(ok)*ok=true;
        return (*data.vString)();
    }
    if(ok)*ok=false;
    return NULL;
}

bool AVariant::operator==(const AVariant &val) const
{
    if(type!=val.type)
    {
        if(type==tInvalide || val.type==tInvalide)return false;
        if(type==tPointer || val.type==tPointer)return false;
        if(type==tHash || val.type==tHash)return false;
        if(type==tArray || val.type==tArray)return false;

        if(toString()==val.toString())return true;
        return false;
    }
    if(type==tData && (*data.vData)==(*val.data.vData))return true;
    if(type==tArray && (*data.vArray)==(*val.data.vArray))return true;
    if(type==tHash && (*data.vHash)==(*val.data.vHash))return true;
    if(type==tString && (*data.vString)==(*val.data.vString))return true;
    if(type==tBool && data.vBool==val.data.vBool)return true;
    if(type==tReal && data.vReal==val.data.vReal)return true;
    if(type==tPointer && data.vPointer==val.data.vPointer)return true;
    if(type==tInvalide)return true;
    return false;
}

AVariant& AVariant::operator=(const AVariant &val)
{
    if(type!=val.type)clear();    
    switch(val.type)
    {
    case tData:
        if(type!=val.type)data.vData=new AData;
        *(data.vData)=*(val.data.vData);
        break;
    case tString:
        if(type!=val.type)data.vString=new AString;
        *(data.vString)=*(val.data.vString);
        break;
    case tHash:
        if(type!=val.type)data.vHash=new ATHash<AString,AVariant>;
        *(data.vHash)=*(val.data.vHash);
        break;
    case tArray:
        if(type!=val.type)data.vArray=new ATArray<AVariant>;
        *(data.vArray)=*(val.data.vArray);
        break;
    default:
        data=val.data;
    }
    type=val.type;
    return *this;
}
