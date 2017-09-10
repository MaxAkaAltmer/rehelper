#include "netlist_reader.h"

NetListReader::NetListReader()
    : ALexBase()
{
    buildLexTable();
    buildPrepDict();
}

NetListReader::~NetListReader()
{
    for(int i=0;i<syntax.size();i++)
    {
        delete syntax.value(i);
    }
}

void NetListReader::makeSyntaError(AString reson, ALexElement &lex)
{
    int xoff=lex.columne;
    int yoff=lex.line;
    qDebug() << "Syntax error:" << reson << cash.key(lex.file_index)
             << yoff << xoff << stringLexema(lex);
}

void NetListReader::resaveSyntax()
{
    for(int i=0;i<syntax.size();i++)
    {
        QFile hand;
        AStringPathParcer fstr(syntax.key(i));
        hand.setFileName((syntax.key(i)+".renet")());

        hand.open(QFile::Truncate|QFile::WriteOnly);

        AString str=syntax.value(i)->print();
        hand.write(str());

        hand.close();
    }
}

AString NetListReader::stringLexema(ALexElement &lex)
{
    int off=lex.offset;
    int size=lex.size;
    return AString::fromFix((char*)(&(cash.value(lex.file_index)()[off])),size);
}

bool NetListReader::isOper(ATArray<ALexElement> &lex, int ind, char op)
{
    if(ind>=lex.size())return false;
    if(lex[ind].type!=LEX_OPER)return false;
    if(cash.value(lex[ind].file_index)[lex[ind].offset]!=op)return false;
    return true;
}

bool NetListReader::isID(ATArray<ALexElement> &lex, int ind)
{
    if(ind>=lex.size())return false;
    if(lex[ind].type!=LEX_ID)return false;
    return true;
}

bool NetListReader::isConst(ATArray<ALexElement> &lex, int ind)
{
    if(ind>=lex.size())return false;
    if(lex[ind].type!=LEX_CONST)return false;
    return true;
}

bool NetListReader::syntaxMakeImport(NetListFile &out, ATArray<ALexElement> &lex, int &j)
{
    for(j++;j<lex.size();j++)
    {
        if(isID(lex,j))
        {
            out.imports.insert(stringLexema(lex[j]).toLower());
        }
        else if(isOper(lex,j,';'))
        {
            return true;
        }
        else if(isOper(lex,j,','))
        {
            continue;
        }
        else
        {
            makeSyntaError("import",lex[j]);
            return false;
        }
    }
    return false;
}

bool NetListReader::syntaxNetArray(NetListFileArgument &arg, ATArray<ALexElement> &lex, int &j, AString errmess)
{
    j++;
    if(!isConst(lex,j))
    {
        makeSyntaError(errmess,lex[j]);
        return false;
    }
    arg.first=arg.last=stringLexema(lex[j++]).toInt<int>();

    if(isOper(lex,j,'.') || isOper(lex,j,'-'))
    {
        arg.viaPoint=0;
        if((isOper(lex,j,'.') && (!isOper(lex,j+1,'.') || !isConst(lex,j+2))) ||
                (isOper(lex,j,'-') && !isConst(lex,j+1)))
        {
            makeSyntaError(errmess,lex[j]);
            return false;
        }
        else if(isOper(lex,j,'.'))
        {
            arg.viaPoint=1;
            j++;
        }
        j++;

        arg.last=stringLexema(lex[j++]).toInt<int>();
    }
    if(!isOper(lex,j,']'))
    {
        makeSyntaError(errmess,lex[j]);
        return false;
    }
    j++;

    if(arg.first>arg.last)
    {
        makeSyntaError(errmess+" first>last",lex[j]);
        return false;
    }

    return true;
}

