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

#ifndef NETLIST_READER_DEF_H
#define NETLIST_READER_DEF_H

#include "alex_base.h"

struct NetListFileArgument
{
    NetListFileArgument()
    {
        bitsize=-1;
        first=-1;
        last=-1;
        bit_first=-1;
        bit_last=-1;
        viaPoint=-1;
        strong=false;
        bitCode=-1;
        unconnected=false;
    }

    AString name;
    AString type;
    AString comment;
    int bitsize;
    int bit_first;
    int bit_last;
    int first;
    int last;
    bool strong;

    int viaPoint;

    int bitCode;
    bool unconnected;

    bool isReplacable(NetListFileArgument &val)
    {
        if(bitCode>=0 || unconnected)return false;
        if(val.bitCode>=0 || val.unconnected)return false;
        if(bitsize!=val.bitsize)return false;
        if(first!=last || val.first!=val.last)
        {
            if(first!=val.first)return false;
            if(last!=val.last)return false;
        }
        if(bit_first!=val.bit_first)return false;
        if(bit_last!=val.bit_last)return false;
        return true;
    }

    bool isSinonimal(NetListFileArgument &val)
    {
        if(bitCode>=0 || unconnected)return false;
        if(val.bitCode>=0 || val.unconnected)return false;
        if(val.bitsize!=bitsize)return false;
        if(val.first>=0 || first>=0)return false;
        if(val.bit_first>=0 || bit_first>=0)return false;
        return true;
    }

    bool isNext(NetListFileArgument &val)
    {
        if(bitCode>=0 || unconnected)return false;
        if(val.name!=name)return false;
        if(val.bitsize!=bitsize)return false;
        if(val.last>=0 && last>=0 && last+1==val.last)return true;
        if(val.bit_last>=0 && bit_last>=0 && bit_last+1==val.bit_last)return true;
        return false;
    }

    void addNext(NetListFileArgument &val)
    {
        if(val.last>=0)last=val.last;
        if(val.bit_last>=0)bit_last=val.bit_last;
    }

    bool justBit()
    {
        if(bitsize<0)return false;
        if(bit_first>=0)return true;
        return bitsize==1;
    }

    int bitTotal()
    {
        if(bitsize<0)return bitsize;
        if(first>=0)return (last-first+1)*bitsize;
        else if(bit_first>=0)return bit_last-bit_first+1;
        return bitsize;
    }

    AString print(bool withtype=true)
    {
        if(bitCode>=0)return bitCode?"1":"0";
        AString rv=unconnected?"?":name;
        if(bitsize>1)rv+="\'"+AString::fromInt(bitsize)+"\'";
        if(bitsize<0)rv+="\'?\'";
        if(first>=0)
        {
            if(first==last)rv+="["+AString::fromInt(first)+"]";
            else rv+="["+AString::fromInt(first)+(viaPoint?"..":"-")+AString::fromInt(last)+"]";
        }
        if(bit_first>=0)
        {
            if(bit_first==bit_last)rv+="{"+AString::fromInt(bit_first)+"}";
            else rv+="{"+AString::fromInt(bit_first)+(viaPoint?"..":"-")+AString::fromInt(bit_last)+"}";
        }
        if(!type.isEmpty() && withtype)rv+="/*"+type+"*/";
        return rv;
    }

};

struct NetListFileCall
{
    AString name;
    NetListFileArgument label;
    ATArray<NetListFileArgument> args;

