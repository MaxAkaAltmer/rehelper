#include "apreprocessor.h"
#include <ctype.h>

ALexPreprocessor::ALexPreprocessor()
    : ALexBase()
{    
    buildLexTable();
    buildPrepDict();
}

ALexPreprocessor::~ALexPreprocessor()
{
}

ATSet<AString> ALexPreprocessor::findStringsForTranslation()
{
    ATSet<AString> rv;
    for(int i=0;i<cash.size();i++)
    {
        AData buffer=cash.value(i);
        ATArray<ALexElement> *dat=lexDatum[cash.key(i)];
        for(int j=0;j<dat->size();j++)
        {
            if((*dat)[j].type!=LEX_ID)continue;

            int off=(*dat)[j].offset;
            int size=(*dat)[j].size;

            AString id=AString::fromFix((char*)(&(buffer()[off])),size);

            bool enter_find=false;
            int old_j=j;
            if(id=="at")
            {
                AString str;
                for(j++;j<dat->size();j++)
                {
                    int off=(*dat)[j].offset;
                    int size=(*dat)[j].size;

                    if((*dat)[j].type==LEX_OPER && buffer()[off]=='(')
                    {
                        enter_find=true;
                        continue;
                    }
                    if(!enter_find)continue;

                    if((*dat)[j].type==LEX_OPER)
                    {
                        if(buffer()[off]==')')break;
                        enter_find=false;
                        break;
                    }
                    else if((*dat)[j].type==LEX_STR)
                    {
                        if(size<=2)continue;
                        str+=AString::fromFix((char*)(&(buffer()[off+1])),size-2);
                    }
                    else if((*dat)[j].type!=LEX_SEP && (*dat)[j].type!=LEX_UNDEF)
                    {
                        enter_find=false;
                        break;
                    }
                }

                if(!enter_find)j=old_j;
                else if(!str.isEmpty())rv.insert(str);
            }
        }
    }
    return rv;
}

void ALexPreprocessor::buildLexTable()
{
    for(int i=0;i<256;i++)lexTable[i]=LEX_UNDEF;

    lexTable[' ']=LEX_SEP;
    lexTable['\t']=LEX_SEP;
    lexTable['\r']=LEX_SEP;
    lexTable['\n']=LEX_SEP;
    lexTable['\v']=LEX_SEP;
    lexTable['\f']=LEX_SEP;

    lexTable['/']=LEX_COMM;

    lexTable['#']=LEX_PREP;

    const char ops[]="{},|[]+-%;:?<>=!&~^*\\()";
    for(uintx i=0;i<sizeof(ops)/sizeof(ops[0]);i++)lexTable[(uint8)ops[i]]=LEX_OPER;

    lexTable['.']=LEX_CONST;
    for(int v='0';v<='9';v++)lexTable[v]=LEX_CONST;

    lexTable['_']=LEX_ID;
    for(int v='A';v<='Z';v++)lexTable[v]=LEX_ID;
    for(int v='a';v<='z';v++)lexTable[v]=LEX_ID;

    lexTable['\"']=LEX_STR;

    lexTable['\'']=LEX_CHAR;
}

void ALexPreprocessor::buildPrepDict()
{
    //prepDict.insert("include",PREP_INCLUDE);
    prepDict.insert("define",PREP_DEFINE);
//    prepDict.insert("if",PREP_IF);
//    prepDict.insert("ifdef",PREP_IFDEF);
//    prepDict.insert("ifndef",PREP_IFNDEF);
//    prepDict.insert("else",PREP_ELSE);
//    prepDict.insert("elif",PREP_ELSE);
//    prepDict.insert("endif",PREP_ENDIF);
    prepDict.insert("undef",PREP_UNDEF);
    prepDict.insert("line",PREP_LINE);
    prepDict.insert("error",PREP_ERROR);
    prepDict.insert("pragma",PREP_PRAGMA);
    prepDict.insert("import",PREP_IMPORT);
    prepDict.insert("using",PREP_USING);
}

