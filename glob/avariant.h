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

#ifndef AVARIANT_H
#define AVARIANT_H

#include "types.h"
#include "at_hash.h"
#include "at_array.h"
#include "adata.h"
#include "astring.h"

//todo: конвертация Real в строку и обратно

class AVariant
{
public:
    AVariant()
    {
        type=tInvalide;
    }
    AVariant(const AVariant &val)
    {
        type=tInvalide;
        *this=val;
    }
    AVariant(bool val)
    {
        type=tBool;
        data.vBool=val;
    }
    AVariant(intx val)
    {
        type=tInt;
        data.vInt=val;
    }
    AVariant(int val)
    {
        type=tInt;
        data.vInt=val;
    }
    AVariant(uintx val)
    {
        type=tInt;
        data.vInt=val;
    }
    AVariant(uint val)
    {
        type=tInt;
        data.vInt=val;
    }
#ifndef ANDROID_NDK
    AVariant(realx val)
    {
        type=tReal;
        data.vReal=val;
    }
#endif
    AVariant(real val)
    {
        type=tReal;
        data.vReal=val;
    }
    AVariant(const AString &val)
    {
        type=tString;
        data.vString=new AString;
        *data.vString=val;
    }
    AVariant(const AData &val)
    {
        type=tData;
        data.vData=new AData;
        *data.vData=val;
    }
    AVariant(const ATHash<AString,AVariant> &val)
    {
        type=tHash;
        data.vHash=new ATHash<AString,AVariant>;
        *data.vHash=val;
    }
    AVariant(const ATArray<AVariant> &val)
    {
        type=tArray;
        data.vArray=new ATArray<AVariant>;
        *data.vArray=val;
    }

    AVariant(const char *val)
    {
        type=tString;
        data.vString=new AString(val);
    }

    AVariant(void *val, int size)
    {
        if(size<=0)
        {
            type=tPointer;
            data.vPointer=val;
        }
        else
        {
            type=tData;
            data.vData=new AData(val,size);
        }
    }

    ~AVariant()
    {
        clear();
    }

    void clear()
    {
        switch(type)
        {
        case tString:
            delete data.vString;
            break;
        case tData:
            delete data.vData;
            break;
        case tHash:
            delete data.vHash;
            break;
        case tArray:
            delete data.vArray;
            break;
        default:
            break;
        };        
        type=tInvalide;
    }

    //////////////////////////////////////////////////////////////////////////////////
    //преобразовалки
    bool toBool(bool *ok=NULL) const;
    intx toInt(bool *ok=NULL) const;
    uintx toUInt(bool *ok=NULL) const {return toInt(ok);}
    realx toReal(bool *ok=NULL) const;
    AData toData(bool *ok=NULL) const;
    AString toString(bool *ok=NULL) const;
    ATHash<AString,AVariant> toHash(bool *ok=NULL) const;
    ATArray<AVariant> toArray(bool *ok=NULL) const;
    ATArray<AVariant>* pointArray();
    void* toPointer(bool *ok=NULL) const;    

    bool isValid() const {return type!=tInvalide;}
    bool isBool() const {return type==tBool;}
    bool isInt() const {return type==tInt;}
    bool isReal() const {return type==tReal;}
    bool isString() const {return type==tString;}
    bool isData() const {return type==tData;}
    bool isHash() const {return type==tHash;}
    bool isArray() const {return type==tArray;}
    bool isPointer() const {return type==tPointer;}

    AVariant& operator=(const AVariant &val);

    bool operator==(const AVariant &val) const;
    bool operator!=(const AVariant &val) const
    {
        return !((*this)==val);
    }

private:

    enum Type
    {
        tInvalide = 0,
        tBool,
        tInt,
        tReal,
        tString,
        tData,
        tHash,
        tArray,
        tPointer
    };

    Type type;
    union
    {
        //стандартные данные
        bool vBool;
        intx vInt;
        realx vReal;
        AString *vString;
        AData *vData;
        ATHash<AString,AVariant> *vHash;
        ATArray<AVariant> *vArray;
        void *vPointer;
    }data;
};

#endif // AVARIANT_H