bool NetListReader::syntaxMakeDef(NetListFile &out, ATArray<ALexElement> &lex, int &j)
{
    NetListFileDef tmp;

    j++;
    if(!isID(lex,j))
    {
        makeSyntaError("def",lex[j]);
        return false;
    }

    tmp.name=stringLexema(lex[j++]);

    if(!isOper(lex,j,'('))
    {
        makeSyntaError("def connectom",lex[j]);
        return false;
    }
    j++;

    //считываем входные цепи
    while(j<lex.size() && !isOper(lex,j,')'))
    {
        NetListFileArgument arg;

        if(isOper(lex,j,':'))
        {
            j++;
            if(!isID(lex,j))
            {
                makeSyntaError("def type",lex[j]);
                return false;
            }
            AString type=stringLexema(lex[j++]);
            if(type!="IO" && type!="BUS" && type!="IN"
                    && type!="OUT" && type!="TRI" && type!="BI")
                makeSyntaError("def type unk "+type,lex[j]);
            for(int k=tmp.args.size()-1;k>=0;k--)
            {
                if(!tmp.args.value_ref(k).type.isEmpty())break;
                tmp.args.value_ref(k).type=type;
            }
            if(isOper(lex,j,';'))j++;
            if(isOper(lex,j,')'))break;
        }

        if(!isID(lex,j))
        {
            makeSyntaError("def net",lex[j]);
            return false;
        }

        AString first=stringLexema(lex[j++]);

        //первым был модификатор типа
        if(isOper(lex,j,'/'))
        {
            j++;
            if((first.indexOf("INT") && first!="BIT") || !isID(lex,j))
            {
                makeSyntaError("def type",lex[j]);
                return false;
            }
            if(first=="BIT"){arg.bitsize=1;arg.strong=true;}
            else arg.bitsize=first.right(3).toInt<int>();
            arg.name=stringLexema(lex[j++]);
            if(lex[j-1].extra.size() && lex[j-1].extra.last().isString())
            {
                arg.comment=lex[j-1].extra.last().toString();
            }
        }
        else
        {
            arg.bitsize=1;
            arg.name=first;
            if(lex[j-1].extra.size() && lex[j-1].extra.last().isString())
            {
                arg.comment=lex[j-1].extra.last().toString();
            }
        }

        //массив шнурков
        if(isOper(lex,j,'['))
        {
            if(!syntaxNetArray(arg, lex, j, "def array"))
                return false;

            if(lex[j-1].extra.size() && lex[j-1].extra.last().isString())
            {
                arg.comment=lex[j-1].extra.last().toString();
            }
        }

        tmp.args.insertAll(arg.name,arg);
    }

    if(!tmp.args.size() || tmp.args.last().type.isEmpty())
        makeSyntaError("def incompete type",lex[j]);

    if(!isOper(lex,j,')') || !isOper(lex,j+1,';') || !isID(lex,j+2))
    {
        makeSyntaError("def connectom end",lex[j]);
        return false;
    }
    j+=2;

    AString next=stringLexema(lex[j]);
    while(next!="BEGIN" && j<lex.size())
    {
        int bitsize=-1;

        //считываем локальные цепи
        while(j<lex.size() && !isOper(lex,j,':') && !isOper(lex,j,';'))
        {
            NetListFileArgument arg;

            if(!isID(lex,j))
            {
                makeSyntaError("def local",lex[j]);
                return false;
            }

            AString first=stringLexema(lex[j]);
            if(first=="BEGIN"){next=first;break;}
            j++;

            //первым был модификатор типа
            if(isOper(lex,j,'/'))
            {
                j++;
                if(first.indexOf("INT") || !isID(lex,j))
                {
                    makeSyntaError("def type",lex[j]);
                    return false;
                }
                bitsize=arg.bitsize=first.right(3).toInt<int>();
                arg.name=stringLexema(lex[j++]);
            }
            else
            {
                if(bitsize<0){makeSyntaError("local type",lex[j]);bitsize=1;}
                arg.bitsize=bitsize;
                arg.name=first;
            }

            //массив шнурков
            if(isOper(lex,j,'['))
            {
                if(!syntaxNetArray(arg, lex, j, "def array"))
                    return false;
            }

            arg.type="LOCAL";
            tmp.locals.insertAll(arg.name,arg);
        }

        if(next=="BEGIN")break;
        if(isOper(lex,j,';') && isID(lex,j+1))
        {
            j++;
        }
        else if(!isOper(lex,j,':') || !isID(lex,j+1) || !isOper(lex,j+2,';')
                || !isID(lex,j+3))
        {
            makeSyntaError("def locals",lex[j]);
            return false;
        }
        else
        {
            j+=3;
        }
        next=stringLexema(lex[j]);
    }

    if(next!="BEGIN")
    {
        makeSyntaError("def begin",lex[j]);
        return false;
    }
    j++;

    //разбор тела
    while(j<lex.size() && isID(lex,j))
    {
        NetListFileCall call;

        next=stringLexema(lex[j++]);
        if(next=="END")break;

        NetListFileArgument label;
        label.name=next;

        if(lex[j-2].extra.size() && lex[j-2].extra.last().isString())
        {
            label.comment=lex[j-2].extra.last().toString();
        }

        //массив шнурков
        if(isOper(lex,j,'['))
        {
            if(!syntaxNetArray(label, lex, j, "label array"))
                return false;
        }

        if(!(isOper(lex,j,':') && isOper(lex,j+1,'=') && isID(lex,j+2) && isOper(lex,j+3,'(')))
        {
            makeSyntaError("def call complex",lex[j]);
            return false;
        }
        j+=2;

        label.bitsize=0;
        call.label=label;
        call.name=stringLexema(lex[j]);
        j+=2;

        //считываем цепи
        while(j<lex.size() && !isOper(lex,j,')'))
        {
            NetListFileArgument arg;

            if(!isID(lex,j))
            {
                makeSyntaError("def call net",lex[j]);
                return false;
            }

            AString first=stringLexema(lex[j++]);
            arg.name=first;

            //массив шнурков
            if(isOper(lex,j,'['))
            {
                if(!syntaxNetArray(arg, lex, j, "call array"))
                    return false;
            }
            if(isOper(lex,j,'{'))
            {
                if(arg.first>0)
                {
                    makeSyntaError("call+ []{}",lex[j]);
                    return false;
                }

                j++;
                if(!isConst(lex,j))
                {
                    makeSyntaError("call+ array",lex[j]);
                    return false;
                }
                arg.bit_first=arg.bit_last=stringLexema(lex[j++]).toInt<int>();

                if(isOper(lex,j,'.') || isOper(lex,j,'-'))
                {
                    arg.viaPoint=0;
                    if((isOper(lex,j,'.') && (!isOper(lex,j+1,'.') || !isConst(lex,j+2))) ||
                            (isOper(lex,j,'-') && !isConst(lex,j+1)))
                    {
                        makeSyntaError("call+ array",lex[j]);
                        return false;
                    }
                    else if(isOper(lex,j,'.'))
                    {
                        arg.viaPoint=1;
                        j++;
                    }
                    j++;

                    arg.bit_last=stringLexema(lex[j++]).toInt<int>();
                }
                if(!isOper(lex,j,'}'))
                {
                    makeSyntaError("call+ array",lex[j]);
                    return false;
                }
                j++;

                if(arg.first>arg.last)
                {
                    makeSyntaError("call+ first>last",lex[j]);
                    return false;
                }
            }
            call.args.append(arg);
        }

        if(!isOper(lex,j,')') || !isOper(lex,j+1,';'))
        {
            makeSyntaError("call+ array",lex[j]);
            return false;
        }
        j+=2;

        tmp.calls.append(call);

    }

    if(next!="END" || !isOper(lex,j,';'))
    {
        makeSyntaError("def end",lex[j]);
        return false;
    }

    out.defs.insert(tmp.name,tmp);
    return true;
}

bool NetListReader::syntaxScipModule(ATArray<ALexElement> &lex, int &j)
{
    for(j++;j<lex.size();j++)
    {
        if(isID(lex,j) && isID(lex,j+1) && isOper(lex,j+2,';'))
        {
            if(stringLexema(lex[j])=="END" && stringLexema(lex[j+1])=="MODULE")
            {
                j+=2;
                return true;
            }
        }
    }
    return false;
}

void NetListReader::syntaxParce()
{
    ATArray<ADual<ATArray<ALexElement>*,int> > stack;

    for(int i=0;i<syntax.size();i++)
    {
        delete syntax.value(i);
    }
    syntax.clear();

    for(int i=0;i<cash.size();i++)
    {
        AData buffer=cash.value(i);
        ATArray<ALexElement> *dat=lexDatum[cash.key(i)];
        ATArray<ALexElement> prep;
        NetListFile *temp=new NetListFile;

        //препроцессируем и пропустим разделители и комментарии
        for(int j=0;j<dat->size();j++)
        {
            if((*dat)[j].type==LEX_SEP || (*dat)[j].type==LEX_UNDEF
                    || (*dat)[j].type==LEX_MACRO)
            {
                //просто пропустим
            }
            else if((*dat)[j].type==LEX_COMM)
            {
                if(prep.size()) //захват комментариев последней лексемой
                {
                    int off=(*dat)[j].offset+2;
                    int size=(*dat)[j].size-4;
                    AString comm=AString::fromFix((char*)(&(buffer()[off])),size);
                    prep.last().extra.append(comm);
                }
            }
            else if((*dat)[j].type==LEX_IFBLOCK && (*dat)[j].macroLexStack[0].macroLexStack.size()>0
                    && defines.contains((*dat)[j].extra[0].toString()))
            {
                //qDebug() << "Enter to" << (*dat)[j].extra[0].toString() << (*dat)[j].macroLexStack[0].macroLexStack.size();
                stack.append(ADual<ATArray<ALexElement>*,int>(dat,j+1));
                dat=&((*dat)[j].macroLexStack[0].macroLexStack);
                j=0;
            }
            else if((*dat)[j].type!=LEX_IFBLOCK)
            {
                prep.append((*dat)[j]);
            }

            if(j==dat->size()-1 && stack.size())
            {
                j=stack.last().right();
                dat=stack.last().left();
                stack.pop();
            }
        }

        //обработаем препроцессированные данные
        bool ok=true;
        for(int j=0;j<prep.size();j++)
        {
            int off=prep[j].offset;
            int size=prep[j].size;

            switch(prep[j].type)
            {
            case LEX_ID:
            {   AString id=AString::fromFix((char*)(&(buffer()[off])),size);
                if(id=="IMPORT")
                {
                    if(!syntaxMakeImport(*temp,prep,j))ok=false;
                }
                else if(id=="DEF")
                {
                    if(!syntaxMakeDef(*temp,prep,j))ok=false;
                }
                else if(id=="MODULE")
                {
                    if(!syntaxScipModule(prep,j))ok=false;
                }
                else
                {
                    makeSyntaError("undefined id",prep[j]);
                    ok=false;
                }
                break; }
            default:
                makeSyntaError("wrong lexem",prep[j]);
                ok=false;
            };
            if(!ok)break;
        }
        if(ok)
        {
            AStringPathParcer inf(cash.key(i));
            temp->name=inf.getNameNoExt();
            syntax.insert(cash.key(i),temp);
        }
        else
        {
            delete temp;
        }
    }
    syntaxVerify();
}