ARetCode ALexPreprocessor::parce(const AString &fname)
{
    AFile hand(fname);

    //чистим предыдущие результаты
    clear();

    //пытаемся загрузить и обработать файл
    if(hand.open(AFile::OReadOnly))
    {
        int64 size=hand.size();

        //сформируем точку разбора
        AData temp=hand.read(size);
        fileStack.append(new ALexFileNodePreprocessor(fname,temp,0));

        //подготовимся к рекурсии
        cash[fname]=temp;
        fileStack.last()->currLexStack = new ATArray<ALexElement>;
        lexDatum[fname]=fileStack.last()->currLexStack;
        parceCurrent();

        //почистим за собой
        delete fileStack.last();
        fileStack.pop();
        hand.close();
        return -errStack.size();
    }

    //формируем ошибку открытия файла
    makeError(ERR_FILE_OPEN);
    return -errStack.size();
}

bool ALexPreprocessor::scipConst(int ind, char fsym)
{
    if(!fsym && fileStack[ind]->getSym(0)=='.')
    {
        if(fileStack[ind]->getSym(1)=='.'
                || lexTable[(uint8)fileStack[ind]->getSym(1)]!=LEX_CONST)
            return false;
    }

    if(fsym=='.')
    {
        if(fileStack[ind]->getSym(0)=='.'
                || lexTable[(uint8)fileStack[ind]->getSym(0)]!=LEX_CONST)
            return false;
    }

    while(!fileStack[ind]->atEnd())
    {
        char c=fileStack[ind]->getSym(0);
        if(lexTable[(uint8)c]!=LEX_CONST && lexTable[(uint8)c]!=LEX_ID)break;
        fileStack[ind]->popSym();
    }

    return true;
}

bool ALexPreprocessor::scipComment(int ind)
{
    if(fileStack[ind]->getSym(0)=='/')
    {//строчный комментарий
        fileStack[ind]->popSym();
        while(!fileStack[ind]->atEnd())
        {
            char c=fileStack[ind]->getSym(0);
            if(c=='\n')break;
            fileStack[ind]->popSym();
        }
        return true;
    }
    else if(fileStack[ind]->getSym(0)=='*')
    {//блочный комментарий
        fileStack[ind]->popSym();
        while(!fileStack[ind]->atEnd())
        {
            if(fileStack[ind]->getSym(0)=='*' &&
                    fileStack[ind]->getSym(1)=='/')
            {
                fileStack[ind]->popSym();
                fileStack[ind]->popSym();
                break;
            }
            fileStack[ind]->popSym();
        }
        return true;
    }
    return false;
}

bool ALexPreprocessor::prepScipToNextElement()
{
    int ind=fileStack.size()-1;

    while(!fileStack[ind]->atEnd())
    {
        char csym=fileStack[ind]->getSym(0);
        if(csym=='\n')
        {
            return false;
        }
        else if(csym=='/')//проверим комменты
        {
            fileStack[ind]->popSym();
            if(!scipComment(ind))
            {
                return false;
            }
        }
        else if(lexTable[(uint8)csym]!=LEX_SEP)
        {
            return true;
        }
        fileStack[ind]->popSym();
    }
    return false;
}

