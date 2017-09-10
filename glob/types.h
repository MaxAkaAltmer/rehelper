﻿/*****************************************************************************

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

//////////////////////////////////////////////////////////////////
//дополнительные включения
//////////////////////////////////////////////////////////////////

#ifndef TYPES_H
#define TYPES_H

//////////////////////////////////////////////////////////////////
//определение типов
//////////////////////////////////////////////////////////////////

typedef  unsigned char      uint8;
typedef  unsigned short     uint16;
typedef  unsigned int       uint32;
typedef  unsigned long long uint64;

typedef  signed char        int8;
typedef  short              int16;
typedef  int                int32;
typedef  signed long long   int64;

typedef  float              real32;
typedef  double             real64;
#ifndef ANDROID_NDK
typedef  long double        real80;
typedef  long double        realx;
#else
typedef  double             realx;
#endif
typedef  double             real;

typedef  signed long long intx;
typedef  unsigned long long uintx;

typedef  unsigned int uint;

typedef  unsigned short charx;

#ifndef NULL
 #define NULL 0
#endif

inline uintx _type_ptr2int(void *val)
{
    return reinterpret_cast<uintx>(val);
}

template<class T>
inline T* _type_int2ptr(uintx val)
{
    return reinterpret_cast<T*>(val);
}

template<class T>
inline T _type_unaligned_read(void *buff)
{
    T rv;
    for(uint i=0;i<sizeof(T);i++)
    {
        ((uint8*)&rv)[i]=((uint8*)buff)[i];
    }
    return rv;
}

template<class T>
inline void _type_unaligned_write(void *buff, T val)
{
    for(uint i=0;i<sizeof(T);i++)
    {
        ((uint8*)buff)[i]=((uint8*)&val)[i];
    }
}

class ARetCode
{
public:
    ARetCode(){ state=0; }
    ARetCode(const ARetCode &val){ state=val.state; }
    ARetCode(const int &val){ state=val; }
    virtual ~ARetCode(){}

    virtual ARetCode& operator=(const ARetCode &val)
    {
        state=val.state;
        return *this;
    }
    virtual ARetCode& operator=(int val)
    {
        state=val;
        return *this;
    }

    virtual ARetCode& set(int val){state=val;return *this;}
    int get(){return state;}

    uint value() //значение
    {
        if(state<0)return 0;
        return state;
    }
    uint error() //номер ошибки
    {
        if(state>=0)return 0;
        return -state;
    }

    operator int(){return state;}

protected:
    int state;
};

template <typename L, typename R>
class ADual
{
public:

    ADual(){}
    ~ADual(){}
    ADual(L left_val, R right_val){vl=left_val;vr=right_val;}
    ADual(const ADual &val){*this=val;}
    ADual& operator=(const ADual &val){vl=val.vl;vr=val.vr;return *this;}

    L& left(){return vl;}
    R& right(){return vr;}

    L left() const {return vl;}
    R right() const {return vr;}

private:
    L vl;
    R vr;
};

template <typename L, typename M, typename R>
class ATrio
{
public:

    ATrio(){}
    ~ATrio(){}
    ATrio(L left_val, M middle_val, R right_val){vl=left_val;vm=middle_val;vr=right_val;}
    ATrio(const ADual<L,R> &val){vl=val.left();vr=val.right();}
    ATrio(const ATrio &val){*this=val;}
    ATrio& operator=(const ATrio &val){vl=val.vl;vm=val.vm;vr=val.vr;return *this;}

    L& left(){return vl;}
    M& middle(){return vm;}
    R& right(){return vr;}

    L left() const {return vl;}
    M middle() const {return vm;}
    R right() const {return vr;}

private:
    L vl;
    M vm;
    R vr;
};

template <typename T>
class AQuad
{
public:

    AQuad(){}
    ~AQuad(){}
    AQuad(T left_up_val, T right_up_val, T right_down_val, T left_down_val)
    {
        lu=left_up_val;
        ru=right_up_val;
        rd=right_down_val;
        ld=left_down_val;
    }
    AQuad(const AQuad &val){*this=val;}
    AQuad& operator=(const AQuad &val){lu=val.lu;ru=val.ru;rd=val.rd;ld=val.ld;return *this;}

    T operator[](int i) const
    {
        switch(i&3)
        {
        case 0: return lu;
        case 1: return ru;
        case 2: return rd;
        };
        return ld;
    }

    T& leftUp() {return lu;}
    T& leftDown() {return ld;}
    T& rightUp() {return ru;}
    T& rightDown() {return rd;}

    T leftUp() const {return lu;}
    T leftDown() const {return ld;}
    T rightUp() const {return ru;}
    T rightDown() const {return rd;}

private:
    T lu,ru,rd,ld;
};

#endif // TYPES_H
