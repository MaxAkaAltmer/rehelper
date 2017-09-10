#include "netlist_reader.h"

bool isIdentical(ATArray<NetListFileArgument> &list, int total=-1)
{
    bool identical=true;
    if(total==-1)total=list.size();
    for(int i=1;i<total;i++)
    {
        if(list[0].bitTotal()!=list[i].bitTotal())
        {
            identical=false;
            break;
        }
    }
    return identical;
}

ATArray<NetListFileArgument> unitingArgs(ATArray<NetListFileArgument> args)
{
    ATArray<NetListFileArgument> rv;
    NetListFileArgument tmp=args[0];
    for(int i=1;i<args.size();i++)
    {
        if(!tmp.isNext(args[i]))
        {
            rv.append(tmp);
            tmp=args[i];
        }
        else
        {
            tmp.addNext(args[i]);
        }
    }
    rv.append(tmp);
    return rv;
}

AString printSA(ATArray<NetListFileArgument> &val)
{
    AString rv;
    for(int i=0;i<val.size();i++)
    {
        if(i)rv+="@";
        rv+=val[i].print(false);
    }
    return rv;
}

AString printList(ATArray<NetListFileArgument> &list, int from, int num)
{
    AString rv;
    for(int i=from;i<from+num;i++)
    {
        if(i!=from)rv+=",";
        rv+=list[i].print(false);
    }
    return rv;
}

bool isSubIdentical(ATArray<NetListFileArgument> &list, int total=-1)
{
    bool identical=true;

    if(total==-1)total=list.size();
    for(int i=1;i<total;i++)
    {
        if(list[0].bitTotal()!=list[i].bitTotal() && list[i].bitTotal()!=1)
        {
            identical=false;
            break;
        }
    }
    return identical;
}