void ALexPreprocessor::prepParceInclude(ALexElement &el)
{
    int ind=fileStack.size()-1;    
    el.type=LEX_INCLUDE;

    if(!prepScipToNextElement())
    {
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        scipLine(ind);
        return;
    }

    AString iname;

    if(fileStack[ind]->getSym(0)=='\"')
    {
        fileStack[ind]->popSym();
        while(!fileStack[ind]->atEnd())
        {
            char c=fileStack[ind]->getSym(0);
            if(c=='\n')break;
            if(c=='\"')
            {
                fileStack[ind]->popSym();
                break;
            }
            if(!c)
            {
                fileStack[ind]->popSym();
                continue;
            }
            iname.append(c);
            fileStack[ind]->popSym();
        }
        iname=findIncludeFilePath(iname,true);
    }
    else if(fileStack[ind]->getSym(0)=='<')
    {
        fileStack[ind]->popSym();
        while(!fileStack[ind]->atEnd())
        {
            char c=fileStack[ind]->getSym(0);
            if(c=='\n')break;
            if(c=='>')
            {
                fileStack[ind]->popSym();
                break;
            }
            if(!c)
            {
                fileStack[ind]->popSym();
                continue;
            }
            iname.append(c);
            fileStack[ind]->popSym();
        }
        iname=findIncludeFilePath(iname,false);
    }
    else
    {
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        scipLine(ind);
        return;
    }    
    el.extra.append(iname);
    scipLine(ind);

    //пытаемся открыть файл
    if(cash.contains(iname))
    {
        return;
    }
    else
    {
        AFile hand(iname);

        //пытаемся загрузить и обработать файл
        if(hand.open(AFile::OReadOnly))
        {
            int64 size=hand.size();

            //сформируем точку разбора
            AData temp=hand.read(size);
            hand.close();
            fileStack.append(new ALexFileNodePreprocessor(iname,temp,0));

            //подготовимся к рекурсии
            cash[iname]=temp;
            fileStack.last()->currLexStack = new ATArray<ALexElement>;
            lexDatum[iname]=fileStack.last()->currLexStack;
            parceCurrent();

            //приберем за собой
            delete fileStack.last();
            fileStack.pop();

            return;
        }

        //формируем ошибку открытия файла
        makeError(ERR_FILE_OPEN);
        return;
    }

}

void ALexPreprocessor::prepParceLine(ALexElement &el)
{
    int ind=fileStack.size()-1;
    el.type=LEX_LINE;

    if(!prepScipToNextElement())
    {
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        scipLine(ind);
        return;
    }

    int start = fileStack[ind]->position();
    if(!scipConst(ind,0))
    {
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        scipLine(ind);
        return;
    }
    AString line_num=fileStack[ind]->getElement(start,fileStack[ind]->position()-start);

    AString line_name;
    if(prepScipToNextElement())
    {
        if(fileStack[ind]->getSym(0)=='\"')
        {
            fileStack[ind]->popSym();
            while(!fileStack[ind]->atEnd())
            {
                char c=fileStack[ind]->getSym(0);
                if(c=='\n')break;
                if(c=='\"')
                {
                    fileStack[ind]->popSym();
                    break;
                }
                if(!c)
                {
                    fileStack[ind]->popSym();
                    continue;
                }
                line_name.append(c);
                fileStack[ind]->popSym();
            }
        }
    }
    scipLine(ind);    
    el.extra.append(AVariant(line_num.toInt<int>()));
    el.extra.append(AVariant(fileStack[ind]->position()+1));
    if(!line_name.isEmpty())el.extra.append(line_name);
}

void ALexPreprocessor::prepParceIf(ALexElement &el, int type)
{
    int ind=fileStack.size()-1;
    int start = fileStack[ind]->position();
    scipLine(ind);

    el.size=fileStack[ind]->position()-el.offset;
    el.type=LEX_IFBLOCK;

    ALexElement bel;
    bel.type=LEX_PREP;
    bel.file_index=el.file_index;
    bel.offset=el.offset;
    bel.columne=el.columne;
    bel.line=el.line;
    bel.size=fileStack[ind]->position()-el.offset;
    bel.extra.append(AVariant(type));
    bel.extra.append(AVariant(start));
    bel.extra.append(AVariant(fileStack[ind]->position()-start));
    //запишем позицию условного выражения
    el.extra.append(AVariant(type));
    el.extra.append(AVariant(start));
    el.extra.append(AVariant(fileStack[ind]->position()-start));

    //готовимся к рекурсии
    el.macroLexStack.append(bel);
    fileStack[ind]->currLexStack->append(el);

    //организуем следующий узел
    fileStack[ind]->ifBlockStack.append(fileStack[ind]->currLexStack);
    fileStack[ind]->currLexStack=&fileStack[ind]->currLexStack->last().macroLexStack.last().macroLexStack;

}

