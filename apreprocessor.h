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

#ifndef APREPROCESSOR_H
#define APREPROCESSOR_H

#include <QtCore>
#include "alex_base.h"

class ALexFileNodePreprocessor: public ALexFileNode
{
public:

    //кострукторы, деструкторы и операторы
    ALexFileNodePreprocessor(const AString &fname, const AData &src, int off=0)
        : ALexFileNode(fname,src,off)
    {        
    }
    ALexFileNodePreprocessor()
        : ALexFileNode()
    {        
    }
    ALexFileNodePreprocessor(const ALexFileNodePreprocessor &val)
        : ALexFileNode(val)
    {        
    }
    ~ALexFileNodePreprocessor(){}

private:

    int scipNotNewLine_helper(int posit)
    {
        while(data.size()>posit && data[posit]=='\\')
        {
            if(data.size()>posit+1 &&
                    (data[posit+1]=='\r' || data[posit+1]=='\n'))
            {
                if( data.size()>posit+2 &&
                    (data[posit+2]=='\r' || data[posit+2]=='\n') &&
                    data[posit+1]!=data[posit+2] )
                {
                    posit+=3;
                }
                else
                {
                    posit+=2;
                }
                continue;
            }
            break;
        }
        return posit;
    }

};

///////////////////////////////////////////////////////////////////////////////
/// \brief The ALexPreprocessor class
/// Препроцессор + предварительный лексический разбор
///

class ALexPreprocessor: public ALexBase
{
public:
    ALexPreprocessor();
    ~ALexPreprocessor();

    //код ошибки говорит о количестве ошибок
    ARetCode parce(const AString &fname);

    //для перевода приложений
    ATSet<AString> findStringsForTranslation();

    enum LexTypePrep
    {
        LEX_PREP=LEX_USER_BASE,
        LEX_MACRO,
        LEX_LINE,
        LEX_INCLUDE,
        LEX_IFBLOCK,
        LEX_UNMACRO,
        LEX_ERROR,
        LEX_PRAGMA,
        LEX_IMPORT,
        LEX_USING,
        LEX_TOKEN_SLASH,
        LEX_TOKEN_STRING,
        LEX_TOKEN_CHAR,
        LEX_TOKEN_UNION
    };

    enum ErrorCodesPrep
    {
        ERR_PREPROCESSOR_DIRECTIVE=ERR_USER_BASE
    };

    enum PrepDirectiveType
    {
        PREP_CLEAR,
        PREP_INCLUDE,
        PREP_DEFINE,
        PREP_IF,
        PREP_IFDEF,
        PREP_IFNDEF,
        PREP_ELSE,
        PREP_ELIF,
        PREP_ENDIF,
        PREP_UNDEF,
        PREP_LINE,
        PREP_ERROR,
        PREP_PRAGMA,
        PREP_IMPORT,
        PREP_USING
    };

protected:

    //лексические таблицы
    void buildLexTable();

    ATHash<AString,int> prepDict;
    void buildPrepDict();

    void parceCurrent();

    void prepDirective(ALexElement &el);
    void prepParceInclude(ALexElement &el);
    void prepParceLine(ALexElement &el);
    void prepParceIf(ALexElement &el, int type);
    void prepParceElse(ALexElement &el);
    void prepParceElif(ALexElement &el);
    void prepParceEndif(ALexElement &el);
    void prepParceDefine(ALexElement &el);
    void prepParceUndef(ALexElement &el);
    void prepParceError(ALexElement &el);
    void prepParcePragma(ALexElement &el);
    void prepParceImport(ALexElement &el);
    void prepParceUsing(ALexElement &el);

    bool prepScipToNextElement();

    bool scipConst(int ind, char fsym);
    bool scipComment(int ind);    

};

#endif // APREPROCESSOR_H