NetListFile *NetListReader::netlistByName(AString name)
{
    for(int i=0;i<syntax.size();i++)
    {
        if(syntax.value(i)->name.toLower()==name.toLower())
            return syntax.value(i);
    }
    return NULL;
}

NetListFileDef* NetListReader::findDefForCall(AString call_name)
{
    for(int ii=0;ii<syntax.size();ii++)
    {
        NetListFile *file=syntax.value(ii);
        NetListFileDef *tmp=file->containsDef(call_name);
        if(tmp)return tmp;
    }
    return NULL;
}

bool NetListReader::verifyCall(NetListFileDef *caller, NetListFileCall *call, NetListFileDef *def, int pass)
{
    bool rv_responsed=false;
    ATArray<NetListFileArgument> def_deco=def->genSepList();
    ATArray<NetListFileArgument> call_deco;

    bool haveInt=false;
    for(int i=0;i<def_deco.size();i++)
    {
        if(def_deco[i].bitsize>1){haveInt=true;break;}
    }

    //if(!pass && callerHaveInt)qDebug() << "!HaveInt!" << caller->name;

    bool normalPrim=!(call->name=="JOIN" || call->name=="JOIN_BUS" ||
                        call->name=="AND_PRIM" || call->name=="NOR_PRIM" ||
                        call->name=="OR_PRIM" || call->name=="XOR_PRIM" ||
                        call->name=="NAND_PRIM");

    bool isNumeral=call->label.first>=0 && call->label.first!=call->label.last;
    for(int i=0;i<call->args.size();i++)
    {
        bool unk=true;
        NetListFileArgument *arg=caller->findArg(&call->args[i]);
        if(arg)
        {
            call->args[i].bitsize=arg->bitsize;
            unk=false;
        }

        if(pass<0 && call->args[i].bitsize<0)
        {
            call->args[i].bitsize=1;
            qDebug() << "Foce single" << call->args[i].print() << caller->name;
        }

        if(call->args[i].type.isEmpty() && normalPrim && def_deco.size()>call_deco.size())
        {
            call->args[i].type=def_deco[call_deco.size()].type;
        }

        if(normalPrim && call->args[i].bitsize<0 && call->args[i].bit_first<0 && def_deco.size()>call_deco.size() &&
                (call->label.first!=call->label.last || haveInt /*|| (!callerHaveInt && pass)*/
                 /*|| call->args[i].first>=0*/))
        {
            if(def_deco[call_deco.size()].bitsize>1 && call->args[i].first>=0)
                qDebug() << "WARNING Indexed buses" << call->args[i].print() << caller->name;
            call->args[i].bitsize=def_deco[call_deco.size()].bitsize;
        }

        if(!haveInt && call->args[i].bitsize<0 && call->args[i].bit_first<0 &&
                (call->label.first!=call->label.last))
        {
            call->args[i].bitsize=1;
        }

        if(call->args[i].bitsize<0 && call->args[i].bit_first<0 &&
                (call->name=="TIE0" || call->name=="TIE1"
                                       || call->name=="TIE1_PRIM" || call->name=="TIE0_PRIM"))
        {
            call->args[i].bitsize=1;
        }

        NetListFileArgument tmp=call->args[i];

        if((!isNumeral && call->args[i].first>=0) || (isNumeral && call->label.viaPoint!=call->args[i].viaPoint))
        {
            for(int j=call->args[i].first;j<=call->args[i].last;j++)
            {
                tmp.first=tmp.last=j;
                if(call->args[i].bitsize<0)
                {
                    NetListFileArgument *arg=caller->findArg(&tmp);
                    if(arg)
                    {
                        tmp.bitsize=call->args[i].bitsize=arg->bitsize;
                        for(int x=0;x<(j-call->args[i].first);x++)
                        {
                            call_deco[call_deco.size()-1-x].bitsize=arg->bitsize;
                        }
                        unk=false;
                    }
                }

                call_deco.append(tmp);
            }
        }
        else if((!isNumeral && call->args[i].bit_first>=0) || (isNumeral && call->label.viaPoint!=call->args[i].viaPoint))
        {
            for(int j=call->args[i].bit_first;j<=call->args[i].bit_last;j++)
            {
                tmp.bit_first=tmp.bit_last=j;
                call_deco.append(tmp);
            }
        }
        else
        {
            call_deco.append(tmp);
        }

        if(unk && call->args[i].bitsize>0)
        {
            rv_responsed=true;
            caller->unks.insertAll(call->args[i].name,call->args[i]);
        }
    }

    if(call->name=="JOIN" || call->name=="JOIN_BUS")
    {
        if(call->args[0].bitsize<0)
        {
            bool k=true;
            for(int i=1;i<call->args.size();i++)
            {
                if(call->args[i].bitsize<0)
                {
                    k=false;
                    break;
                }
            }
            if(k && call->args[0].first<0 && call->args[0].bit_first<0)
            {
                call->args[0].bitsize=call->connectomTotal()+1;
                caller->unks.insertAll(call->args[0].name,call->args[0]);
                rv_responsed=true;
            }
        }
        else if(call->args[0].bitsize>0)
        {
            int k=0,ind;
            int tot=0;
            for(int i=0;i<call->args.size();i++)
            {
                if(call->args[i].bitsize<0){k++;ind=i;}
                else if(i)tot+=call->args[i].bitTotal();
            }
            if(k==1 && call->args[ind].first<0 && call->args[ind].bit_first<0)
            {
                call->args[ind].bitsize=call->args[0].bitsize-tot;
                caller->unks.insertAll(call->args[ind].name,call->args[ind]);
                rv_responsed=true;
            }
        }
    }

    if(def_deco.size()!=call_deco.size() && normalPrim)
    {
        qDebug() << "Verify Error diff size: " << caller->name << def->name;
        qDebug() << call->print() << call_deco.size();
        qDebug() << def->printArgs() << def_deco.size();
    }
    else if(!normalPrim && call->name!="JOIN" && call->name!="JOIN_BUS")
    {
        if(call->args[0].bitsize<0 && !haveInt)
        {
            bool k=true;
            for(int i=1;i<call->args.size();i++)
            {
                if(!call->args[i].justBit())
                {
                    k=false;
                    break;
                }
            }
            if(k && call->args[0].bit_first<0)
            {
                call->args[0].bitsize=1;
                caller->unks.insertAll(call->args[0].name,call->args[0]);
                rv_responsed=true;
            }
        }
    }
    if(def_deco.size()==call_deco.size() && normalPrim && !haveInt)
    {
        bool ok=true;
        bool unk_out=false;
        bool unk_in=false;
        int curr_out_size=-1;
        int curr_in_size=-1;

        for(int i=0;i<def_deco.size();i++)
        {
            if(call_deco[i].bitsize<0)
            {
                if(def_deco[i].type=="OUT" || def_deco[i].type=="IO" || def_deco[i].type=="TRI")
                {
                    unk_out=true;
                }
                else
                {
                    unk_in=true;
                }
            }
            else
            {
                if(def_deco[i].type=="OUT" || def_deco[i].type=="IO" || def_deco[i].type=="TRI")
                {
                    if(curr_out_size<0)curr_out_size=call_deco[i].bitTotal();
                    else if(call_deco[i].bitTotal()!=curr_out_size)
                    {
                        ok=false;
                        //qDebug() << "EpicFail OUT" << call->print();
                        //qDebug() << def->printArgs();
                        break;
                    }
                }
                else
                {
                    if(curr_in_size<call_deco[i].bitTotal())
                    {
                        if(curr_in_size>1)
                        {
                            ok=false;
                            //qDebug() << "EpicFail IN" << call->print();
                            //qDebug() << def->printArgs();
                            break;
                        }
                        curr_in_size=call_deco[i].bitTotal();
                    }
                }
            }
        }

        if(ok)
        {
            if(unk_out && ((curr_in_size>0 && !unk_in) || curr_out_size>0))
            {
                if(curr_out_size<0)curr_out_size=curr_in_size;
                for(int i=0;i<call->args.size();i++)
                {
                    if(call->args[i].bitsize<0 && call->args[i].bit_first<0 && (call->args[i].type=="OUT" || call->args[i].type=="IO"|| call->args[i].type=="TRI"))
                    {
                        call->args[i].bitsize=curr_out_size;
                        caller->unks.insertAll(call->args[i].name,call->args[i]);
                        rv_responsed=true;
                    }
                }
            }
            if(unk_in && !unk_out && curr_out_size==1)
            {
                for(int i=0;i<call->args.size();i++)
                {
                    if(call->args[i].bitsize<0 && call->args[i].bit_first<0)
                    {
                        call->args[i].bitsize=1;
                        caller->unks.insertAll(call->args[i].name,call->args[i]);
                        rv_responsed=true;
                    }
                }
            }
        }
    }

    return rv_responsed;
}