AString NetListReader::printPrim(NetListFileCall *call, AString mono)
{
    if(call->name=="JOIN" || call->name=="BUF_PRIM")
    {
        AString rv=mono+call->args[0].print(false) + " <= ";
        for(int i=1;i<call->args.size();i++)
        {
            if(i-1)rv+=" @ ";
            rv+=call->args[i].print(false);
        }
        return rv+";\n";
    }
    else if(call->name=="NAND_PRIM")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isSubIdentical(list))
        {
            AString rv=mono+list[0].print(false) + " <= ~(";
            for(int i=1;i<list.size();i++)
            {
                if(i-1)rv+=" & ";
                if(list[i].bitTotal()==1 && list[0].bitTotal()!=1)
                    rv+="(-"+list[i].print(false)+")";
                else rv+=list[i].print(false);
            }
            return rv+");\n";
        }
    }
    else if(call->name=="NOR_PRIM")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isIdentical(list))
        {
            AString rv=mono+list[0].print(false) + " <= ~(";
            for(int i=1;i<list.size();i++)
            {
                if(i-1)rv+=" | ";
                rv+=list[i].print(false);
            }
            return rv+");\n";
        }
    }
    else if(call->name=="AND_PRIM")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isSubIdentical(list))
        {
            AString rv=mono+list[0].print(false) + " <= ";
            for(int i=1;i<list.size();i++)
            {
                if(i-1)rv+=" & ";
                if(list[i].bitTotal()==1 && list[0].bitTotal()!=1)
                    rv+="(-"+list[i].print(false)+")";
                else rv+=list[i].print(false);
            }
            return rv+";\n";
        }
    }
    else if(call->name=="XOR_PRIM")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isSubIdentical(list))
        {
            AString rv=mono+list[0].print(false) + " <= ";
            for(int i=1;i<list.size();i++)
            {
                if(i-1)rv+=" ^ ";
                if(list[i].bitTotal()==1 && list[0].bitTotal()!=1)
                    rv+="(-"+list[i].print(false)+")";
                else rv+=list[i].print(false);
            }
            return rv+";\n";
        }
    }
    else if(call->name=="OR_PRIM")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isSubIdentical(list))
        {
            AString rv=mono+list[0].print(false) + " <= ";
            for(int i=1;i<list.size();i++)
            {
                if(i-1)rv+=" | ";
                if(list[i].bitTotal()==1 && list[0].bitTotal()!=1)
                    rv+="(-"+list[i].print(false)+")";
                else rv+=list[i].print(false);
            }
            return rv+";\n";
        }
    }
    else if(call->name=="ANR2P" || call->name=="ANR2")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isIdentical(list) && list.size()==5)
        {
            AString rv=mono+list[0].print(false) + " <= "
                    +"~(("+list[1].print(false)+" & "+list[2].print(false)+") | ("
                    +list[3].print(false)+" & "+list[4].print(false)+"))";

            return rv+";\n";
        }
    }
    else if(call->name=="INV_PRIM")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isIdentical(list) && list.size()==2)
        {
            AString rv=mono+list[0].print(false) + " <= ~"+list[1].print(false);
            return rv+";\n";
        }
    }
    else if(call->name=="EN")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isIdentical(list) && list.size()==3)
        {
            AString rv=mono+list[0].print(false) + " <= ~("+list[1].print(false)
                    +" ^ "+list[2].print(false)+")";
            return rv+";\n";
        }
    }
    else if(call->name=="HS1")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isIdentical(list) && list.size()==4)
        {
            AString rv=mono+list[0].print(false) + " <= ~("+list[2].print(false)
                    +" ^ "+list[3].print(false)+");\n";
            rv+=mono+list[1].print(false) + + " <= "+list[2].print(false)
                    +" | "+list[3].print(false)+";\n";
            return rv;
        }
    }
    else if((call->name=="ADDER_PRIM") && call->label.first==call->label.last)
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(list.size()==5 && list[0].bitTotal()==list[2].bitTotal() && list[0].bitTotal()==list[3].bitTotal())
        {
            AString rv=mono+list[0].print(false);
            if(!list[1].unconnected)
            {
                if(list[1].bitTotal()>1)qDebug() << "!!! ADDER_PRIM sizeof(co)>1 !!!";
                rv+=" @ "+list[1].print(false);
            }
            rv+=" <= "+list[2].print(false)+" + "+list[3].print(false);
            if(list[4].bitCode!=0)rv+=" + "+list[4].print(false);
            return rv+";\n";
        }
    }
    else if(call->name=="AOR1" || call->name=="AOR1P")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isIdentical(list) && list.size()==4)
        {
            AString rv=mono+list[0].print(false) + " <= "+list[3].print(false)
                    +" | ("+list[1].print(false)+" & "+list[2].print(false)+")";
            return rv+";\n";
        }
    }
    else if(call->name=="ANR1" || call->name=="ANR1P")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isIdentical(list) && list.size()==4)
        {
            AString rv=mono+list[0].print(false) + " <= ~("+list[3].print(false)
                    +" | ("+list[1].print(false)+" & "+list[2].print(false)+"))";
            return rv+";\n";
        }
    }
    else if(call->name=="OND1")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isIdentical(list) && list.size()==4)
        {
            AString rv=mono+list[0].print(false) + " <= ~("+list[3].print(false)
                    +" & ("+list[1].print(false)+" | "+list[2].print(false)+"))";
            return rv+";\n";
        }
    }
    else if(call->name=="OAN1P" || call->name=="OAN1")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        if(isIdentical(list) && list.size()==4)
        {
            AString rv=mono+list[0].print(false) + " <= "+list[3].print(false)
                    +" & ("+list[1].print(false)+" | "+list[2].print(false)+")";
            return rv+";\n";
        }
    }
    else if(call->name=="FLIPFLOP_PRIM")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size())
        {
            AString rv;
            int i;
            for(i=0;i<gr.size();i++)
                if(gr[i].size()!=1)break;
            if(i==gr.size())
            {
                AString pre;
                if(gr[4][0].bitCode==1)pre="0";
                else if(gr[4][0].bitCode==0)pre="-1";
                else pre="-(~"+gr[4][0].print(false)+")";
                if(!gr[0][0].unconnected)
                {
                    rv+=mono+gr[0][0].print(false)+"("+gr[3][0].print(false)+") <= ";
                    if(gr[5][0].bitCode!=1)
                            rv+=gr[5][0].print(false)+" ? "+gr[2][0].print(false)+" : "+pre+";\n";
                    else rv+=gr[2][0].print(false)+";\n";
                    if(!gr[1][0].unconnected)
                        rv+=mono+gr[1][0].print(false)+" <= ~"+gr[0][0].print(false)+";\n";
                    return rv;
                }
                else if(!gr[1][0].unconnected)
                {
                    rv+=mono+gr[1][0].print(false)+"("+gr[3][0].print(false)+") <= ~(";
                    if(gr[5][0].bitCode!=1)
                            rv+=gr[5][0].print(false)+" ? "+gr[2][0].print(false)+" : "+pre+");\n";
                    else rv+=gr[2][0].print(false)+");\n";
                    return rv;
                }
            }
        }
    }
    else if(call->name=="FLIPFLOPS_PRIM")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size())
        {
            AString rv;
            int i;
            for(i=0;i<gr.size();i++)
                if(gr[i].size()!=1)break;
            if(i==gr.size())
            {
                AString pre;
                if(gr[4][0].bitCode==1)pre="0";
                else if(gr[4][0].bitCode==0)pre="-1";
                else pre="-(~"+gr[4][0].print(false)+")";

                AString main=gr[7][0].print(false)+" ? "+gr[6][0].print(false)+" : "+gr[2][0].print(false);

                if(!gr[0][0].unconnected)
                {
                    rv+=mono+gr[0][0].print(false)+"("+gr[3][0].print(false)+") <= ";
                    if(gr[5][0].bitCode!=1)
                            rv+=gr[5][0].print(false)+" ? ("+main+") : "+pre+";\n";
                    else rv+=main+";\n";
                    if(!gr[1][0].unconnected)
                        rv+=mono+gr[1][0].print(false)+" <= ~"+gr[0][0].print(false)+";\n";
                    return rv;
                }
                else if(!gr[1][0].unconnected)
                {
                    rv+=mono+gr[1][0].print(false)+"("+gr[3][0].print(false)+") <= ~(";
                    if(gr[5][0].bitCode!=1)
                            rv+=gr[5][0].print(false)+" ? ("+main+") : "+pre+");\n";
                    else rv+=main+");\n";
                    return rv;
                }
            }
        }
    }
    else if(call->name=="LATCH_PRIM")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size())
        {
            AString rv;
            int i;
            for(i=0;i<gr.size();i++)
                if(gr[i].size()!=1)break;
            if(i==gr.size())
            {
                AString pre;
                if(gr[4][0].bitCode==1)pre="0";
                else if(gr[4][0].bitCode==0)pre="-1";
                else pre="-(~"+gr[4][0].print(false)+")";

                if(!gr[0][0].unconnected)
                {
                    AString main=gr[3][0].print(false)+" ? "+gr[2][0].print(false)+" : "+gr[0][0].print(false);
                    rv+=mono+gr[0][0].print(false)+"() <= ";
                    if(gr[5][0].bitCode!=1)
                            rv+=gr[5][0].print(false)+" ? ("+main+") : "+pre+";\n";
                    else rv+=main+";\n";
                    if(!gr[1][0].unconnected)
                        rv+=mono+gr[1][0].print(false)+" <= ~"+gr[0][0].print(false)+";\n";
                    return rv;
                }
                else if(!gr[1][0].unconnected)
                {
                    AString main=gr[3][0].print(false)+" ? "+gr[2][0].print(false)+" : ~"+gr[1][0].print(false);
                    rv+=mono+gr[1][0].print(false)+"() <= ~(";
                    if(gr[5][0].bitCode!=1)
                            rv+=gr[5][0].print(false)+" ? ("+main+") : "+pre+");\n";
                    else rv+=main+");\n";
                    return rv;
                }
            }
        }
    }
    else if(call->name=="MUX4_PRIM")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==7)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            if(gr[6].size()!=1 || gr[6][0].bitCode!=0)
                rv+=printSA(gr[6])+" ? 0 : ";
            rv+="{"+printSA(gr[1])+","+printSA(gr[2])+","+printSA(gr[3])+","+printSA(gr[4])+"}["
                    +printSA(gr[5])+"];\n";
            return rv;
        }
    }
    else if(call->name=="MX6")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==3 && list.size()==10)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            rv+="{"+printList(list,1,6)+","+list[5].print(false)
                    +","+list[6].print(false)+"}["
                    +printSA(gr[2])+"];\n";
            return rv;
        }
    }
    else if(call->name=="MUX8_PRIM")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==4 && list.size()==13)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            if(gr[3].size()!=1 || gr[3][0].bitCode!=0)
                rv+=printSA(gr[3])+" ? 0 : ";
            rv+="{"+printList(list,1,8)+"}["
                    +printSA(gr[2])+"];\n";
            return rv;
        }
    }
    else if(call->name=="MUX2_PRIM")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==5)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            if(gr[4].size()!=1 || gr[4][0].bitCode!=0)
                rv+=printSA(gr[4])+" ? 0 : ";
            rv+="{"+printSA(gr[1])+","+printSA(gr[2])+"}["
                    +printSA(gr[3])+"];\n";
            return rv;
        }
    }
    else if(call->name=="MX12B")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==3 && list.size()==17)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            rv+="{"+printList(list,1,12)+",0,0,0,0"+"}["
                    +printSA(gr[2])+"];\n";
            return rv;
        }
    }
    else if(call->name=="MXI2P")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==4)
        {
            AString rv=mono+printSA(gr[0])+" <= ~(";
            rv+=printSA(gr[3])+" ? "+printSA(gr[2])+" : "
                    +printSA(gr[1])+");\n";
            return rv;
        }
    }
    else if(call->name=="ADDBMUX")
    {
        ATArray<NetListFileArgument> list=call->genSepList();
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==9 && list.size()==10)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            rv+="{"+printSA(gr[3])+","+printSA(gr[5])+","+printSA(gr[7])+","+printSA(gr[3])+"}["
                    +printSA(gr[2])+"];\n";
            rv+=mono+printSA(gr[1])+" <= ";
            rv+="{"+printSA(gr[4])+","+printSA(gr[6])+","+printSA(gr[8])+","+printSA(gr[4])+"}["
                    +printSA(gr[2])+"];\n";
            return rv;
        }
    }
    else if(call->name=="D38H" || call->name=="DECH38")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==4)
        {
            AString rv=mono+printSA(gr[0])+" <= 1 << ";
            ATArray<NetListFileArgument> list;
            list.append(gr[1]).append(gr[2]).append(gr[3]);
            list=unitingArgs(list);
            rv+=printSA(list)+";\n";
            return rv;
        }
    }
    else if(call->name=="D24H")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==3)
        {
            AString rv=mono+printSA(gr[0])+" <= 1 << ";
            ATArray<NetListFileArgument> list;
            list.append(gr[1]).append(gr[2]);
            list=unitingArgs(list);
            rv+=printSA(list)+";\n";
            return rv;
        }
    }
    else if(call->name=="D416G2L")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==6)
        {
            AString rv=mono+printSA(gr[0])+" <= ~((1 << ";
            ATArray<NetListFileArgument> list;
            list.append(gr[1]).append(gr[2]).append(gr[3]).append(gr[4]);
            list=unitingArgs(list);
            rv+=printSA(list)+") & ("+printSA(gr[5])+"?0:-1));\n";
            return rv;
        }
    }
    else if(call->name=="D416GH")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==6)
        {
            AString rv=mono+printSA(gr[0])+" <= (1 << ";
            ATArray<NetListFileArgument> list;
            list.append(gr[1]).append(gr[2]).append(gr[3]).append(gr[4]);
            list=unitingArgs(list);
            rv+=printSA(list)+") & ("+printSA(gr[5])+"?0:-1);\n";
            return rv;
        }
    }
    else if(call->name=="DECL38E")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==5)
        {
            AString rv=mono+printSA(gr[0])+" <= ~("+printSA(gr[4])+" << ";
            ATArray<NetListFileArgument> list;
            list.append(gr[1]).append(gr[2]).append(gr[3]);
            list=unitingArgs(list);
            rv+=printSA(list)+");\n";
            return rv;
        }
    }
    else if(call->name=="DECH38EL" || call->name=="D38GH")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==5)
        {
            AString rv=mono+printSA(gr[0])+" <= ("+printSA(gr[4])+"^1) << ";
            ATArray<NetListFileArgument> list;
            list.append(gr[1]).append(gr[2]).append(gr[3]);
            list=unitingArgs(list);
            rv+=printSA(list)+";\n";
            return rv;
        }
    }
    else if(call->name=="MXI2")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==4)
        {
            AString rv=mono+printSA(gr[0])+" <= ~(";
            rv+="{"+printSA(gr[1])+","+printSA(gr[2])+"}["
                    +printSA(gr[3])+"]);\n";
            return rv;
        }
    }
    else if(call->name=="CMP6")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==3)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            rv+="("+printSA(gr[1])+" == "+printSA(gr[2])+")?1:0;\n";
            return rv;
        }
    }
    else if(call->name=="CMP6I")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==3)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            rv+="("+printSA(gr[1])+" == "+printSA(gr[2])+")?0:1;\n";
            return rv;
        }
    }
    else if(call->name=="AOR2")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==5)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            rv+="("+printSA(gr[1])+" & "+printSA(gr[2])+") | ("+printSA(gr[3])+" & "+printSA(gr[4])+");\n";
            return rv;
        }
    }
    else if(call->name=="TBEN")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==4)
        {
            AString rv=mono+printSA(gr[1])+" <= ";
            rv+=printSA(gr[2])+" | "+printSA(gr[3])+";\n";
            rv+=mono+printSA(gr[0])+" <= ";
            rv+="~( (~"+printSA(gr[2])+") | "+printSA(gr[3])+");\n";
            return rv;
        }
    }
    else if(call->name=="TBENW")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==3)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            rv+="~( (~"+printSA(gr[1])+") | "+printSA(gr[2])+");\n";
            return rv;
        }
    }
    else if(call->name=="FA23")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==3)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            rv+=printSA(gr[1])+" + "+printSA(gr[2])+";\n";
            return rv;
        }
    }
    else if(call->name=="FA32_INT")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==7)
        {
            AString rv=mono+printSA(gr[0])+"@"+printSA(gr[1])+" <= ";
            rv+=printSA(gr[5])+" + "+printSA(gr[6])+" + "+printSA(gr[4])+";\n";
            rv+=mono+printSA(gr[2])+"@"+printSA(gr[3])+" <= 0;\n";
            return rv;
        }
    }
    else if(call->name=="ENP")
    {
        ATArray<ATArray<NetListFileArgument> > gr=getCallOrdered(call);
        if(gr.size()==3)
        {
            AString rv=mono+printSA(gr[0])+" <= ";
            rv+="~("+printSA(gr[1])+" ^ "+printSA(gr[2])+");\n";
            return rv;
        }
    }
    return mono+call->print()+"\n";
}
