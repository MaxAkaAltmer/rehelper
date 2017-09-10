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

#ifndef NETLISTREADER_H
#define NETLISTREADER_H

#include "netlist_reader_def.h"

class NetListReader: public ALexBase
{
public:
    NetListReader();
    ~NetListReader();

    enum LexTypePrep
    {
        LEX_MACRO=LEX_USER_BASE,
        LEX_IFBLOCK
    };

    enum ErrorCodesPrep
    {
        ERR_PREPROCESSOR_DIRECTIVE=ERR_USER_BASE
    };

    enum PrepDirectiveType
    {
        PREP_IF,
        PREP_ENDIF,
        PREP_GRIDCOUNT,
        PREP_GATECOUNT,
        PREP_MEGACOUNT,
        PREP_TYPEWIDE,
        PREP_PRIMITIVE
    };

    void syntaxParce();
    void addDefine(AString val){defines.insert(val);}
    void excludeImport(AString val){exclude_imports.insert(val.toLower());}

    void resaveSyntax();
    void makePrimitive(AString name);
    void compileElement(AString name);

private:

    AString printInlinedDef(NetListFileDef *curr, NetListFileCall *call, AString pref, ATSet<AString> &globalNames);

    void makeRegisters();
    ATArray<ATArray<NetListFileArgument> > getCallOrdered(NetListFileCall *call);

    void callReductor(NetListFileCall *call);
    NetListFileDef *searchDef(AString name);
    AString printUnitedDef(NetListFileDef *curr, NetListFileCall *call, int level);
    void optimizeUnitedDef(NetListFileDef *curr, NetListFileCall *call, int level);

    ATSet<AString> primitives;
    void syntaxVerify();
    void makeConstant(NetListFileDef *def, int callInd, int val);
    NetListFile* netlistByName(AString name);
    NetListFileDef* findDefForCall(AString call_name);
    bool verifyCall(NetListFileDef *caller, NetListFileCall *call, NetListFileDef *def, int pass);

    bool isOper(ATArray<ALexElement> &lex, int ind, char op);
    bool isID(ATArray<ALexElement> &lex, int ind);
    bool isConst(ATArray<ALexElement> &lex, int ind);
    AString stringLexema(ALexElement &lex);

    bool syntaxMakeImport(NetListFile &out, ATArray<ALexElement> &lex, int &j);
    bool syntaxMakeDef(NetListFile &out, ATArray<ALexElement> &lex, int &j);
    bool syntaxScipModule(ATArray<ALexElement> &lex, int &j);

    bool syntaxNetArray(NetListFileArgument &arg, ATArray<ALexElement> &lex, int &j, AString errmess);

    void makeSyntaError(AString reson, ALexElement &lex);

    //кэш данных из задействованных файлов
    ATSet<AString> exclude_imports;
    ATSet<AString> defines;
    ATHash< AString, NetListFile* > syntax;

    void buildLexTable();
    void parceCurrent();

    ATHash<AString,int> prepDict;
    void buildPrepDict();

    bool scipComment(int ind);

    void prepDirective(ALexElement &el);
    void prepParceIf(ALexElement &el);
    void prepParceEndif(ALexElement &el);
    void prepParceMacro(ALexElement &el);

    AString printPrim(NetListFileCall *call, AString mono);

};

#endif // NETLISTREADER_H
