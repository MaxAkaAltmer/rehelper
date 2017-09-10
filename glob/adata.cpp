﻿/*****************************************************************************

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

#include "adata.h"

ADataInternal AData::empty={0,0,0,{0}};

AData& AData::operator=(const AData &val)
{
    if(data==val.data)return *this;
    if((data->refcount<2) && (data->alloc >= val.data->size) )
    {
        if(val.data->size)
            _ats_memcpy(data->buff,val.data->buff,val.data->size);
        data->size=val.data->size;
        return *this;
    }
    deleteInternal();
    data=val.data;
    data->refcount++;
    return *this;
}

AData& AData::operator+=(const AData &val)
{
    int nsiz=val.data->size+data->size;
    if(!val.data->size)return *this;

    if(data->alloc>=nsiz && data->refcount<2)
    {
        _ats_memcpy(&data->buff[data->size],val.data->buff,val.data->size);
        data->size=nsiz;
        return *this;
    }
    ADataInternal *tmp=newInternal(nsiz);
    if(data->size)_ats_memcpy(tmp->buff,data->buff,data->size);
    _ats_memcpy(&tmp->buff[data->size],val.data->buff,val.data->size);
    deleteInternal();
    data=tmp;
    return *this;
}

AData AData::operator+(const AData &val) const
{
    if(!val.data->size)return *this;
    if(!data->size)return val;
    int nsiz=val.data->size+data->size;

    AData tmp(nsiz);
    _ats_memcpy(tmp.data->buff,data->buff,data->size);
    _ats_memcpy(&tmp.data->buff[data->size],val.data->buff,val.data->size);
    return tmp;
}

AData& AData::append(uint8 val)
{
    int nsiz=data->size+1;
    if(data->alloc>=nsiz && data->refcount<2)
    {
        data->buff[data->size]=val;
        data->size=nsiz;
        return *this;
    }

    ADataInternal *tmp=newInternal(nsiz);
    if(data->size)_ats_memcpy(tmp->buff,data->buff,data->size);
    tmp->buff[data->size]=val;
    deleteInternal();
    data=tmp;
    return *this;
}

AData& AData::append(const void *buff, int size)
{
    if(size<=0)return *this;
    int nsiz=data->size+size;
    if(data->alloc>=nsiz && data->refcount<2)
    {
        _ats_memcpy(&data->buff[data->size],(uint8*)buff,size);
        data->size=nsiz;
        return *this;
    }

    ADataInternal *tmp=newInternal(nsiz);
    if(data->size)_ats_memcpy(tmp->buff,data->buff,data->size);
    _ats_memcpy(&tmp->buff[data->size],(uint8*)buff,size);
    deleteInternal();
    data=tmp;
    return *this;
}

AData& AData::prepend(uint8 val)
{
    int nsiz=data->size+1;
    if(data->alloc>=nsiz && data->refcount<2)
    {
        if(data->size)_ats_memcpy(data->buff+1,data->buff,data->size);
        data->buff[0]=val;
        data->size=nsiz;
        return *this;
    }

    ADataInternal *tmp=newInternal(nsiz);
    tmp->buff[0]=val;
    if(data->size)_ats_memcpy(tmp->buff+1,data->buff,data->size);
    deleteInternal();
    data=tmp;
    return *this;
}

AData& AData::prepend(const void *buff, int size)
{
    if(size<=0)return *this;
    int nsiz=data->size+size;
    if(data->alloc>=nsiz && data->refcount<2)
    {
        if(data->size)_ats_memcpy(data->buff+size,data->buff,data->size);
        _ats_memcpy(data->buff,(uint8*)buff,size);
        data->size=nsiz;
        return *this;
    }

    ADataInternal *tmp=newInternal(nsiz);
    _ats_memcpy(tmp->buff,(uint8*)buff,size);
    if(data->size)_ats_memcpy(tmp->buff+size,data->buff,data->size);
    deleteInternal();
    data=tmp;
    return *this;
}

AData& AData::remove(int ind, int size)
{
    if(size<=0 || ind<0 || ind>=data->size)return *this;

    int rsiz=size;
    if(ind+rsiz>data->size)rsiz=data->size-ind;

    if(rsiz+ind!=data->size)
    {
        _ats_memcpy(data->buff+ind,data->buff+ind+rsiz,data->size-ind-rsiz);
    }

    data->size-=rsiz;
    return *this;
}

uint8 _adata_make_half_hex(uint8 val, bool up)
{
    if(val<10)return '0'+val;
    if(up)return 'A'+val-10;
    return 'a'+val-10;
}

AString AData::toCPPArray(AString name, bool up_case)
{
    AString rv="static const uint8 "+name+"[] =\r\n{";

    for(int i=0;i<data->size;i++)
    {
        if(!(i&0xf))rv+="\r\n\t";
        rv+="0x";
        uint8 val=data->buff[i];
        rv.append(_adata_make_half_hex(val>>4,up_case));
        rv.append(_adata_make_half_hex(val&0xf,up_case));
        if(i<data->size-1)rv+=",";
    }

    return rv+"\r\n};\r\n";
}

AString AData::toHex(bool up_case, bool sep)
{
    AString rv;
    if(!data->size)return rv;

    rv.resize(data->size*2+(sep?data->size-1:0));
    char *buff=rv();
    int k=0;
    for(int i=0;i<data->size;i++)
    {
        uint8 val=data->buff[i];
        if(sep && i)buff[k++]=' ';
        buff[k++]=_adata_make_half_hex(val>>4,up_case);
        buff[k++]=_adata_make_half_hex(val&0xf,up_case);
    }
    return rv;
}

AData AData::fromHex(const char *buff, int size)
{
    AData rv;

    if(size<0)size=_ats_strlen(buff);
    rv.resize((size+1)>>1);

    for(int i=size-1;i>=0;i--)
    {
        uint8 half=0;
        if(buff[i]>='0' && buff[i]<='9')
        {
            half=buff[i]-'0';
        }
        else if(buff[i]>='A' && buff[i]<='F')
        {
            half=buff[i]-'A'+10;
        }
        else if(buff[i]>='a' && buff[i]<='f')
        {
            half=buff[i]-'a'+10;
        }

        if((i^size)&1)rv.data->buff[i>>1]=half;
        else rv.data->buff[i>>1]|=half<<4;
    }

    return rv;
}
