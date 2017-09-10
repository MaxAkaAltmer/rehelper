﻿
/*****************************************************************************

Copyright (C) 2006 Гришин Максим Леонидович (altmer@arts-union.ru)
          (C) 2006 Maxim L. Grishin  (altmer@arts-union.ru)

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


#ifndef INTMATH_UTILS_HEADER_DEFINITION
#define INTMATH_UTILS_HEADER_DEFINITION

#include "types.h"

__inline uint8 __rev8(uint8 x)
{
 x = (((x & 0xaa) >> 1) | ((x << 1) & 0xaa));
 x = (((x & 0xcc) >> 2) | ((x << 2) & 0xcc));
 x = ((x >> 4) | (x << 4));
 return x;
}

__inline uint32 __rev32(uint32 x)
{
 x = (((x & 0xaaaaaaaa) >> 1) | ((x << 1) & 0xaaaaaaaa));
 x = (((x & 0xcccccccc) >> 2) | ((x << 2) & 0xcccccccc));
 x = (((x & 0xf0f0f0f0) >> 4) | ((x << 4) & 0xf0f0f0f0));
 x = (((x & 0xff00ff00) >> 8) | ((x << 8) & 0xff00ff00));
 x = ((x >> 16) | (x << 16));
 return x;
}

template <class T>
__inline T __abs(T val)
{
        if(val<0)return -val;
        return val;
}

template <class T>
__inline T __exign(T val, int byte_count)
{
    if( val&((T)1<<(8*byte_count-1)) )
    {
        val|=(~((T)0))<<(8*byte_count-1);
    }
    return val;
}

template <class T>
__inline void __swap(T &v1, T &v2)
{
    T tmp=v1;
    v1=v2;
    v2=tmp;
}

template <class T>
__inline T __max_val(T &v1, T &v2)
{
    if(v1>v2)return v1;
    return v2;
}

template <class T>
__inline T __min_val(T &v1, T &v2)
{
    if(v1<v2)return v1;
    return v2;
}

template <class I, class R>
__inline I __round(R val)
{
    return val >= R(0.0) ? I(val + R(0.5))
                         : I(val - I(val-1) + R(0.5)) + I(val-1);
}

__inline int __bsr32(uint32 num)  //число бит необходимых на значение
{
 int retval;

        if(!num)return 1;
        //можно оптимизировать командой x86 - bsr
        if(num>>16){num>>=16;retval=16;}
        else retval=0;
        if(num>>8){num>>=8;retval+=8;}
        if(num>>4){num>>=4;retval+=4;}
        if(num>>2){num>>=2;retval+=2;}
        if(num>>1){num>>=1;retval+=2;}
        else if(num)retval++;

        return retval;
}

__inline uint32 __bsr64(uint64 val)
{
 uint32 retval;
        if(val>>32)
        {
                retval=__bsr32(val>>=32);
                retval+=32;
        }
        else retval=__bsr32(val);

        return retval;
}

template <class T>
uint32 _bsrT(T val)
{
        return __bsr64((uint64)val);
}

__inline uint32 __hbc32(uint32 x)  //подсчет единичных бит
{
        x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
        x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
        x = (x & 0xffff) + (x >> 16);
        x = (x & 0xf0f) + ((x >> 4) & 0xf0f);
        return (x&0xff)+ (x>>8);
}

__inline uint16 __bswap16(uint16 x)
{
    return (x>>8) | (x<<8);
}

__inline uint32 __bswap32by16(uint32 x)
{
    return (x>>16) | (x<<16);
}

__inline uint64 __bswap64by16(uint64 x)
{
    return (x>>48) | ((x>>16)&0xffff0000) | ((x&0xffff0000)<<16) | (x<<48);
}

__inline uint32 __bswap32(uint32 x)
{
    return (x>>24) | ((x>>8)&0x0000FF00L) | ((x&0x0000FF00L)<<8) | (x<<24);
}

__inline uint64 __bswap64(uint64 x)
{
    return (x>>56) | ((x>>40)&0x0000FF00) | ((x>>24)&0x00FF0000) | ((x>>8)&0xFF000000) |
            ((x&0xFF000000)<<8) | ((x&0x00FF0000)<<24) | ((x&0x0000FF00)<<40) | (x<<56);
}

__inline uint32 __rotr32(uint32 val, uint32 shift)
{
    //на большинстве процессоров можно оптимизировать
    if(!shift)return val;  //бывают тупые компиляторы в которых сдвиг на ноль непредсказуем
    return (val>>shift)|(val<<(32-shift));
}

template <class T>
__inline T __rotr(T val, T shift)
{
    shift&=(sizeof(T)*8-1);
    if(!shift)return val;
    return (val>>shift)|(val<<(sizeof(T)*8-shift));
}

template <class T>
__inline T __rotl(T val, T shift)
{
    shift&=(sizeof(T)*8-1);
    if(!shift)return val;
    return (val<<shift)|(val>>(sizeof(T)*8-shift));
}

__inline uint32 __isqrt32( uint32 val )
{
 uint32 tmp, mask, root=0;

        if(!val)return root;
        //считаем необходимое число циклов
        //можно оптимизировать командой x86 - bsr
        mask=val;
        if(mask>>16){mask>>=16;tmp=16;}
        else tmp=0;
        if(mask>>8){mask>>=8;tmp+=8;}
        if(mask>>4){mask>>=4;tmp+=4;}
        if(mask>>2)tmp+=2;
        mask=1<<tmp;

        do   //цикл вычисления корня
        {
                tmp=root|mask;
                root>>=1;
                if(tmp<=val)
                {
                        val-=tmp;
                        root|=mask;
                }
                mask>>=2;
        }while(mask);

        if(root<val)root++;   //округление до ближайшего целого

        return root;
}

//довольно медленная процедура - просто для коллекции,
//лучше использовать сопроцессор или библиотеку gmpext (mpx_ln)
__inline int32 __iln32(uint32 val, uint32 rezfix)
{
 uint32 x, cnt;
 uint64 tmp, curr,dop;
        if(!val)return 0x80000000;
        if(val==1)return 0;
        //далее вычисления по ряду ln((1+x)/(1-x))=2*(x+x^3/3+x^5/5+x^7/7+....)
        //до тех пор пока x^n/n!=0
        //x=(val-1)/(val+1)
        rezfix++;
        x=((val-1)<<rezfix)/(val+1);
        curr=tmp=x;
        tmp*=tmp;
        tmp>>=rezfix;

        cnt=1;
        do
        {
                cnt+=2;
                curr*=tmp;
                dop=(curr/cnt)>>rezfix;
                x+=dop;
                curr>>=rezfix;
        }
        while(dop);

        return x;
}



#endif