void ALexPreprocessor::prepParceElse(ALexElement &el)
{
    int ind=fileStack.size()-1;
    scipLine(ind);
    el.size=fileStack[ind]->position()-el.offset;

    el.extra.append(AVariant(PREP_ELSE));

    if(!fileStack[ind]->ifBlockStack.size())
    {
        fileStack[ind]->currLexStack->append(el);
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        return;
    }

    int inf=fileStack[ind]->ifBlockStack.last()->last().extra[0].toInt();
    if(inf!=PREP_IFDEF && inf!=PREP_IF && inf!=PREP_IFNDEF && inf!=PREP_ELIF)
    {
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
    }

    //организуем следующий узел
    fileStack[ind]->ifBlockStack.last()->last().macroLexStack.append(el);
    fileStack[ind]->currLexStack=&fileStack[ind]->ifBlockStack.last()->last().macroLexStack.last().macroLexStack;

}

void ALexPreprocessor::prepParceElif(ALexElement &el)
{
    int ind=fileStack.size()-1;
    int start = fileStack[ind]->position();
    scipLine(ind);
    el.size=fileStack[ind]->position()-el.offset;

    el.extra.append(AVariant(PREP_ELIF));
    el.extra.append(AVariant(start));
    el.extra.append(AVariant(fileStack[ind]->position()-start));

    if(!fileStack[ind]->ifBlockStack.size())
    {
        fileStack[ind]->currLexStack->append(el);
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        return;
    }

    int inf=fileStack[ind]->ifBlockStack.last()->last().extra[0].toInt();
    if(inf!=PREP_IFDEF && inf!=PREP_IF && inf!=PREP_IFNDEF && inf!=PREP_ELIF)
    {
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
    }

    //организуем следующий узел
    fileStack[ind]->ifBlockStack.last()->last().macroLexStack.append(el);
    fileStack[ind]->currLexStack=&fileStack[ind]->ifBlockStack.last()->last().macroLexStack.last().macroLexStack;

}

void ALexPreprocessor::prepParceEndif(ALexElement &el)
{
    int ind=fileStack.size()-1;
    scipLine(ind);
    el.size=fileStack[ind]->position()-el.offset;
    el.extra.append(AVariant(PREP_ENDIF));

    if(!fileStack[ind]->ifBlockStack.size())
    {
        fileStack[ind]->currLexStack->append(el);
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        return;
    }

    //организуем следующий узел
    fileStack[ind]->ifBlockStack.last()->last().macroLexStack.append(el);
    fileStack[ind]->currLexStack=fileStack[ind]->ifBlockStack.pop();
    fileStack[ind]->currLexStack->last().size=fileStack[ind]->position()-fileStack[ind]->currLexStack->last().offset;

}

