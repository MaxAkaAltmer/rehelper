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

#include "../glob/afile.h"

#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef linux
    #include <io.h>
#endif
#include <fcntl.h>
#include <unistd.h>

bool aDirIsWriteble(AString path)
{
    if(path.last()!='/')path+="/";
    int testInd=0;
    AString fname=path+"test.txt";
    while(AFile::exists(fname))
    {
        fname=path+"test"+AString::fromInt(testInd++)+".txt";
    }
    AFile hand(fname);
    if(hand.open(AFile::OWriteOnly))
    {
        if(hand.write(fname(),fname.size())==fname.size())
        {
            hand.close();
            if(hand.open(AFile::OReadOnly))
            {
                if(hand.readText()==fname)
                {
                    hand.close();
                    AFile::remove(fname);
                    return true;
                }
            }
        }
        hand.close();
        AFile::remove(fname);
    }
    return false;
}

ATArray<AString> aDirEntryList(const AString &path, ATSet<AString> extFilter, int type)
{
    ATArray<AString> rv;
#ifndef linux
    ATArray<wchar_t> upath=path.toUnicode();
    _WDIR *dir;
    struct _wdirent *drnt;
    dir = _wopendir(upath());
    while (dir && (drnt = _wreaddir(dir)) != NULL)
    {
        AString node=AString::fromUnicode(drnt->d_name);
        if(node=="." || node=="..")continue;
        if(type)
        {
            struct stat _Stat;
            ATArray<wchar_t> sub_name=(path+"/"+node).toUnicode();
            wstat(sub_name(),&_Stat);
            if(type<0 && S_ISREG(_Stat.st_mode))continue;
            if(type>0 && !S_ISREG(_Stat.st_mode))continue;
        }
        if(extFilter.size())
        {
            int ind = node.findBackChar('.');
            if(ind<0)continue;
            AString ext=node.right(ind+1).toLower();
            if(!extFilter.contains(ext))continue;
        }
        rv.append(node);
    }
#else
    DIR *dir;
    struct dirent *drnt;
    dir = opendir(path());
    while (dir && (drnt = readdir(dir)) != NULL)
    {
        AString node(drnt->d_name);
        if(node=="." || node=="..")continue;
        if(type)
        {
            if(type>0 && drnt->d_type!=DT_REG)continue;
            else if(type<0 && drnt->d_type==DT_REG)continue;
        }
        if(extFilter.size())
        {
            int ind = node.findBackChar('.');
            if(ind<0)continue;
            AString ext=node.right(ind+1).toLower();
            if(!extFilter.contains(ext))continue;
        }
        rv.append(node);
    }
#endif
    return rv;
}

AFile::AFile()
{
    handler = -1;
}

AFile::AFile(const AString &name)
{
    fname = name;
    handler = -1;
}

AFile::~AFile()
{
    close();
}

bool AFile::setFileName(const AString &name)
{
    if(open_flags)return false;
    fname=name;
    return true;
}

void AFile::close()
{
    if(handler>=0)
    {
        ::close(handler);
        handler = -1;
    }
    AFileProto::close();
}

bool AFile::open(int flags)
{
    if(fname.isEmpty())return false;
    if(!flags)return false;

    int oflags=0;
#ifdef O_LARGEFILE
    oflags|=O_LARGEFILE;
#endif
#ifdef O_BINARY
    oflags|=O_BINARY;
#endif
    if((flags&OReadOnly) && (flags&OWriteOnly))oflags|=O_RDWR;
    else if(flags&OReadOnly)oflags|=O_RDONLY;
    else if(flags&OWriteOnly)oflags|=O_WRONLY;
    if(flags&OAppend)oflags|=O_APPEND;
    if(flags&OTruncate)oflags|=O_TRUNC;

#ifndef linux
    ATArray<wchar_t> wname=fname.toUnicode();
    int hand=::_wopen(wname(),oflags);

    if(hand<0 && (flags&OWriteOnly))
    {
        hand=::_wcreat(wname(),00666);
        if(hand>=0)
        {
            ::close(hand);
            hand=::_wopen(wname(),oflags);
        }
    }
#else
    int hand=::open(fname(),oflags);

    if(hand<0 && (flags&OWriteOnly))
    {
        hand=::creat(fname(),00666);
        if(hand>=0)
        {
            ::close(hand);
            hand=::open(fname(),oflags);
        }
    }
#endif

    if(hand<0)return false;

    open_flags = flags;
    handler=hand;

    return true;
}

int64 AFile::size() const
{
    if(handler<0)return -1;

    int64 oldpos=pos();
    ::lseek64(handler,0,SEEK_END);
    int64 rv=pos();
    ::lseek64(handler,oldpos,SEEK_SET);
    return rv;
}

bool AFile::seek(int64 pos)
{
    if(handler<0)return false;
    return ::lseek64(handler,pos,SEEK_SET)==pos;
}

int64 AFile::pos() const
{
    if(handler<0)return -1;
    return ::lseek64(handler,0,SEEK_CUR);
}

int AFile::read_hand(void *buff, int size)
{
    if(handler<0)return -1;
    if(size<=0)return 0;
    int rv = ::read(handler,buff,size);

    return rv;
}

int AFile::write_hand(const void *buff, int size)
{
    if(handler<0)return -1;
    if(size<=0)return 0;
    return ::write(handler,buff,size);
}

bool AFile::resize(int64 size)
{
    if(handler<0)return false;
    if (::ftruncate64(handler, size) != 0) return false;
    return true;
}

bool AFile::isSequential() const
{
    return false;
}

bool AFile::exists(AString fname)
{
    if(fname.isEmpty())return false;
#ifndef linux
    ATArray<wchar_t> wname=fname.toUnicode();
    if(_waccess(wname(),F_OK)==-1)return false;
#else
    if(access(fname(),F_OK)==-1)return false;
#endif
    return true;
}


bool AFile::remove(AString fname)
{
    if(fname.isEmpty())return false;
#ifndef linux
    ATArray<wchar_t> wname=fname.toUnicode();
    if(_wunlink(wname())==-1)return false;
#else
    if(unlink(fname())==-1)return false;
#endif
    return true;
}

bool AFile::rename(AString before, AString after)
{
#ifndef linux
    ATArray<wchar_t> bname=before.toUnicode();
    ATArray<wchar_t> aname=after.toUnicode();
    if(!(_wrename(bname(),aname())))return true;
#else
    if(!(::rename(before(),after())))return true;
#endif
    return false;
}

ATime AFile::changeTime(AString fname)
{
    struct stat t_stat;
#ifndef linux
    ATArray<wchar_t> wname=fname.toUnicode();
    wstat(wname(), &t_stat);
#else
    stat(fname(), &t_stat);
#endif
    if(sizeof(t_stat.st_mtime)==8)
        return ATime(((uint64)t_stat.st_mtime)*(uint64)1000000);
    return ATime(((uint32)t_stat.st_mtime)*(uint64)1000000);
}

bool AFile::replicate(AString src, AString dst)
{
    AFile hsrc(src);
    AFile hdst(dst);
    if(!hsrc.open())return false;
    if(!hdst.create())return false;

    int64 total=hsrc.size();
    if(hdst.copy(&hsrc,total)!=total)return false;
    return true;
}