void NetListReader::makePrimitive(AString name)
{
    NetListFileDef *tmp=findDefForCall(name);
    if(tmp){tmp->calls.clear();primitives.insert(name);}
}

void NetListReader::syntaxVerify()
{
    for(int ii=0;ii<syntax.size();ii++)
    {
        NetListFile *file=syntax.value(ii);

        //create full import lists
        ATSet<AString> imports;
        imports.insert(file->imports);

        for(int i=0;i<imports.size();i++)
        {
            if(exclude_imports.contains(imports[i]))continue;

            NetListFile *imp=netlistByName(imports[i]);
            if(!imp)
            {
                qDebug() << "Verify Error Not Exists: " << syntax.key(ii) << imports[i];
            }
            else
            {
                imports.insert(imp->imports);
            }
        }
        file->imports=imports-exclude_imports;

        //check
        ATSet<AString> defall;
        for(int i=0;i<file->defs.size();i++)
        {
            if(!file->defs.value_ref(i).calls.size())
                primitives.insert(file->defs.value_ref(i).name);
            if(defall.contains(file->defs.value_ref(i).name))
            {
                qDebug() << "Dublicated Def: " << file->defs.value_ref(i).name;
            }
            defall.insert(file->defs.value_ref(i).name);

            ATSet<AString> unk;
            for(int j=0;j<file->defs.value_ref(i).calls.size();j++)
            {
                NetListFileDef *def=findDefForCall(file->defs.value_ref(i).calls[j].name);
                if(!def)
                {
                    unk.insert(file->defs.value_ref(i).calls[j].name);
                }
                else
                {
                    verifyCall(&file->defs.value_ref(i),&file->defs.value_ref(i).calls[j],def,0);
                }
            }
            if(unk.size())
            {
                AString str="Unfounded "+file->name+": ";
                for(int j=0;j<unk.size();j++)
                {
                    if(j)str+=",";
                    str+=unk[j];
                }
                qDebug() << str;
            }
        }


        bool tst_again=false;
        for(int i=0;i<file->defs.size();i++)
        {
            tst_again=false;
            for(int j=0;j<file->defs.value_ref(i).calls.size();j++)
            {
                NetListFileDef *def=findDefForCall(file->defs.value_ref(i).calls[j].name);
                if(def)
                {
                    tst_again|=verifyCall(&file->defs.value_ref(i),&file->defs.value_ref(i).calls[j],def,1);
                }
            }
            if(tst_again)i--;
        }

        for(int i=0;i<file->defs.size();i++)
        {
            for(int j=0;j<file->defs.value_ref(i).calls.size();j++)
            {
                NetListFileDef *def=findDefForCall(file->defs.value_ref(i).calls[j].name);
                if(def)
                {
                    verifyCall(&file->defs.value_ref(i),&file->defs.value_ref(i).calls[j],def,-1);
                }
            }
        }

        //REMOVE ZEROS
        for(int i=0;i<file->defs.size();i++)
        {
            for(int j=0;j<file->defs.value_ref(i).calls.size();j++)
            {
                if(file->defs.value_ref(i).calls[j].name=="TIE1_PRIM" ||
                        file->defs.value_ref(i).calls[j].name=="TIE1")
                {
                    makeConstant(&file->defs.value_ref(i),j,1);
                }
                else if(file->defs.value_ref(i).calls[j].name=="TIE0_PRIM" ||
                        file->defs.value_ref(i).calls[j].name=="TIE0")
                {
                    makeConstant(&file->defs.value_ref(i),j,0);
                }
            }
        }
    }
    if(primitives.size())
    {
        AString str="Primitives["+AString::fromInt(primitives.size())+"]: ";
        for(int j=0;j<primitives.size();j++)
        {
            if(j)str+=", ";
            str+=primitives[j];
        }
        qDebug() << str;
    }
}

