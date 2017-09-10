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

#ifndef ASTRING_FS_UTILS_H
#define ASTRING_FS_UTILS_H

#include "astring.h"
#include "at_array.h"

class AStringPathParcer
{
public:
    AStringPathParcer(){_defSep='/';}
    AStringPathParcer(const AString &path){_defSep='/'; setPath(path);}
    ~AStringPathParcer(){}

    void setDefSep(char sep){_defSep=sep;}
    void setPath(const AString &path);

    static ATArray<AString> split(const AString &path);

    AString getExtension();
    AString getName();
    AString getBaseName();
    AString getNameNoExt();
    AString getDirectory();
    AString getPath(){return _path;}

private:

    AString _path;
    char _defSep;
};

#endif // ASTRING_FS_UTILS_H