    ATArray<NetListFileArgument> genSepList()
    {
        ATArray<NetListFileArgument> call_deco;

        bool isNumeral=label.first>=0 && label.first!=label.last;
        for(int i=0;i<args.size();i++)
        {
            NetListFileArgument tmp=args[i];

            if((!isNumeral && args[i].first>=0) || (isNumeral && label.viaPoint!=args[i].viaPoint))
            {
                for(int j=args[i].first;j<=args[i].last;j++)
                {
                    tmp.first=tmp.last=j;
                    call_deco.append(tmp);
                }
            }
            else if((!isNumeral && args[i].bit_first>=0) || (isNumeral && label.viaPoint!=args[i].viaPoint))
            {
                for(int j=args[i].bit_first;j<=args[i].bit_last;j++)
                {
                    tmp.bit_first=tmp.bit_last=j;
                    call_deco.append(tmp);
                }
            }
            else
            {
                call_deco.append(tmp);
            }
        }
        return call_deco;
    }

    AString print()
    {
        AString rv=label.print()+":="+name+"(";
        for(int i=0;i<args.size();i++)
        {
            if(i)rv+=",";
            rv+=args[i].print();
        }
        rv+=");";
        return rv;
    }

    int connectomTotal()
    {
        int tot=0;
        for(int i=0;i<args.size();i++)
        {
            if(args[i].first>=0)tot+=(args[i].last-args[i].first+1)*args[i].bitsize;
            else if(args[i].bit_first>=0)tot+=args[i].bit_last-args[i].bit_first+1;
            else tot+=args[i].bitsize;
        }
        return tot;
    }
};

struct NetListFileDef
{
    AString name;
    ATHash<AString, NetListFileArgument> args;
    ATHash<AString, NetListFileArgument> locals, unks;
    ATArray<NetListFileCall> calls;

    //данные после парсинга
    ATArray<NetListFileArgument> registers;

    ATArray<NetListFileArgument> genSepList();

    int findStrongAttr(NetListFileArgument *arg)
    {
        ATArray<int> list=args.index_list(arg->name);
        for(int i=0;i<list.size();i++)
        {
            if(args.value_ref(list[i]).name!=arg->name)continue;
            if(args.value_ref(list[i]).bitsize!=arg->bitsize)continue;
            if(args.value_ref(list[i]).first!=arg->first)continue;
            if(args.value_ref(list[i]).last!=arg->last)continue;
            if(args.value_ref(list[i]).bit_last!=arg->bit_last)continue;
            if(args.value_ref(list[i]).bit_first!=arg->bit_first)continue;

            return list[i];
        }
        return -1;
    }

    ATArray<NetListFileArgument*> findArgInCallsSoft(NetListFileArgument *arg)
    {
        ATArray<NetListFileArgument*> rv;
        for(int i=0;i<calls.size();i++)
        {
            for(int j=0;j<calls[i].args.size();j++)
            {
                if(calls[i].args[j].name==arg->name)
                {
                    if(calls[i].args[j].first>=0 && arg->first>=0) //проверим вхождение
                    {
                        rv.append(&calls[i].args[j]);
                    }
                    else if(calls[i].args[j].first<0 && arg->first<0)
                    {
                        rv.append(&calls[i].args[j]);
                    }
                }
            }
        }
        return rv;
    }

    NetListFileArgument* findArgSoft(NetListFileArgument *arg)
    {
        ATArray<int> list=args.index_list(arg->name);
        for(int i=0;i<list.size();i++)
        {
            if(args.value_ref(list[i]).name==arg->name)
            {
                if(args.value_ref(list[i]).first>=0 && arg->first>=0) //проверим вхождение
                {
                    return &args.value_ref(list[i]);
                }
                else if(args.value_ref(list[i]).first<0 && arg->first<0)
                {
                    return &args.value_ref(list[i]);
                }
            }
        }
        return NULL;
    }

    int findArgIndex(NetListFileArgument *arg)
    {
        ATArray<int> list=args.index_list(arg->name);
        for(int i=0;i<list.size();i++)
        {
            if(args.value_ref(list[i]).name==arg->name)
            {
                if(args.value_ref(list[i]).first>=0 && arg->first>=0) //проверим вхождение
                {
                    if(arg->first>=args.value_ref(list[i]).first && arg->last<=args.value_ref(list[i]).last)
                        return list[i];
                }
                else if(args.value_ref(list[i]).first<0 && arg->first<0)
                {
                    return list[i];
                }
            }            
        }
        return -1;
    }