void NetListReader::makeConstant(NetListFileDef *def, int callInd, int val)
{
    if(def->calls[callInd].label.first>=0 ||
            def->calls[callInd].args[0].bitTotal()>1)
    {
        qDebug() << "Error Constant" << def->name << def->calls[callInd].label.print();
        return;
    }
    if(def->findArg(&def->calls[callInd].args[0],true))
    {
        def->calls[callInd].name="JOIN";
        NetListFileArgument arg;
        arg.bitCode=val;
        arg.bitsize=1;
        def->calls[callInd].args.append(arg);
    }
    else
    {
        def->calls[callInd].name="DUMMY_PRIM";
        for(int i=0;i<def->calls.size();i++)
        {
            for(int j=0;j<def->calls[i].args.size();j++)
            {
                if(def->calls[callInd].args[0].name==def->calls[i].args[j].name
                        && def->calls[i].args[j].first<0)
                {
                    def->calls[i].args[j].bitCode=val;
                }
            }
        }
    }
}

void NetListReader::buildLexTable()
{
    for(int i=0;i<256;i++)lexTable[i]=LEX_UNDEF;

    lexTable[' ']=LEX_SEP;
    lexTable['\t']=LEX_SEP;
    lexTable['\r']=LEX_SEP;
    lexTable['\n']=LEX_SEP;
    lexTable['\v']=LEX_SEP;
    lexTable['\f']=LEX_SEP;
    lexTable[',']=LEX_SEP;

    const char ops[]="{}[]<>().:;=/-";
    for(uintx i=0;i<sizeof(ops)/sizeof(ops[0]);i++)lexTable[(uint8)ops[i]]=LEX_OPER;

    lexTable['/']=LEX_COMM;
    lexTable['(']=LEX_COMM;

    for(int v='0';v<='9';v++)lexTable[v]=LEX_CONST;

    lexTable['_']=LEX_ID;
    lexTable['\\']=LEX_ID;
    for(int v='A';v<='Z';v++)lexTable[v]=LEX_ID;
    for(int v='a';v<='z';v++)lexTable[v]=LEX_ID;

    lexTable['#']=LEX_MACRO;
}

void NetListReader::parceCurrent()
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
            makeError(ERR_UNDEFINED_LEX);
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
        case LEX_MACRO:
            prepDirective(el);
            continue;
        case LEX_OPER:
            el.size=fileStack[ind]->position()-el.offset;
            break;
        case LEX_ID:
            scipID(ind);
            el.size=fileStack[ind]->position()-el.offset;
            break;
        case LEX_CONST:
            scipConst(ind, csym);
            el.size=fileStack[ind]->position()-el.offset;
            break;
        };
        fileStack[ind]->currLexStack->append(el);
    }
}

bool NetListReader::scipComment(int ind)
{
    int incount=0;
    if(fileStack[ind]->getSym(0)=='*')
    {//блочный комментарий
        fileStack[ind]->popSym();
        while(!fileStack[ind]->atEnd())
        {
            if(fileStack[ind]->getSym(1)=='*' &&
                    (fileStack[ind]->getSym(0)=='/' || fileStack[ind]->getSym(0)=='('))
                incount++;

            if(fileStack[ind]->getSym(0)=='*' &&
                    (fileStack[ind]->getSym(1)=='/' || fileStack[ind]->getSym(1)==')'))
            {
                fileStack[ind]->popSym();
                fileStack[ind]->popSym();
                if(!incount)break;
                incount--;
            }
            fileStack[ind]->popSym();
        }
        return true;
    }
    return false;
}

void NetListReader::prepDirective(ALexElement &el)
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
        else if(csym=='\n' || lexTable[(uint8)csym]!=LEX_SEP)
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
        case PREP_IF:
            prepParceIf(el);
            break;
        case PREP_ENDIF:
            prepParceEndif(el);
            break;
        default:
            prepParceMacro(el);
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

void NetListReader::prepParceMacro(ALexElement &el)
{
    int ind=fileStack.size()-1;
    int start = fileStack[ind]->position();
    scipLine(ind);
    el.extra.append(AVariant(start));
    el.extra.append(AVariant(fileStack[ind]->position()-start));
}

void NetListReader::prepParceIf(ALexElement &el)
{
    int ind=fileStack.size()-1;
    int start = fileStack[ind]->position();
    scipLine(ind);

    el.extra.append(fileStack[ind]->getElement(start,fileStack[ind]->position()-start).trimmed());

    el.size=fileStack[ind]->position()-el.offset;
    el.type=LEX_IFBLOCK;

    ALexElement bel;
    bel.type=LEX_MACRO;
    bel.file_index=el.file_index;
    bel.offset=el.offset;
    bel.columne=el.columne;
    bel.line=el.line;
    bel.size=fileStack[ind]->position()-el.offset;

    //готовимся к рекурсии
    el.macroLexStack.append(bel);
    fileStack[ind]->currLexStack->append(el);

    //организуем следующий узел
    fileStack[ind]->ifBlockStack.append(fileStack[ind]->currLexStack);
    fileStack[ind]->currLexStack=&fileStack[ind]->currLexStack->last().macroLexStack.last().macroLexStack;

}

void NetListReader::prepParceEndif(ALexElement &el)
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

void NetListReader::buildPrepDict()
{
    prepDict.insert("if",PREP_IF);
    prepDict.insert("endif",PREP_ENDIF);
    prepDict.insert("GRIDCOUNT",PREP_GRIDCOUNT);
    prepDict.insert("GATECOUNT",PREP_GATECOUNT);
    prepDict.insert("MEGACOUNT",PREP_MEGACOUNT);
    prepDict.insert("TYPEWIDE",PREP_TYPEWIDE);
    prepDict.insert("PRIMITIVE",PREP_PRIMITIVE);
}

ATArray<ATArray<NetListFileArgument> > unitingCall(ATArray<NetListFileArgument> &call, ATArray<NetListFileArgument> &def)
{
    ATArray<ATArray<NetListFileArgument> > rv;
    for(int i=0;i<call.size();)
    {
        NetListFileArgument def_ag=def[i];
        NetListFileArgument call_ag=call[i];
        ATArray<NetListFileArgument> tmp;
        for(i=i+1;i<call.size() && def_ag.isNext(def[i]);i++)
        {
            if(!call_ag.isNext(call[i]))
            {
                tmp.append(call_ag);
                call_ag=call[i];
            }
            else
            {
                call_ag.addNext(call[i]);
            }
            def_ag.addNext(def[i]);
        }
        tmp.append(call_ag);
        rv.append(tmp);
    }
    return rv;
}