void ALexPreprocessor::prepParceDefine(ALexElement &el)
{
    int ind=fileStack.size()-1;
    el.type=LEX_MACRO;

    if(!prepScipToNextElement())
    {
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        scipLine(ind);
        return;
    }

    int start = fileStack[ind]->position();

    if(lexTable[(uint8)fileStack[ind]->getSym(0)]!=LEX_ID)
    {
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        scipLine(ind);
        return;
    }
    scipID(ind);
    el.extra.append(fileStack[ind]->getElement(start,fileStack[ind]->position()-start));

    if(fileStack[ind]->getSym(0)=='(')
    { //извлекаем параметры
        fileStack[ind]->popSym();
        bool just_sep=true;
        bool terminated=false;
        while(!fileStack[ind]->atEnd())
        {
            if(!prepScipToNextElement())
            {
                makeError(ERR_PREPROCESSOR_DIRECTIVE);
                scipLine(ind);
                return;
            }

            if(fileStack[ind]->getSym(0)==')')
            {
                if(just_sep)
                {
                    makeError(ERR_PREPROCESSOR_DIRECTIVE);
                    scipLine(ind);
                    return;
                }
                fileStack[ind]->popSym();
                break;
            }
            else if(fileStack[ind]->getSym(0)==',' && !terminated)
            {
                if(just_sep)
                {
                    makeError(ERR_PREPROCESSOR_DIRECTIVE);
                    scipLine(ind);
                    return;
                }
                fileStack[ind]->popSym();
                just_sep=true;
                continue;
            }
            else if(fileStack[ind]->getSym(0)=='.'
                    && fileStack[ind]->getSym(1)=='.'
                    && fileStack[ind]->getSym(2)=='.' && !terminated)
            {
                terminated=true;
                el.extra.append("...");
                fileStack[ind]->popSym(3);
                continue;
            }

            if(lexTable[(uint8)fileStack[ind]->getSym(0)]!=LEX_ID || !just_sep || terminated)
            {
                makeError(ERR_PREPROCESSOR_DIRECTIVE);
                scipLine(ind);
                return;
            }

            just_sep=false;
            start=fileStack[ind]->position();
            scipID(ind);
            el.extra.append(fileStack[ind]->getElement(start,fileStack[ind]->position()-start));
        }
    }

    //парсим макро-подстановку
    while(!fileStack[ind]->atEnd())
    {
        char sym=fileStack[ind]->getSym(0);
        if(sym=='\n')break;

        ALexElement tel;
        tel.file_index=ind;
        tel.offset=fileStack[ind]->position();
        tel.line=fileStack[ind]->currLine();
        tel.columne=fileStack[ind]->currColumn();

        if(sym=='\\')
        {
            tel.type=LEX_TOKEN_SLASH;
            fileStack[ind]->popSym();
        }
        else if(sym=='#')
        {
            if(fileStack[ind]->getSym(1)=='#')
            {
                tel.type=LEX_TOKEN_UNION;
                fileStack[ind]->popSym(2);
            }
            if(fileStack[ind]->getSym(1)=='@')
            {
                tel.type=LEX_TOKEN_CHAR;
                fileStack[ind]->popSym(2);
            }
            else
            {
                tel.type=LEX_TOKEN_STRING;
                fileStack[ind]->popSym();
            }
        }
        else
        {
            tel.type=lexTable[(uint8)sym];
            fileStack[ind]->popSym();
            switch(tel.type)
            {
            case LEX_UNDEF: //вычленяем разделители
                while(!fileStack[ind]->atEnd()
                      && lexTable[(uint8)fileStack[ind]->getSym(0)]==LEX_UNDEF)
                {
                    fileStack[ind]->popSym();
                }
                break;
            case LEX_SEP: //вычленяем разделители
                while(!fileStack[ind]->atEnd()
                      && lexTable[(uint8)fileStack[ind]->getSym(0)]==LEX_SEP)
                {
                    if(fileStack[ind]->getSym(0)=='\n')break;
                    fileStack[ind]->popSym();
                }
                break;
            case LEX_COMM: //вычленяем комментарии
                if(!scipComment(ind))
                {
                    tel.type=LEX_OPER;
                }
                break;
            case LEX_OPER:
                break;
            case LEX_STR:
                while(!fileStack[ind]->atEnd())
                {
                    char c=fileStack[ind]->getSym(0);
                    if(c=='\n' || c=='\"')
                    {
                        break;
                    }
                    fileStack[ind]->popSym();
                }
                if(!fileStack[ind]->atEnd() &&
                        fileStack[ind]->getSym(0)=='\"')
                {
                    fileStack[ind]->popSym();
                }
                else
                {
                    //генерим ошибку
                    makeError(ERR_INCOMPLETE_STRING);
                }
                break;
            case LEX_CHAR:
                while(!fileStack[ind]->atEnd())
                {
                    char c=fileStack[ind]->getSym(0);
                    if(c=='\n' || c=='\'')
                    {
                        break;
                    }
                    fileStack[ind]->popSym();
                }
                if(!fileStack[ind]->atEnd() &&
                        fileStack[ind]->getSym(0)=='\'')
                {
                    fileStack[ind]->popSym();
                }
                else
                {
                    //генерим ошибку
                    makeError(ERR_INCOMPLETE_CHAR);
                }
                break;
            case LEX_ID:
                scipID(ind);
                break;
            case LEX_CONST:
                if(!scipConst(ind, sym))
                {
                    tel.type=LEX_OPER;
                }
                break;
            };
        }

        tel.size=fileStack[ind]->position()-tel.offset;
        el.macroLexStack.append(tel);
    }

}