    NetListFileArgument* findArg(NetListFileArgument *arg, bool scipLocal=false)
    {
        ATArray<int> list=args.index_list(arg->name);
        for(int i=0;i<list.size();i++)
        {
            if(args.value_ref(list[i]).name==arg->name)
            {
                if(args.value_ref(list[i]).first>=0 && arg->first>=0) //проверим вхождение
                {
                    if(arg->first>=args.value_ref(list[i]).first && arg->last<=args.value_ref(list[i]).last)
                        return &args.value_ref(list[i]);
                }
                else if(args.value_ref(list[i]).first<0 && arg->first<0)
                {
                    return &args.value_ref(list[i]);
                }
            }
        }
        if(scipLocal)return NULL;
        list=locals.index_list(arg->name);
        for(int i=0;i<list.size();i++)
        {
            if(locals.value_ref(list[i]).name==arg->name)
            {
                if(locals.value_ref(list[i]).first>=0 && arg->first>=0) //проверим вхождение
                {
                    if(arg->first>=locals.value_ref(list[i]).first && arg->last<=locals.value_ref(list[i]).last)
                        return &locals.value_ref(list[i]);
                }
                else if(locals.value_ref(list[i]).first<0 && arg->first<0)
                {
                    return &locals.value_ref(list[i]);
                }
            }
        }
        list=unks.index_list(arg->name);
        for(int i=0;i<list.size();i++)
        {
            if(unks.value_ref(list[i]).name==arg->name)
            {
                if(unks.value_ref(list[i]).first>=0 && arg->first>=0) //проверим вхождение
                {
                    if(arg->first>=unks.value_ref(list[i]).first && arg->last<=unks.value_ref(list[i]).last)
                        return &unks.value_ref(list[i]);
                }
                else if(unks.value_ref(list[i]).first<0 && arg->first<0)
                {
                    return &unks.value_ref(list[i]);
                }
            }
        }
        return NULL;
    }

    AString printArgs()
    {
        AString rv="def "+name+"(";
        for(int i=0;i<args.size();i++)
        {
            if(i)rv+=",";
            rv+=args.value_ref(i).print();
        }
        return rv+")";
    }

    AString printRegs()
    {
        AString rv;
        for(int i=0;i<registers.size();i++)
        {
            if(i)rv+=",";
            rv+=registers[i].print(false);
        }
        return rv;
    }

    AString printLocals()
    {
        AString rv;
        for(int i=0;i<locals.size();i++)
        {
            if(i)rv+=",";
            rv+=locals.value_ref(i).print();
        }
        return rv;
    }

    AString print()
    {
        AString rv="def "+name+"(\n";
        for(int i=0;i<args.size();i++)
        {
            rv+="\t"+args.value_ref(i).print()+"\n";
        }
        rv+="\t);\n\t";
        for(int i=0;i<locals.size();i++)
        {
            if(i)rv+=",";
            rv+=locals.value_ref(i).print();
        }
        rv+="\nbegin\n\n";
        for(int i=0;i<calls.size();i++)
        {
            rv+=calls[i].print()+"\n";
        }
        rv+="\nend;\n\n";
        return rv;
    }
};

struct NetListFile
{
    AString name;
    ATSet<AString> imports;
    ATHash<AString,NetListFileDef> defs;

    NetListFileDef* containsDef(AString val)
    {
        if(defs.contains(val))
            return &defs[val];
        return NULL;
    }

    AString print()
    {
        AString rv="import ";
        for(int i=0;i<imports.size();i++)
        {
            if(i)rv+=",";
            rv+=imports[i];
        }
        rv+=";\n\n";
        for(int i=0;i<defs.size();i++)
        {
            rv+=defs.value(i).print();
        }
        return rv;
    }
};

#endif // NETLIST_READER_DEF_H