void NetListReader::callReductor(NetListFileCall *call)
{
    if(call->name=="ND5"){call->name="NAND_PRIM";return;}
    if(call->name=="ND7"){call->name="NAND_PRIM";return;}
    if(call->name=="ND9"){call->name="NAND_PRIM";return;}
    if(call->name=="ND2M"){call->name="NAND_PRIM";return;}
    if(call->name=="ND2U"){call->name="NAND_PRIM";return;}
    if(call->name=="ND5P"){call->name="NAND_PRIM";return;}
    if(call->name=="ND26"){call->name="NAND_PRIM";return;}
    if(call->name=="ND11"){call->name="NAND_PRIM";return;}
    if(call->name=="ND10"){call->name="NAND_PRIM";return;}
    if(call->name=="ND16"){call->name="NAND_PRIM";return;}
    if(call->name=="NAND14"){call->name="NAND_PRIM";return;}

    if(call->name=="OR2U"){call->name="OR_PRIM";return;}
    if(call->name=="OR3U"){call->name="OR_PRIM";return;}
    if(call->name=="OR3_H"){call->name="OR_PRIM";return;}
    if(call->name=="OR2_H"){call->name="OR_PRIM";return;}
    if(call->name=="OR7M"){call->name="OR_PRIM";return;}
    if(call->name=="OR9"){call->name="OR_PRIM";return;}
    if(call->name=="OR16"){call->name="OR_PRIM";return;}

    if(call->name=="NR5"){call->name="NOR_PRIM";return;}
    if(call->name=="NR14"){call->name="NOR_PRIM";return;}
    if(call->name=="NR16"){call->name="NOR_PRIM";return;}
    if(call->name=="NR26"){call->name="NOR_PRIM";return;}
    if(call->name=="NR32"){call->name="NOR_PRIM";return;}

    if(call->name=="AN2U"){call->name="AND_PRIM";return;}
    if(call->name=="AN5"){call->name="AND_PRIM";return;}
    if(call->name=="AN2M"){call->name="AND_PRIM";return;}
    if(call->name=="AN2H"){call->name="AND_PRIM";return;}
    if(call->name=="AN6M"){call->name="AND_PRIM";return;}
    if(call->name=="AN3U"){call->name="AND_PRIM";return;}
    if(call->name=="AN7H"){call->name="AND_PRIM";return;}
    if(call->name=="AN7M"){call->name="AND_PRIM";return;}
    if(call->name=="AN7U"){call->name="AND_PRIM";return;}
    if(call->name=="AN6U"){call->name="AND_PRIM";return;}
    if(call->name=="AN4M"){call->name="AND_PRIM";return;}
    if(call->name=="AN4U"){call->name="AND_PRIM";return;}
    if(call->name=="AN3H"){call->name="AND_PRIM";return;}
    if(call->name=="AN10"){call->name="AND_PRIM";return;}
    if(call->name=="AN3M"){call->name="AND_PRIM";return;}
    if(call->name=="AN5P"){call->name="AND_PRIM";return;}
    if(call->name=="AN4H"){call->name="AND_PRIM";return;}
    if(call->name=="AND10"){call->name="AND_PRIM";return;}
    if(call->name=="AND11"){call->name="AND_PRIM";return;}
    if(call->name=="AND12"){call->name="AND_PRIM";return;}

    NetListFileDef *def=searchDef(call->name);

    for(int i=0;i<def->calls.size();i++)
    {
        callReductor(&def->calls[i]);
        NetListFileDef *def_test=searchDef(def->calls[i].name);
        if((!primitives.contains(def->calls[i].name) && !def_test->calls.size()) ||
                def->calls[i].name=="DUMMY_PRIM")
        {
            if(i+1<def->calls.size())
                def->calls[i+1].label.comment=def->calls[i].label.comment+"\n"+def->calls[i+1].label.comment;
            def->calls.cut(i);i--;
            continue;
        }
        if((def->calls[i].name=="BUF_PRIM" || def->calls[i].name=="JOIN" || def->calls[i].name=="JOIN_BUS") &&
                def->calls[i].args.size()==2 &&
                def->calls[i].args[0].isSinonimal(def->calls[i].args[1]))
        {
            if(!def->findArgSoft(&def->calls[i].args[1]))
            {
                ATArray<NetListFileArgument*> list = def->findArgInCallsSoft(&def->calls[i].args[1]);
                for(int k=0;k<list.size();k++)
                {
                    list[k]->name=def->calls[i].args[0].name;
                }
                if(i+1<def->calls.size())
                    def->calls[i+1].label.comment=def->calls[i].label.comment+"\n"+def->calls[i+1].label.comment;
                def->calls.cut(i);i--;
                continue;
            }
        }
    }

    if(def->calls.size()!=1)return;
    if(def->locals.size())return;
    if(def->calls[0].args.size()<def->args.size())return;
    if(def->calls[0].label.first>=0)return;

    for(int i=0;i<def->args.size();i++)
    {
        if(def->args.value_ref(i).name!=def->calls[0].args[i].name)return;
        if(def->args.value_ref(i).bitsize!=def->calls[0].args[i].bitsize)return;
        if(def->args.value_ref(i).first!=def->calls[0].args[i].first)return;
        if(def->args.value_ref(i).last!=def->calls[0].args[i].last)return;
        if(def->args.value_ref(i).bit_last!=def->calls[0].args[i].bit_last)return;
        if(def->args.value_ref(i).bit_first!=def->calls[0].args[i].bit_first)return;
    }

    for(int i=def->args.size();i<def->calls[0].args.size();i++)
    {
        if(def->calls[0].args[i].bitCode<0)return;
    }

    for(int i=def->args.size();i<def->calls[0].args.size();i++)
    {
        call->args.append(def->calls[0].args[i]);
    }

    call->name=def->calls[0].name;
}