void ALexPreprocessor::prepParceUndef(ALexElement &el)
{
    int ind=fileStack.size()-1;    
    if(!prepScipToNextElement())
    {
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        scipLine(ind);
        return;
    }

    if(lexTable[(uint8)fileStack[ind]->getSym(0)]!=LEX_ID)
    {
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        scipLine(ind);
        return;
    }

    int start = fileStack[ind]->position();
    scipID(ind);

    el.type=LEX_UNMACRO;
    el.extra.append(AVariant(start));
    el.extra.append(AVariant(fileStack[ind]->position()-start));

    scipLine(ind);
}

void ALexPreprocessor::prepParceError(ALexElement &el)
{
    int ind=fileStack.size()-1;
    int start = fileStack[ind]->position();
    scipLine(ind);
    el.type=LEX_ERROR;
    el.extra.append(AVariant(start));
    el.extra.append(AVariant(fileStack[ind]->position()-start));
}

void ALexPreprocessor::prepParcePragma(ALexElement &el)
{
    int ind=fileStack.size()-1;
    int start = fileStack[ind]->position();
    scipLine(ind);
    el.type=LEX_PRAGMA;
    el.extra.append(AVariant(start));
    el.extra.append(AVariant(fileStack[ind]->position()-start));
}

void ALexPreprocessor::prepParceImport(ALexElement &el)
{
    int ind=fileStack.size()-1;
    int start = fileStack[ind]->position();
    scipLine(ind);
    el.type=LEX_IMPORT;
    el.extra.append(AVariant(start));
    el.extra.append(AVariant(fileStack[ind]->position()-start));
}

void ALexPreprocessor::prepParceUsing(ALexElement &el)
{
    int ind=fileStack.size()-1;
    int start = fileStack[ind]->position();
    scipLine(ind);
    el.type=LEX_USING;
    el.extra.append(AVariant(start));
    el.extra.append(AVariant(fileStack[ind]->position()-start));
}

void ALexPreprocessor::prepDirective(ALexElement &el)
{
    int ind=fileStack.size()-1;
    //ищем команду препроцессора
    bool idOk=false;
    while(!fileStack[ind]->atEnd())
    {
        char csym=fileStack[ind]->getSym(0);
        if(lexTable[(uint8)csym]==LEX_ID)
        {
            idOk=true;
            break;
        }
        else if(csym=='\n')
        {
            break;
        }
        else if(csym=='/')//проверим комменты
        {
            fileStack[ind]->popSym();
            if(!scipComment(ind))
            {
                //фиксируем ошибку
                makeError(ERR_PREPROCESSOR_DIRECTIVE);
                break;
            }
        }
        else if(lexTable[(uint8)csym]!=LEX_SEP)
        {
            //фиксируем ошибку            
            makeError(ERR_PREPROCESSOR_DIRECTIVE);
            break;
        }
        fileStack[ind]->popSym();
    }
    if(!idOk)
    {
        scipLine(ind);
        el.size=fileStack[ind]->position()-el.offset;
        fileStack[ind]->currLexStack->append(el);
        return;
    }

    int nameStart=fileStack[ind]->position();
    scipID(ind);
    AString dname=fileStack[ind]->getElement(nameStart,fileStack[ind]->position()-nameStart);

    if(prepDict.contains(dname))
    {
        switch(prepDict[dname])
        {
        case PREP_INCLUDE:
            prepParceInclude(el);
            break;
        case PREP_LINE:
            prepParceLine(el);
            break;

        case PREP_IFDEF:
        case PREP_IFNDEF:
        case PREP_IF:
            prepParceIf(el,prepDict[dname]);
            return;
        case PREP_ELSE:
            prepParceElse(el);
            return;
        case PREP_ELIF:
            prepParceElif(el);
            return;
        case PREP_ENDIF:
            prepParceEndif(el);
            return;

        case PREP_DEFINE:
            prepParceDefine(el);
            break;
        case PREP_UNDEF:
            prepParceUndef(el);
            break;
        case PREP_ERROR:
            prepParceError(el);
            break;
        case PREP_PRAGMA:
            prepParcePragma(el);
            break;

        case PREP_IMPORT:
            prepParceImport(el);
            break;
        case PREP_USING:
            prepParceUsing(el);
            break;

        default:
            //фиксируем ошибку            
            makeError(ERR_PREPROCESSOR_DIRECTIVE);
            scipLine(ind);
            break;
        };
    }
    else
    {
        //фиксируем ошибку        
        makeError(ERR_PREPROCESSOR_DIRECTIVE);
        scipLine(ind);
    }

    el.size=fileStack[ind]->position()-el.offset;
    fileStack[ind]->currLexStack->append(el);

}

