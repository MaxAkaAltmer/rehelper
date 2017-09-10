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

#include "atime.h"

#include <time.h>
#include <sys/time.h>

ATime ATime::current()
{
    return ATime(uStamp());
}

uint64 ATime::uStamp()
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    if(sizeof(currentTime.tv_sec)==8)
        return (uint64)currentTime.tv_sec*(uint64)1000000+currentTime.tv_usec;

    return ((uint32)currentTime.tv_sec)*(uint64)1000000+currentTime.tv_usec;
}

void ATime::fragmentation(
        int *year,
        int *month,
        int *day,
        int *wday,
        int *hour,
        int *min,
        int *sec,
        int *usec)
{
    time_t t=stamp/1000000;
    struct tm *val=localtime(&t);

    if(year)*year=val->tm_year+1900;
    if(month)*month=val->tm_mon+1;
    if(day)*day=val->tm_mday;
    if(wday)*wday=val->tm_wday;
    if(hour)*hour=val->tm_hour;
    if(min)*min=val->tm_min;
    if(sec)*sec=val->tm_sec;
    if(usec)*usec=stamp%1000000;
}

AString ATime::toString()
{
    time_t t=stamp/1000000;
    struct tm *val=localtime(&t);

    return AString::fromIntFormat(val->tm_mday,2)+"."+
            AString::fromIntFormat(val->tm_mon+1,2)+"."+
            AString::fromIntFormat(val->tm_year+1900,4)+" "+
            AString::fromIntFormat(val->tm_hour,2)+":"+
            AString::fromIntFormat(val->tm_min,2)+":"+
            AString::fromIntFormat(val->tm_sec,2)+"."+
            AString::fromIntFormat((stamp/1000)%1000,3);
}