void NetListReader::optimizeUnitedDef(NetListFileDef *curr, NetListFileCall *call, int level)
{
    //редуцирование примитивов
    for(int i=0;i<curr->calls.size();i++)
    {
        callReductor(&curr->calls[i]);
        NetListFileDef *def_test=searchDef(curr->calls[i].name);
        if((!primitives.contains(curr->calls[i].name) && !def_test->calls.size()) ||
                curr->calls[i].name=="DUMMY_PRIM" ||
                (curr->calls[i].name=="JOIN" && curr->calls[i].args[0].unconnected) ||
                (curr->calls[i].name=="JOIN_BUS" && curr->calls[i].args[0].unconnected) ||
                (curr->calls[i].name=="BUF_PRIM" && curr->calls[i].args[0].unconnected))
        {
            if(i+1<curr->calls.size())
                curr->calls[i+1].label.comment=curr->calls[i].label.comment+"\n"+curr->calls[i+1].label.comment;
            curr->calls.cut(i);i--;
            continue;
        }
    }

    //рекурсивная редукция
    for(int i=0;i<curr->calls.size();i++)
    {
        NetListFileDef *tmp=searchDef(curr->calls[i].name);
        if(tmp && !primitives.contains(curr->calls[i].name))
        {
            optimizeUnitedDef(tmp,&curr->calls[i],level+1);
        }
    }

    //разметка неиспользуемых
    for(int i=0;i<curr->calls.size();i++)
    {
        for(int j=0;j<curr->calls[i].args.size();j++)
        {
            if(curr->calls[i].args[j].bitCode>=0)continue;
            if(curr->findArgSoft(&curr->calls[i].args[j]))
                continue;
            ATArray<NetListFileArgument*> list=curr->findArgInCallsSoft(&curr->calls[i].args[j]);
            if(list.size()>1)continue;
            if(!(curr->calls[i].args[j].type=="OUT" || curr->calls[i].args[j].type=="IO")
                    && !((curr->calls[i].name=="JOIN" || curr->calls[i].name=="JOIN_BUS") && j==0))
            {
                qDebug() << "Warning ? type not OUT" << curr->calls[i].args[j].print() << curr->name;
                continue;
            }
            curr->calls[i].args[j].unconnected=true;
        }
    }

    //редукция с перестановкой и дополнением
    for(int i=0;i<curr->calls.size();i++)
    {
        if(primitives.contains(curr->calls[i].name))continue;

        NetListFileDef *tmp=searchDef(curr->calls[i].name);
        if(tmp->calls.size()!=1)
        {
            //if(curr->calls[i].name=="FD2Q")qDebug() << ">>>>> Fail 1";
            continue;
        }
        if(tmp->calls[0].label.first>=0)
        {
            continue;
        }

        ATArray<NetListFileArgument> call_sep=curr->calls[i].genSepList();
        ATArray<NetListFileArgument> def_sep=tmp->genSepList();

        if(call_sep.size()==def_sep.size())
        {
            ATArray<ATArray<NetListFileArgument> > groupped=unitingCall(call_sep,def_sep);
            if(groupped.size()==tmp->args.size())
            {
                bool uncon=true;
                for(int j=0;j<groupped.size();j++)
                {
                    if(groupped[j].size()==1)continue;
                    uncon=false;
                    break;
                }
                if(uncon) //теперь объединяем
                {
                    ATArray<NetListFileArgument> recall;
                    for(int j=0;j<tmp->calls[0].args.size();j++)
                    {
                        if(tmp->calls[0].args[j].bitCode>=0 || tmp->calls[0].args[j].unconnected)
                        {
                            recall.append(tmp->calls[0].args[j]);
                            recall.last().name=tmp->calls[0].label.name+"."+recall.last().name;
                        }
                        else
                        {
                            int idx=tmp->findStrongAttr(&tmp->calls[0].args[j]);
                            if(idx<0)break;
                            recall.append(groupped[idx][0]);
                            recall.last().type=tmp->calls[0].args[j].type;
                        }
                    }
                    if(recall.size()==tmp->calls[0].args.size())
                    {
                        curr->calls[i].name=tmp->calls[0].name;
                        curr->calls[i].args=recall;
                    }
                }
            }
        }
    }
}

AString NetListReader::printUnitedDef(NetListFileDef *curr, NetListFileCall *call, int level)
{
    AString rv;

    if(!call)
    {
        rv+="def "+curr->name+"(\n";
        for(int i=0;i<curr->args.size();i++)
        {
            rv+="\t"+curr->args.value_ref(i).print()+",";

            if(!curr->args.value_ref(i).comment.isEmpty())
            {
                ATArray<AString> list=curr->args.value_ref(i).comment.split('\n',true);
                for(int k=0;k<list.size();k++)
                {
                    if(k)rv+="\n\t";
                    rv+="\t///"+list[k];
                }
            }
            rv+="\n";
        }
        rv+=")"+curr->printRegs()+"{\n";
    }
    else
    {
        rv+=AString::mono('\t',level);
        rv+=call->label.print()+" := "+curr->name+"(\n";

        ATArray<NetListFileArgument> call_sep=call->genSepList();
        ATArray<NetListFileArgument> def_sep=curr->genSepList();

        if(call_sep.size()!=def_sep.size())
        {
            return AString::mono('\t',level+1)+"!!!Error CALL<=>DEF!!!\n";
        }

        ATArray<ATArray<NetListFileArgument> > groupped=unitingCall(call_sep,def_sep);

        if(groupped.size()!=curr->args.size())
        {
            return AString::mono('\t',level+1)+"!!!Error group CALL<=>DEF!!!\n";
        }

        for(int i=0;i<groupped.size();i++)
        {
            rv+=AString::mono('\t',level+1)+curr->args.value_ref(i).print(false)+"\t=\t";
            for(int j=0;j<groupped[i].size();j++)
            {
                if(j)rv+=" @ ";
                rv+=groupped[i][j].print();                
            }
            rv+=",";
            if(!curr->args.value_ref(i).comment.isEmpty())
            {
                ATArray<AString> list=curr->args.value_ref(i).comment.split('\n',true);
                for(int k=0;k<list.size();k++)
                {
                    if(k)rv+="\n"+AString::mono('\t',level+1);
                    rv+="\t///"+list[k];
                }
            }
            rv+="\n";
        }

        rv+=AString::mono('\t',level);
        rv+=")"+curr->printRegs()+"{\n";
    }

    //вывод в текстовом виде
    for(int i=0;i<curr->calls.size();i++)
    {
        ATArray<AString> list=curr->calls[i].label.comment.split('\n',true);
        for(int k=0;k<list.size();k++)
        {
            rv+=AString::mono('\t',level+1);
            rv+="///"+list[k]+"\n";
        }

        NetListFileDef *tmp=searchDef(curr->calls[i].name);
        if(tmp && !primitives.contains(curr->calls[i].name))
        {
            rv+=printUnitedDef(tmp,&curr->calls[i],level+1);
        }
        else
        {
            rv+=printPrim(&curr->calls[i],AString::mono('\t',level+1));
        }
    }

    if(level)rv+=AString::mono('\t',level);
    rv+="};\n";

    return rv;
}