void ALexPreprocessor::parceCurrent()
{
    int ind=fileStack.size()-1;
    int findex=cash.indexOf(fileStack[ind]->fileName());

    while(!fileStack[ind]->atEnd())
    {
        ALexElement el;
        el.file_index=findex;
        el.offset=fileStack[ind]->position();
        el.line=fileStack[ind]->currLine();
        el.columne=fileStack[ind]->currColumn();
        char csym=fileStack[ind]->popSym();
        el.type=lexTable[(uint8)csym];

        switch(el.type)
        {
        case LEX_UNDEF: //вычленяем разделители
            while(!fileStack[ind]->atEnd()
                  && lexTable[(uint8)fileStack[ind]->getSym(0)]==LEX_UNDEF)
            {
                fileStack[ind]->popSym();
            }
            el.size=fileStack[ind]->position()-el.offset;
            break;
        case LEX_SEP: //вычленяем разделители
            while(!fileStack[ind]->atEnd()
                  && lexTable[(uint8)fileStack[ind]->getSym(0)]==LEX_SEP)
            {
                fileStack[ind]->popSym();
            }
            el.size=fileStack[ind]->position()-el.offset;
            break;
        case LEX_COMM: //вычленяем комментарии
            if(!scipComment(ind))
            {                
                el.type=LEX_OPER;
            }            
            el.size=fileStack[ind]->position()-el.offset;
            break;
        case LEX_PREP:
            prepDirective(el);
            continue;
        case LEX_OPER:
            if(csym=='\\')el.type=LEX_TOKEN_SLASH;
            el.size=fileStack[ind]->position()-el.offset;
            break;
        case LEX_STR:
            while(!fileStack[ind]->atEnd())
            {
                char c=fileStack[ind]->getSym(0);
                if(c=='\n' || c=='\"')
                {
                    break;
                }
                fileStack[ind]->popSym();
            }
            if(!fileStack[ind]->atEnd() &&
                    fileStack[ind]->getSym(0)=='\"')
            {
                fileStack[ind]->popSym();
                el.size=fileStack[ind]->position()-el.offset;
            }
            else
            {
                el.size=fileStack[ind]->position()-el.offset;
                //генерим ошибку
                makeError(ERR_INCOMPLETE_STRING);
            }
            break;
        case LEX_CHAR:
            while(!fileStack[ind]->atEnd())
            {                
                char c=fileStack[ind]->getSym(0);
                if(c=='\n' || c=='\'')
                {
                    break;
                }
                fileStack[ind]->popSym();
            }
            if(!fileStack[ind]->atEnd() &&
                    fileStack[ind]->getSym(0)=='\'')
            {
                fileStack[ind]->popSym();
                el.size=fileStack[ind]->position()-el.offset;
            }
            else
            {
                el.size=fileStack[ind]->position()-el.offset;
                //генерим ошибку
                makeError(ERR_INCOMPLETE_CHAR);
            }
            break;
        case LEX_ID:
            scipID(ind);
            el.size=fileStack[ind]->position()-el.offset;
            break;
        case LEX_CONST:
            if(!scipConst(ind, csym))
            {
                el.type=LEX_OPER;
            }
            el.size=fileStack[ind]->position()-el.offset;
            break;
        };
        fileStack[ind]->currLexStack->append(el);
    }
}
