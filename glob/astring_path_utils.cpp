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

#include "astring_path_utils.h"

void AStringPathParcer::setPath(const AString &path)
{
    char sep=_defSep;
    for(int i=0;i<path.size();i++)
    {
        if(path[i]=='/' || path[i]=='\\')
        {
            sep=path[i];
            break;
        }
    }

    ATArray<AString> list=split(path);
    ATArray<AString> rez;
    if(list.size())rez.append(list[0]);
    for(int i=1;i<list.size();i++)
    {
        if(list[i]=="..")
        {
            if(rez.size() && rez.last()!="..")
            {
                rez.pop();
                continue;
            }
            else
            {
                rez.append(list[i]);
            }
        }
        else if(list[i]==".")
        {
            if(rez.size())
            {
                continue;
            }
            else
            {
                rez.append(list[i]);
            }
        }
        else if(list[i].isEmpty())
        {
            continue;
        }
        else
        {
            rez.append(list[i]);
        }
    }
    _path=AString::join(rez,sep);
}

ATArray<AString> AStringPathParcer::split(const AString &path)
{
    ATArray<AString> rv;
    AString tmp;
    for(int i=0;i<path.size();i++)
    {
        if(path[i]=='/' || path[i]=='\\')
        {
            rv.append(tmp);
            tmp.clear();
        }
        else
        {
            tmp.append(path[i]);
        }
    }
    if(!tmp.isEmpty())rv.append(tmp);
    return rv;
}

AString AStringPathParcer::getExtension()
{
    for(int i=_path.size();i>=0;i--)
    {
        if(_path[i]=='/' || _path[i]=='\\')return AString();
        if(_path[i]=='.')return _path.right(i+1);
    }
    return AString();
}

AString AStringPathParcer::getName()
{
    for(int i=_path.size();i>=0;i--)
    {
        if(_path[i]=='/' || _path[i]=='\\')return _path.right(i+1);
    }
    return _path;
}

AString AStringPathParcer::getBaseName()
{
    AString tmp=getName();
    for(int i=0;i<tmp.size();i++)
    {
        if(tmp[i]=='.')return tmp.left(i);
    }
    return tmp;
}

AString AStringPathParcer::getNameNoExt()
{
    AString tmp=getName();
    for(int i=tmp.size();i>=0;i--)
    {
        if(tmp[i]=='.')return tmp.left(i);
    }
    return tmp;
}

AString AStringPathParcer::getDirectory()
{
    for(int i=_path.size();i>=0;i--)
    {
        if(_path[i]=='/' || _path[i]=='\\')return _path.left(i);
    }
    return AString();
}

