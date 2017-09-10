#include "netlist_reader_def.h"

ATArray<NetListFileArgument> NetListFileDef::genSepList()
{
    ATArray<NetListFileArgument> rv;
    for(int i=0;i<args.size();i++)
    {
        NetListFileArgument tmp=args.value(i);
        if(args.value_ref(i).first>=0)
        {
            for(int j=args.value_ref(i).first;j<=args.value_ref(i).last;j++)
            {
                tmp.first=tmp.last=j;
                rv.append(tmp);
            }
        }
        else
        {
            rv.append(tmp);
        }
    }
    return rv;
}
