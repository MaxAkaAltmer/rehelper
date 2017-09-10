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

#ifndef AFILE_H
#define AFILE_H

#include "adata.h"
#include "astring_path_utils.h"
#include "atime.h"

class AFileProto
{
public:
    AFileProto(){open_flags=0;}
    virtual ~AFileProto(){}

    enum OpenFlags
    {
        OReadOnly = 0x0001,
        OWriteOnly = 0x0002,
        OReadWrite = OReadOnly | OWriteOnly,
        OAppend = 0x0004,
        OTruncate = 0x0008
    };
    bool isOpen() const
        {return open_flags;}
    bool isReadable() const
        {return open_flags&OReadOnly;}
    virtual bool isSequential() const
        {return false;}
    bool isWritable() const
        {return open_flags&OWriteOnly;}
    int openFlags(){return open_flags;}

    virtual void close(){open_flags=0;}

    virtual bool open(int flags){open_flags=flags;return true;}

    virtual int64 size() const {return 0;}
    virtual bool seek(int64 pos){return false;}
    virtual int64 pos() const {return 0;}

    virtual bool atEnd()
    {
        int64 off=pos();
        int64 siz=size();
        if(off==siz || off<0 || siz<0)return true;
        return false;
    }

    AData read(int siz=-1)
    {
        AData rv;
        if(siz<0)siz=size();
        rv.reserve(siz);
        int cnt=read(rv(),siz);
        if(cnt<0)return AData();
        rv.resize(cnt);
        return rv;
    }

    AString readText(int siz=-1)
    {
        AString rv;
        if(siz<0)siz=size();
        rv.reserve(siz);
        int cnt=read(rv(),siz);
        if(cnt<0)return AString();
        rv.resize(cnt);
        return rv;
    }

    char* gets(char *str, int num) //port helper
    {
        if(atEnd())return NULL;
        int cnt=0;
        _ats_memset<char>(str,0,num);
        while(cnt<(num-1) && !atEnd())
        {
            if(read(str+cnt,1)<=0)break;
            if(!str[cnt] || str[cnt]=='\r' || str[cnt]=='\n')break;
            cnt++;
        }
        return str;
    }

    int64 copy(AFileProto *hand, int64 size)
    {
        const static int buffSize=1024*128;
        int64 total=0;
        if(size<0)return -1;
        uint8 *buff=new uint8[buffSize];
        while(total<size)
        {
            int lim=buffSize;
            if(lim>size-total)lim=size-total;
            int readed=hand->read(buff,lim);
            if(readed<0){total=-1;break;}
            if(readed!=0)write(buff,readed);
            total+=readed;
            if(readed!=lim)break;
        }
        delete []buff;
        return total;
    }

    int write(const AData &buff)
    {
        return write(buff(),buff.size());
    }

    int read(void *buff, int size){return read_hand(buff,size);}
    int write(const void *buff, int size){return write_hand(buff,size);}

protected:
    int open_flags;

    virtual int read_hand(void *buff, int size){return 0;}
    virtual int write_hand(const void *buff, int size){return 0;}
};

//////////////////////////////////////////////////////////////////////
//интерфейс для работы с файлами
class AFile: public AFileProto
{
public:
    AFile();
    AFile(const AString &name);
    virtual ~AFile();

    AString fileName(){return fname;}
    bool setFileName(const AString &name);

    void close();
    bool open(int flags=OReadOnly);
    bool create(){return open(OTruncate|OReadWrite);}

    int64 size() const;
    bool seek(int64 pos);
    int64 pos() const;

    bool resize(int64 size);
    bool isSequential() const;

    static ATime changeTime(AString fname);

    static bool exists(AString fname);
    static bool remove(AString fname);
    static bool rename(AString before, AString after);
    static bool replicate(AString src, AString dst);

private:

    int read_hand(void *buff, int size);
    int write_hand(const void *buff, int size);

    AString fname;
    int handler;
};

//type: 0 - all, 1 - files, -1 - directories
ATArray<AString> aDirEntryList(const AString &path, ATSet<AString> extFilter=ATSet<AString>(), int type=0);
bool aDirIsWriteble(AString path);

#endif // AFILE_H