AString NetListReader::printInlinedDef(NetListFileDef *curr, NetListFileCall *call, AString pref, ATSet<AString> &globalNames)
{
    AString rv;
    if(!call) //начальная часть
    {
        rv+="def "+curr->name+"(\n";
        for(int i=0;i<curr->args.size();i++)
        {
            rv+="\t"+curr->args.value_ref(i).print()+",";
            globalNames.insert(curr->args.value_ref(i).name);

            if(!curr->args.value_ref(i).comment.isEmpty())
            {
                ATArray<AString> list=curr->args.value_ref(i).comment.split('\n',true);
                for(int k=0;k<list.size();k++)
                {
                    if(k)rv+="\n\t";
                    rv+="\t///"+list[k];
                }
            }
            rv+="\n";
        }
        rv+=")"+curr->printRegs()+"{\n";

        for(int i=0;i<curr->registers.size();i++)
            globalNames.insert(curr->registers[i].name);

        for(int i=0;i<curr->calls.size();i++)
        {
            for(int j=0;j<curr->calls[i].args.size();j++)
            {
                globalNames.insert(curr->calls[i].args[j].name);
            }
        }
    }
    else //вторичные
    {
        ATArray<NetListFileArgument> call_sep=call->genSepList();
        ATArray<NetListFileArgument> def_sep=curr->genSepList();

        if(call_sep.size()!=def_sep.size())
        {
            return "\t!!!Error CALL<=>DEF!!!\n";
        }

        ATArray<ATArray<NetListFileArgument> > groupped=unitingCall(call_sep,def_sep);

        if(groupped.size()!=curr->args.size())
        {
            return "\t!!!Error group CALL<=>DEF!!!\n";
        }

        bool can_inline=true;
        for(int i=0;i<groupped.size();i++)
        {
            if(groupped[i].size()!=1 ||
                    !groupped[i][0].isReplacable(curr->args.value_ref(i)))
            {
                can_inline=false;
                break;
            }
        }

        if(can_inline) //переименуем вызовы и регистры
        {
            ATSet<AString> tempUniq;
            for(int i=0;i<curr->calls.size();i++)
            {
                for(int j=0;j<curr->calls[i].args.size();j++)
                {
                    int ind=curr->findArgIndex(&curr->calls[i].args[j]);
                    if(ind<0) //проверяем на уникальность
                    {
                        if(globalNames.contains(curr->calls[i].args[j].name))
                        {
                            curr->calls[i].args[j].name=pref+curr->calls[i].args[j].name;
                        }
                        else
                        {
                            tempUniq.insert(curr->calls[i].args[j].name);
                        }
                    }
                    else //заменяем на переданный параметр
                    {
                        curr->calls[i].args[j].name=groupped[ind][0].name;
                        if(groupped[ind][0].first==groupped[ind][0].last)
                        {
                            curr->calls[i].args[j].first=groupped[ind][0].first;
                            curr->calls[i].args[j].last=groupped[ind][0].last;
                        }
                    }
                }
            }
            for(int i=0;i<curr->registers.size();i++)
            {
                int ind=curr->findArgIndex(&curr->registers[i]);
                if(ind<0) //проверяем на уникальность
                {
                    if(globalNames.contains(curr->registers[i].name))
                    {
                        curr->registers[i].name=pref+curr->registers[i].name;
                    }
                    else
                    {
                        tempUniq.insert(curr->registers[i].name);
                    }
                }
                else //заменяем на переданный параметр
                {
                    curr->registers[i].name=groupped[ind][0].name;
                    if(groupped[ind][0].first==groupped[ind][0].last)
                    {
                        curr->registers[i].first=groupped[ind][0].first;
                        curr->registers[i].last=groupped[ind][0].last;
                    }
                }
            }
            globalNames.insert(tempUniq);

            rv+="\t::"+call->label.print()+"{"+curr->printRegs()+"};\n";
        }
        else //выведем как есть
        {
            return printUnitedDef(curr,call,1);
        }
    }

    //вывод в текстовом виде
    for(int i=0;i<curr->calls.size();i++)
    {
        ATArray<AString> list=curr->calls[i].label.comment.split('\n',true);
        for(int k=0;k<list.size();k++)
        {
            rv+="\t";
            rv+="///"+list[k]+"\n";
        }

        NetListFileDef *tmp=searchDef(curr->calls[i].name);
        if(tmp && !primitives.contains(curr->calls[i].name))
        {
            rv+=printInlinedDef(tmp,&curr->calls[i],pref+curr->calls[i].label.print()+".",globalNames);
        }
        else
        {
            rv+=printPrim(&curr->calls[i],"\t");
        }
    }

    if(!call)rv+="};\n";

    return rv;
}


NetListFileDef *NetListReader::searchDef(AString name)
{
    NetListFileDef *def=NULL;

    for(int i=0;i<syntax.size();i++)
    {
        def=syntax.value_ref(i)->containsDef(name);
        if(def)break;
    }
    return def;
}

ATArray<ATArray<NetListFileArgument> > NetListReader::getCallOrdered(NetListFileCall *call)
{
    NetListFileDef *tmp=searchDef(call->name);
    if(!tmp)return ATArray<ATArray<NetListFileArgument> >();

    ATArray<NetListFileArgument> call_sep=call->genSepList();
    ATArray<NetListFileArgument> def_sep=tmp->genSepList();

    if(call_sep.size()!=def_sep.size())return ATArray<ATArray<NetListFileArgument> >();

    ATArray<ATArray<NetListFileArgument> > groupped=unitingCall(call_sep,def_sep);

    if(groupped.size()!=tmp->args.size())return ATArray<ATArray<NetListFileArgument> >();
    return groupped;
}

void NetListReader::makeRegisters()
{
    for(int i=0;i<syntax.size();i++)
    {
        for(int j=0;j<syntax.value_ref(i)->defs.size();j++)
        {
            for(int k=0;k<syntax.value_ref(i)->defs.value_ref(j).calls.size();k++)
            {
                NetListFileCall *call=&syntax.value_ref(i)->defs.value_ref(j).calls[k];
                if(call->name=="FLIPFLOP_PRIM" || call->name=="FLIPFLOPS_PRIM" || call->name=="LATCH_PRIM")
                {
                    ATArray<ATArray<NetListFileArgument> > groupped=getCallOrdered(call);

                    if(!groupped.size())continue;

                    if(groupped[0].size()==1 && groupped[1].size()==1)
                    {
                        if(!groupped[0][0].unconnected)
                        {
                            syntax.value_ref(i)->defs.value_ref(j).registers.append(groupped[0][0]);
                        }
                        else if(!groupped[0][1].unconnected)
                        {
                            syntax.value_ref(i)->defs.value_ref(j).registers.append(groupped[0][1]);
                        }
                    }
                }
            }
        }
    }
}

void NetListReader::compileElement(AString name)
{
    NetListFileDef *def=NULL;

    int i;
    for(i=0;i<syntax.size();i++)
    {
        def=syntax.value_ref(i)->containsDef(name);
        if(def)break;
    }

    if(!def)
    {
        qDebug() << "Compile Error - Not find " << name;
        return;
    }

    optimizeUnitedDef(def,NULL,0);
    optimizeUnitedDef(def,NULL,0);

    makeRegisters();

    AString res=printUnitedDef(def,NULL,0);

    QFile hand;
    AStringPathParcer fstr(syntax.key(i));
    hand.setFileName((fstr.getDirectory()+"/"+def->name+".unidef")());
    hand.open(QFile::Truncate|QFile::WriteOnly);
    hand.write(res());
    hand.close();

    ATSet<AString> set;
    res=printInlinedDef(def,NULL,"",set);

    hand.setFileName((fstr.getDirectory()+"/"+def->name+".idef")());
    hand.open(QFile::Truncate|QFile::WriteOnly);
    hand.write(res());
    hand.close();

}
