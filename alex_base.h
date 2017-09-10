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

#ifndef ALEXBASE_H
#define ALEXBASE_H

#include "astring.h"
#include "avariant.h"
#include "afile.h"
#include "astring_path_utils.h"

struct ALexElement
{
    int type;
    int file_index;
    int size;
    int offset;
    int line,columne;
    ATArray<AVariant> extra;
    ATArray<ALexElement> macroLexStack;
};

struct ALexErrorInfo
{
    int realCode;
    AString fname;
    int lineCounter;
    int symCounter;
    int realOffset;
};

class ALexFileNode
{
public:

    //кострукторы, деструкторы и операторы
    ALexFileNode(const AString &fname, const AData &src, int off=0)
    {
        data=src;
        offset=off;
        path=fname;
        currPos=0;
        lineCounter=0;
        symCounter=0;
    }
    ALexFileNode()
    {
        offset=0;
        currPos=0;
        lineCounter=0;
        symCounter=0;
    }
    ALexFileNode(const ALexFileNode &val)
    {
        *this=val;
    }
    virtual ~ALexFileNode(){}

    ALexFileNode& operator=(const ALexFileNode &val)
    {
        offset=val.offset;
        currPos=val.currPos;
        lineCounter=val.lineCounter;
        symCounter=val.symCounter;
        path=val.path;
        data=val.data;
        return *this;
    }

    //работа с ошибками
    ALexErrorInfo fixError(int code) const
    {
        ALexErrorInfo rv;
        rv.fname=path;
        rv.lineCounter=lineCounter;
        rv.realCode=code;
        rv.realOffset=position();
        rv.symCounter=symCounter;
        return rv;
    }

    int currLine(){return lineCounter;}
    int currColumn(){return symCounter;}

    //функционал извлечения символов и статистики
    int position() const
    {
        return currPos+offset;
    }

    bool atEnd()
    {
        scipNotNewLine();
        if(data.size()<=currPos) return true;
        return false;
    }

    AString getElement(int pos, int size)
    {
        AString rv;
        rv.reserve(size);
        for(int i=0;i<size;i++)
        {
            int posit=pos-offset+i;
            posit=scipNotNewLine_helper(posit);
            i=posit-(pos-offset);
            if(i>=size)break;

            if(!data[pos-offset+i])
            {
                rv.append(' ');
                continue;
            }
            rv.append(data[pos-offset+i]);
        }
        return rv;
    }

    char getSym(int ind)
    {
        scipNotNewLine();

        //пропуск отмененных переносов строк
        int posit=currPos;
        while(ind)
        {
            posit++;ind--;
            if(data.size()<=posit)return 0;

            //пропускаем отмененный перевод строки
            posit=scipNotNewLine_helper(posit);
        }

        if(data.size()<=posit)return 0;
        if(data[posit]=='\r')return '\n';
        return data[posit];
    }

    void popSym(int cnt)
    {
        while(cnt--){popSym();}
    }

    char popSym()
    {
        scipNotNewLine();
        if(data.size()<=currPos)return 0;
        char rv=data[currPos];
        if(rv=='\r' || rv=='\n')
        {
            if(data.size()>currPos+1 && (data[currPos+1]=='\r' || data[currPos+1]=='\n'))
            {
                if(rv!=data[currPos+1])currPos++;
            }
            lineCounter++;
            symCounter=0;
            currPos++;
            return '\n';
        }
        currPos++;
        symCounter++;
        return rv;
    }

    AString fileName() const
    {
        return path;
    }

    //указатель на текущий лексический стек
    ATArray<ALexElement> *currLexStack;
    ATArray<ATArray<ALexElement>*> ifBlockStack;

protected:

    virtual int scipNotNewLine_helper(int posit)
    {
        return posit;
    }

    void scipNotNewLine()
    {
        //пропускаем отмененный перевод строки
        currPos=scipNotNewLine_helper(currPos);
    }

    AString path;
    AData data;
    int currPos;

    //счетчики положения
    int lineCounter;
    int symCounter;

    //для механизмов подмены
    int offset;

};

class ALexBase
{
public:
    ALexBase();
    virtual ~ALexBase();

    void clear();

    //код ошибки говорит о количестве ошибок
    virtual ARetCode parce(const AString &fname);

    enum LexType
    {
        LEX_UNDEF,
        LEX_SEP,
        LEX_COMM,
        LEX_OPER,
        LEX_STR,
        LEX_CHAR,
        LEX_ID,
        LEX_CONST,
        LEX_USER_BASE
    };

    enum ErrorCodes
    {
        ERR_NO,
        ERR_FILE_OPEN,
        ERR_INCOMPLETE_STRING,
        ERR_INCOMPLETE_CHAR,
        ERR_UNDEFINED_LEX,
        ERR_USER_BASE
    };

    //отладочный вывод результатов парсинга
    void debugPrint(const AString &path=AString());

    //вернет описание ошибки
    ALexErrorInfo errDescriptor(int index);

    //настройки
    void pushEnviroument(AString id){env.insert(id);}

protected:

    static int countLinePos(AData data, int offset, int *sympos=NULL);

    //лексические таблицы
    int lexTable[256];
    void buildDefLexTable();
    virtual void parceCurrent();

    void makeError(int code);

    //работа с инклудами
    AString findIncludeFilePath_Local(const AString &fname);
    AString findIncludeFilePath_InEVar(const AString &fname, const AString &evar);
    AString findIncludeFilePath(const AString &fname, bool local);

    ATSet<AString> env;

    //кэш данных из задействованных файлов
    ATHash< AString, AData > cash;
    ATHash< AString, ATArray<ALexElement>* > lexDatum;
    ATArray<ALexFileNode*> fileStack;

    //здесь храним найденные ошибки и предупреждения
    ATArray<ALexErrorInfo> errStack;
    ATArray<ALexErrorInfo> warningStack;

    virtual void scipID(int ind);
    virtual void scipLine(int ind);
    virtual bool scipConst(int ind, char fsym);

};



#endif // ALEXBASE_H


