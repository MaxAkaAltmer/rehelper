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

#ifndef ATIME_H
#define ATIME_H

#include "astring.h"

class ATime
{
public:
    ATime(uint64 us=0)
    {
        stamp=us;
    }

    ATime(const ATime &val){*this=val;}

    ATime& operator=(const ATime &val)
    {
        stamp=val.stamp;
        return *this;
    }

    uint64 uSeconds(){return stamp;}

    void fragmentation(int *year=NULL,
            int *month=NULL,
            int *day=NULL,
            int *wday=NULL,
            int *hour=NULL,
            int *min=NULL,
            int *sec=NULL,
            int *usec=NULL);

    static ATime current();
    static uint64 uStamp();

    AString toString();

private:

    uint64 stamp;
};

#endif // ATIME_H
