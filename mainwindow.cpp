#include "mainwindow.h"
#include "apreprocessor.h"
#include "netlist_reader.h"
#include "glob/axobject.h"

//4-bit magnitude comparator, based on 'LS85
void MAG_4_imp(     int &gto,
                    int &eqo,
                    int &lto,
                    int gti,
                    int eqi,
                    int lti,
                    int aa,//[0..3],
                    int bb//[0..3]
                )
{
    if(aa<bb)gto=0;
    else gto=1;
    if(aa>bb)lto=0;
    else lto=1;
    if(aa==bb)
    {
        eqo=1&eqi;
        gto=((~lti)&(~eqi))&1;
        lto=((~gti)&(~eqi))&1;
    }
    else
    {
        eqo=0;
    }
}

void MAG_4(         int &gto,
                    int &eqo,
                    int &lto,
                    int gti,
                    int eqi,
                    int lti,
                    int aa,//[0..3],
                    int bb//[0..3]
                )
{
    int aab_n = ~(aa & bb);
    int aoia_ = ~((bb & aab_n) | (aab_n & aa));
    int aoib_ = aoia_;
    int a[4]={aa&1,(aa>>1)&1,(aa>>2)&1,(aa>>3)&1};
    int aab[4]={aab_n&1,(aab_n>>1)&1,(aab_n>>2)&1,(aab_n>>3)&1};
    int b[4]={bb&1,(bb>>1)&1,(bb>>2)&1,(bb>>3)&1};
    int aoia[4]={aoia_&1,(aoia_>>1)&1,(aoia_>>2)&1,(aoia_>>3)&1};
    int aoib[4]={aoib_&1,(aoib_>>1)&1,(aoib_>>2)&1,(aoib_>>3)&1};
    int ltot[4],gtot[4];


    ltot[0] = ~(a[3] & aab[3]);
    ltot[1] = ~(a[2] & aab[2] & aoia[3]);
    ltot[2] = ~(a[1] & aab[1] & aoia[2] & aoia[3]);
    ltot[3] = ~(a[0] & aab[0] & aoia[1] & aoia[2] & aoia[3]);
    ltot[4] = ~(aoia[0] & aoia[1] & aoia[2] & aoia[3] & gti);
    ltot[5] = ~(aoia[0] & aoia[1] & aoia[2] & aoia[3] & eqi);
    ltot[6] = ltot[0] & ltot[1] & ltot[2] & ltot[3];
    lto = (ltot[4] & ltot[5] & ltot[6])&1;
    int eqot = aoia[0] & aoia[1] & aoia[2] & aoia[3];
    eqo = (eqot & eqi)&1;
    gtot[0] = ~(b[3] & aab[3]);
    gtot[1] = ~(b[2] & aab[2] & aoib[3]);
    gtot[2] = ~(b[1] & aab[1] & aoib[2] & aoib[3]);
    gtot[3] = ~(b[0] & aab[0] & aoib[1] & aoib[2] & aoib[3]);
    gtot[4] = ~(aoib[0] & aoib[1] & aoib[2] & aoib[3] & lti);
    gtot[5] = ~(aoib[0] & aoib[1] & aoib[2] & aoib[3] & eqi);
    gtot[6] = gtot[0] & gtot[1] & gtot[2] & gtot[3];
    gto = (gtot[4] & gtot[5] & gtot[6])&1;
}

void MAG_16(
    int &gt,
    int &eq,
    int &lt,
    int a,//[0..15]
    int b//[0..15]
)
{
    int gtx[3],eqx[3],ltx[3];
    MAG_4(gtx[0],eqx[0],ltx[0],0,1,0,a&0xf,b&0xf);
    MAG_4(gtx[1],eqx[1],ltx[1],(a>>4)&1,0,(b>>4)&1,(a>>5)&0xf,(b>>5)&0xf);
    MAG_4(gtx[2],eqx[2],ltx[2],(a>>9)&1,0,(b>>9)&1,(a>>10)&0xf,(b>>10)&0xf);
    MAG_4(gt,eq,lt,gtx[0],eqx[0],ltx[0],
            (gtx[1]&1)|((gtx[2]<<1)&2)|(((a>>14)&3)<<2),
            (ltx[1]&1)|((ltx[2]<<1)&2)|(((b>>14)&3)<<2));
}

void MAG_16_imp(
        int &gt,
        int &eq,
        int &lt,
        int a,//[0..15]
        int b//[0..15]
    )
{
    if(a>b)gt=1;
    else gt=0;
    if(a<b)lt=1;
    else lt=0;
    if(a==b)eq=1;
    else eq=0;
}

void MAG_16_test()
{
    for(uint64 i=0; i<0x100000000; i++)
    {
        int g1,e1,l1;
        int g2,e2,l2;
        int aa=i&0xffff;
        int bb=(i>>16)&0xffff;
        MAG_16(g1,e1,l1,aa,bb);
        MAG_16_imp(g2,e2,l2,aa,bb);
        if(g1!=g1 || e1!=e2 || l1!=l2)
        {
            qDebug() << i << g1 << g2 << e1 << e2 << l1 << l2 << "~ " << aa << bb;
            break;
        }
    }
}

void MAG_4_test()
{
    for(int i=0;i<2048;i++)
    {
        int aa=i&15;
        int bb=(i>>4)&15;
        int gti=(i>>8)&1;
        int eqi=(i>>9)&1;
        int lti=(i>>10)&1;
        int g1,e1,l1;
        int g2,e2,l2;
        MAG_4(g1,e1,l1,gti,eqi,lti,aa,bb);
        MAG_4_imp(g2,e2,l2,gti,eqi,lti,aa,bb);
        if(g1!=g1 || e1!=e2 || l1!=l2)
        {
            qDebug() << g1 << g2 << e1 << e2 << l1 << l2 << "~ " << aa << bb << gti << eqi << lti;
            break;
        }
    }
}

void CG4(       int &co0_,
                int &co1_,
                int &co2_,
                int &gf,
                int &pf,
                int cin/*IN*/,
                int gg,//[0..3]
                int pp//[0..3],
            )
{
    int g[4]={gg&1,(gg>>1)&1,(gg>>2)&1,(gg>>3)&1};
    int p[4]={pp&1,(pp>>1)&1,(pp>>2)&1,(pp>>3)&1};
    int co1t[2],co2t[3],gt[3];

    int cin_ = ~cin/*IN*/;
    int co0t = p[0] & cin_;

    co0_ = (~(g[0] | co0t))&1; //<<<

    co1t[0] = p[0] & p[1] & cin_;
    co1t[1] = p[1] & g[0];
    co1_ = (~(g[1] | co1t[0] | co1t[1]))&1; //<<<

    co2t[0] = p[0] & p[1] & p[2] & cin_;
    co2t[1] = p[1] & p[2] & g[0];
    co2t[2] = p[2] & g[1];
    co2_ = (~(g[2] | co2t[0] | co2t[1] | co2t[2]))&1; //<<<

    pf = (p[0] & p[1] & p[2] & p[3])&1; //<<<

    gt[0] = g[0] & p[1] & p[2] & p[3];
    gt[1] = g[1] & p[2] & p[3];
    gt[2] = g[2] & p[3];
    gf = (g[3] | gt[0] | gt[1] | gt[2])&1; //<<<
}

void FA4CS(     int &s,//[0..3]
                int &co_,
                int &co0_,
                int &co1_,
                int &g,
                int &p,
                int cin,
                int ci0n,
                int ci1n,
                int a,//[0..3]
                int b//[0..3]
                )
{
    int cit = cin | ci1n;
    int ci = ~(cit & ci0n);
    co_ = (~ci/*IN*/)&1; //<<<
    int pp = a ^ b;
    int gg = a & b;
    int gm[4]={gg&1,(gg>>1)&1,(gg>>2)&1,(gg>>3)&1};
    int pm[4]={pp&1,(pp>>1)&1,(pp>>2)&1,(pp>>3)&1};

    int gt[3];

    gt[0] = gm[0] & pm[1] & pm[2] & pm[3];
    gt[1] = gm[1] & pm[2] & pm[3];
    gt[2] = gm[2] & pm[3];
    g = (gm[3] | gt[0] | gt[1] | gt[2])&1; //<<<
    p = (pm[0] & pm[1] & pm[2] & pm[3])&1; //<<<
    co0_ = (~g/*IN*/)&1; //<<<
    int p_ = ~p/*IN*/;
    co1_ = (co0_ & p_)&1; //<<<
    s = (a/*IN*/ + b/*IN*/ + (ci&1)/*IN*/)&0xf; //<<<
}

void ADD16SAT(  int &r,//'16,
                int &co,
                int a,//16,
                int b,//16,
                int cin,
                int sat,
                int eightbit,
                int hicinh
            )
{
    int eightbit_ = ~eightbit/*IN*/;
    int cin_ = ~cin/*IN*/;

    int q_0_3,q_4_7,q_8_11,q_12_15;
    int co_[4],co0_[4],co1_[4],g[4],p[4];
    int cin_1=0,cin_2=0,cin_3=0;
    FA4CS(q_0_3,co_[0],co0_[0],co1_[0],g[0],p[0],cin_/*IN*/,1,0,a&0xf,b&0xf);
    FA4CS(q_4_7,co_[1],co0_[1],co1_[1],g[1],p[1],cin_1/*IN*/,1,0,(a>>4)&0xf,(b>>4)&0xf);
    FA4CS(q_8_11,co_[2],co0_[2],co1_[2],g[2],p[2],cin_2/*IN*/,1,0,(a>>8)&0xf,(b>>8)&0xf);
    FA4CS(q_12_15,co_[3],co0_[3],co1_[3],g[3],p[3],cin_3/*IN*/,1,0,(a>>12)&0xf,(b>>12)&0xf);

    int gg=g[0]|(g[1]<<1)|(g[2]<<2)|(g[3]<<3);
    int pp=p[0]|(p[1]<<1)|(p[2]<<2)|(p[3]<<3);
    int cint_2,cint_3,_g,_p;
    CG4(cin_1,cint_2,cint_3,_g,_p,cin_,gg/*IN*/,pp/*IN*/);
    cin_2 = eightbit/*IN*/ | cint_2/*IN*/;
    cin_3 = hicinh/*IN*/ | cint_3/*IN*/;

    FA4CS(q_0_3,co_[0],co0_[0],co1_[0],g[0],p[0],cin_/*IN*/,1,0,a&0xf,b&0xf);
    FA4CS(q_4_7,co_[1],co0_[1],co1_[1],g[1],p[1],cin_1/*IN*/,1,0,(a>>4)&0xf,(b>>4)&0xf);
    FA4CS(q_8_11,co_[2],co0_[2],co1_[2],g[2],p[2],cin_2/*IN*/,1,0,(a>>8)&0xf,(b>>8)&0xf);
    FA4CS(q_12_15,co_[3],co0_[3],co1_[3],g[3],p[3],cin_3/*IN*/,1,0,(a>>12)&0xf,(b>>12)&0xf);
    gg=g[0]|(g[1]<<1)|(g[2]<<2)|(g[3]<<3);
    pp=p[0]|(p[1]<<1)|(p[2]<<2)|(p[3]<<3);
    CG4(cin_1,cint_2,cint_3,_g,_p,cin_,gg/*IN*/,pp/*IN*/);
    cin_2 = eightbit/*IN*/ | cint_2/*IN*/;
    cin_3 = hicinh/*IN*/ | cint_3/*IN*/;

    int carry = ~cint_2/*IN*/;
    co = (_g/*IN*/ | (_p/*IN*/ & cin/*IN*/))&1;
    int btop = (eightbit&1)/*IN*/ ? ((b>>7)&1)/*IN*/ : ((b>>15)&1)/*IN*/;
    int ctopb = (eightbit&1)/*IN*/ ? carry/*IN*/ : co/*IN*/;
    int satt = btop/*IN*/ ^ ctopb/*IN*/;
    int saturateb = sat/*IN*/ & satt/*IN*/;
    int hisaturate = eightbit_/*IN*/ & saturateb/*IN*/;
    r = ((saturateb&1)/*IN*/ ? (-(ctopb&1)/*IN*/) : (q_0_3|(q_4_7<<4))/*IN*/)&0xff; //<<<
    r |= (((hisaturate&1)/*IN*/ ? (-(ctopb&1)/*IN*/) : (q_8_11|(q_12_15<<4)))<<8)&0xff00;
}

void ADD16SAT_imp(  uint16 &r,
                int &co,
                int a,//16,
                int b,//16,
                int cin,
                int sat,
                int eightbit,
                int hicinh
            )
{
    if(hicinh && !eightbit)
    {
        r=(a+b+cin)&0xfff;
        r|=(a&(~0xfff))+(b&(~0xfff));
    }
    else
    {
        if(eightbit)
        {
            int c12=(((a&0xfff)+(b&0xfff)+cin)>>12)&1;
            r=(a+b+cin)&0xff;
            int c8=(((a&0xff)+(b&0xff)+cin)>>8)&1;
            int nflag=(((c12&(~hicinh)))&1)<<12;
            r|=((a&(~0xff))+(b&(~0xff)))&0xf00;
            r|=((a&(~0xfff))+(b&(~0xfff))+nflag)&0xf000;
            if(sat & (c8^(b>>7)))
            {
                r=(r&0xff00)|((-c8)&0xff);
            }
        }
        else
        {
            r=a+b+cin;
        }
    }
    co=((a+b+cin)>>16)&1;

    if( (sat & (co^(b>>15))) && !eightbit )
    {
        r=-co;
    }
}

void ADD16SAT_test()
{
    for(uint64 i=0;i<0x1000000000;i++)
    {
        int r1,co1;
        ADD16SAT(r1,co1,i&0xffff,(i>>16)&0xffff,(i>>32)&1,(i>>33)&1,(i>>34)&1,(i>>35)&1);
        int co2;
        uint16 r2;
        ADD16SAT_imp(r2,co2,i&0xffff,(i>>16)&0xffff,(i>>32)&1,(i>>33)&1,(i>>34)&1,(i>>35)&1);
        if(r2!=r1 || co2!=co1)
        {
            qDebug() << AString::fromInt(r1,16) << AString::fromInt(r2,16) << co1 << co2 << "~ "
                     << AString::fromInt(i&0xffff,16) << AString::fromInt((i>>16)&0xffff,16)
                     << ((i>>32)&1) << ((i>>33)&1) << ((i>>34)&1) << ((i>>35)&1);
            break;
        }
    }
}

void FAS16_S(	int &q,//[0..15]
                int &cout_,
                int as,
                int cin_,
                int a,//[0..15]
                int b//[0..15]
            )
{
    int q_0_3,q_4_7,q_8_11,q_12_15;
    int co_[4],co0_[4],co1_[4],g[4],p[4];

    int cin_1=0,cin_2=0,cin_3=0;
    FA4CS(q_0_3,co_[0],co0_[0],co1_[0],g[0],p[0],cin_/*IN*/,1,0,a&0xf,b&0xf);
    FA4CS(q_4_7,co_[1],co0_[1],co1_[1],g[1],p[1],cin_1/*IN*/,1,0,(a>>4)&0xf,(b>>4)&0xf);
    FA4CS(q_8_11,co_[2],co0_[2],co1_[2],g[2],p[2],cin_2/*IN*/,1,0,(a>>8)&0xf,(b>>8)&0xf);
    FA4CS(q_12_15,co_[3],co0_[3],co1_[3],g[3],p[3],cin_3/*IN*/,1,0,(a>>12)&0xf,(b>>12)&0xf);
    int _g,_p;
    int gg=g[0]|(g[1]<<1)|(g[2]<<2)|(g[3]<<3);
    int pp=p[0]|(p[1]<<1)|(p[2]<<2)|(p[3]<<3);
    CG4(cin_1,cin_2,cin_3,_g,_p,cin_,gg/*IN*/,pp/*IN*/);

    int cin = (~cin_)&1/*IN*/;
    cout_ = (~(_g/*IN*/ | (cin/*IN*/ & _p/*IN*/)))&1;

    FA4CS(q_0_3,co_[0],co0_[0],co1_[0],g[0],p[0],cin_/*IN*/,1,0,a&0xf,b&0xf);
    FA4CS(q_4_7,co_[1],co0_[1],co1_[1],g[1],p[1],cin_1/*IN*/,1,0,(a>>4)&0xf,(b>>4)&0xf);
    FA4CS(q_8_11,co_[2],co0_[2],co1_[2],g[2],p[2],cin_2/*IN*/,1,0,(a>>8)&0xf,(b>>8)&0xf);
    FA4CS(q_12_15,co_[3],co0_[3],co1_[3],g[3],p[3],cin_3/*IN*/,1,0,(a>>12)&0xf,(b>>12)&0xf);
    gg=g[0]|(g[1]<<1)|(g[2]<<2)|(g[3]<<3);
    pp=p[0]|(p[1]<<1)|(p[2]<<2)|(p[3]<<3);
    CG4(cin_1,cin_2,cin_3,_g,_p,cin_,gg/*IN*/,pp/*IN*/);

    cin/*OUT*/ = (~cin_)&1/*IN*/;
    cout_/*OUT*/ = (~(_g/*IN*/ | (cin/*IN*/ & _p/*IN*/)))&1;

    q=q_0_3|(q_4_7<<4)|(q_8_11<<8)|(q_12_15<<12);
}

void FAS16_S_imp(	uint16 &q,//[0..15]
                int &cout_,
                int as,
                int cin_,
                int a,//[0..15]
                int b//[0..15]
            )
{
    q=(a+b+((~cin_)&1))&0xffff;
    cout_=(~((a+b+((~cin_)&1))>>16))&1;
}

void FAS16_S_test()
{
    for(uint64 i=0;i<0x200000000;i++)
    {
        int r1,co1;
        FAS16_S(r1,co1,0,(i>>32)&1,i&0xffff,(i>>16)&0xffff);
        uint16 r2;
        int co2;
        FAS16_S_imp(r2,co2,0,(i>>32)&1,i&0xffff,(i>>16)&0xffff);
        if(r2!=r1 || co2!=co1)
        {
            qDebug() << AString::fromInt(r1,16) << AString::fromInt(r2,16) << co1 << co2 << "~ "
                     << AString::fromInt(i&0xffff,16) << AString::fromInt((i>>16)&0xffff,16)
                     << ((i>>32)&1);
            break;
        }
    }
}

void FA332_imp(     uint32 &s,//[0..31]
                int &co,//[0..1]
                int ci,//[0..1]
                uint32 a,//[0..32]
                uint32 b,//[0..32]
                uint32 c//[0..32]
            )
{
    uint64 r0=(uint64)a+(uint64)b+(ci&1);
    uint64 r1=(uint64)c+(r0&0xffffffff)+((ci>>1)&1);

    s=r1;
    co=((r0>>32)&1)|((r1>>31)&2);
}

void MAG4_imp(
                int &gto,
                int &eqo,
                int &lto,
                int gti,
                int eqi,
                int lti,
                int a,//[3..0]
                int b//[3..0]
            )
{
    if(a<b)lto=1;
    else lto=0;
    if(a==b)eqo=eqi;
    else eqo=0;
    if(a>b)gto=1;
    else gto=0;

    if(!eqi){lto=lti;gto=gti;}
}

void MAG12_imp(
        int &agbr,//	=	vgy/*IO*/,
        int &aebr,//	=	?/*IO*/,
        int &albr,//	=	vly/*IO*/,
        int a,//[0..11]	=	vc[0..10]/*IN*/ @ 0,
        int b//[0..11]	=	ypos[0..10]/*IN*/ @ 0,
    )
{
    if(a<b)albr=1;
    else albr=0;
    if(a==b)aebr=1;
    else aebr=0;
    if(a>b)agbr=1;
    else agbr=0;
}

void MAG12(
        int &agbr,//	=	vgy/*IO*/,
        int &aebr,//	=	?/*IO*/,
        int &albr,//	=	vly/*IO*/,
        int a,//[0..11]	=	vc[0..10]/*IN*/ @ 0,
        int b//[0..11]	=	ypos[0..10]/*IN*/ @ 0,
    )
{
    int agb[2],aeb[2],alb[2];
    MAG4_imp(agb[0],aeb[0],alb[0],0,1,0,a>>8,b>>8);
    MAG4_imp(agb[1],aeb[1],alb[1],agb[0],aeb[0],alb[0],(a>>4)&15,(b>>4)&15);
    MAG4_imp(agbr,aebr,albr,agb[1],aeb[1],alb[1],a&15,b&15);
}

void MAG12_test()
{
    for(int i=0;i<0x1000000;i++)
    {
        uint16 m[2];
        for(int k=0;k<4;k++)((uint8*)m)[k]=qrand();

        int g[2],e[2],l[2];
        MAG12(g[0],e[0],l[0],m[0]&0xfff,m[1]&0xfff);
        MAG12_imp(g[1],e[1],l[1],m[0]&0xfff,m[1]&0xfff);

        if(g[0]!=g[1] || e[0]!=e[1] || l[0]!=l[1])
        {
            qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
            return;
        }
    }
}

void MAG_15(	int &gto,
                int &eqo,
                int &lto,
                int a,//[0..14]
                int b//[0..14]
            )
{
    int gt[3],eq[3],lt[3];
    MAG_4_imp(gt[0]/*OUT*/,eq[0]/*OUT*/,lt[0]/*OUT*/,0,1,0,a&0xf/*IN*/,b&0xf/*IN*/);
    MAG_4_imp(gt[1]/*OUT*/,eq[1]/*OUT*/,lt[1]/*OUT*/,(a>>4)&1/*IN*/,0,(b>>4)&1/*IN*/,(a>>5)&0xf/*IN*/,(b>>5)&0xf/*IN*/);
    MAG_4_imp(gt[2]/*OUT*/,eq[2]/*OUT*/,lt[2]/*OUT*/,(a>>9)&1/*IN*/,0,(b>>9)&1/*IN*/,(a>>10)&0xf/*IN*/,(b>>10)&0xf/*IN*/);
    MAG_4_imp(gto/*OUT*/,eqo/*OUT*/,lto/*OUT*/,gt[0]/*IN*/,eq[0]/*IN*/,lt[0]/*IN*/,
            gt[1]|(gt[2]<<1)|(((a>>14)&1)<<2),
            lt[1]|(lt[2]<<1)|(((b>>14)&1)<<2));
}

void MAG_15_imp(	int &gto,
                int &eqo,
                int &lto,
                int a,//[0..14]
                int b//[0..14]
            )
{
    if(a<b)lto=1;
    else lto=0;
    if(a==b)eqo=1;
    else eqo=0;
    if(a>b)gto=1;
    else gto=0;
}

void MAG_15_test()
{
    for(uint64 i=0; i<0x40000000; i++)
    {
        int g1,e1,l1;
        int g2,e2,l2;
        int aa=i&0x7fff;
        int bb=(i>>15)&0x7fff;
        MAG_15(g1,e1,l1,aa,bb);
        MAG_15_imp(g2,e2,l2,aa,bb);
        if(g1!=g1 || e1!=e2 || l1!=l2)
        {
            qDebug() << i << g1 << g2 << e1 << e2 << l1 << l2 << "~ " << aa << bb;
            break;
        }
    }
}

void ADDRCOMP(
            int &a1_outside,
            int a1_x,//'16'
            int a1_y,//'16'
            int a1_win_x,//'15'
            int a1_win_y//'15'
        )
{
    int a1xgr,a1xeq,tmp,a1ygr,a1yeq;
    MAG_15_imp(a1xgr/*OUT*/,a1xeq/*OUT*/,tmp/*OUT*/,a1_x&0x7fff/*IN*/,a1_win_x/*IN*/);
    MAG_15_imp(a1ygr/*OUT*/,a1yeq/*OUT*/,tmp/*OUT*/,a1_y&0x7fff/*IN*/,a1_win_y/*IN*/);
    a1_outside = ((a1_x>>15) | a1xgr | a1xeq | (a1_y>>15) | a1ygr | a1yeq)&1;
}

void ADDRCOMP_imp(
            int &a1_outside,
            int a1_x,//'16'
            int a1_y,//'16'
            int a1_win_x,//'15'
            int a1_win_y//'15'
        )
{
    if(a1_x>=a1_win_x || a1_y>=a1_win_y)a1_outside=1;
    else a1_outside=0;
}

void ADDRCOMP_test()
{
    for(uint64 i=0; i<0x80000000; i++)
    {
        int r1;
        int r2;
        int aa=i&0xffff;
        int bb=(i>>16)&0x7fff;
        ADDRCOMP(r1,aa,0,bb,0);
        ADDRCOMP_imp(r2,aa,0,bb,0);
        if(r1!=r1)
        {
            qDebug() << i << r1 << r2 << "~ " << aa << bb;
            break;
        }
    }
}

void SRCSHIFT(
            uint32 &srcd_0,
            uint32 &srcd_1,
            int big_pix,
            uint32 srcd1lo,
            uint32 srcd1hi,
            uint32 srcd2lo,
            uint32 srcd2hi,
            int srcshift//[0..5]
        )
{
    uint64 shsrc_0_63=srcd2lo|((uint64)srcd2hi<<(uint64)32);
    uint64 shsrc_64_127=srcd1lo|((uint64)srcd1hi<<(uint64)32);

    int srcshift_ = (~srcshift)&0x3f;

    int besh_1_bec_1 = (srcshift_&1) + ((srcshift_>>1)&1);
    int besh_2 = ((besh_1_bec_1>>1)&1) ^ ((srcshift_>>2)&1);
    int besh_4_bec_4 = ((srcshift_>>3)&1) + ((srcshift_>>4)&1);
    int besh_5_bec_5 = ((besh_4_bec_4>>1)&1) + ((srcshift_>>5)&1);
    int besh_6 = ((~besh_5_bec_5)>>1)&1;

    int shift_0 = srcshift&1;
    int shift_1 = big_pix?(besh_1_bec_1&1):((srcshift>>1)&1);
    int shift_2 = big_pix?besh_2:((srcshift>>2)&1);
    int shiftt_3 = (srcshift>>3)&1;
    int shiftt_4 = big_pix?(besh_4_bec_4&1):((srcshift>>4)&1);
    int shiftt_5 = big_pix?(besh_5_bec_5&1):((srcshift>>5)&1);
    int shiftt_6 = besh_6 & big_pix;
    int shift_3 = shiftt_3;
    int shift_4 = shiftt_4;
    int shift_5 = shiftt_5;
    int shift_6 = shiftt_6;

    uint64 onep_8_63 = shift_6?(shsrc_64_127>>8):(shsrc_0_63>>8);
    uint64 onep_64_127 = shift_6?shsrc_0_63:shsrc_64_127;

    uint64 onel_40_63 = shift_5?(onep_8_63&0xffffff):(onep_8_63>>32);
    uint64 onel_64_127 = shift_5?((onep_8_63>>24)|(onep_64_127<<32)):onep_64_127;

    uint64 onew_56_63 = shift_4?(onel_40_63&0xff):(onel_40_63>>16);
    uint64 onew_64_127 = shift_4?((onel_40_63>>8)|(onel_64_127<<16)):onel_64_127;

    uint64 oneb_64_127 = shift_3?(onew_56_63|(onew_64_127<<8)):onew_64_127;

    uint64 onen_64_71 = shift_2 ? (((oneb_64_127&0xf)<<4)|((oneb_64_127>>4)&0xf)) : (oneb_64_127&0xff);

    uint64 onet_64_71 = (shift_1 ? ((onen_64_71>>6)|(onen_64_71<<2)) : onen_64_71)&0xff;

    uint64 oneo_64_71 = (shift_0 ? ((onet_64_71>>7)|(onet_64_71<<1)) : onet_64_71)&0xff;

    srcd_0 = oneo_64_71 | (oneb_64_127&(~0xff));
    srcd_1 = (oneb_64_127>>32);
}

void SRCSHIFT_imp(
            uint32 &srcd_0,
            uint32 &srcd_1,
            int big_pix,
            uint32 srcd1lo,
            uint32 srcd1hi,
            uint32 srcd2lo,
            uint32 srcd2hi,
            int srcshift//[0..5]
        )
{
    uint64 shsrc[2]; //LITTLE ENDIAN ONLY!!!
    shsrc[0]=srcd2lo|((uint64)srcd2hi<<(uint64)32);
    shsrc[1]=srcd1lo|((uint64)srcd1hi<<(uint64)32);

    if(big_pix)srcshift=((0x8-(srcshift&7))&7)|((0x80-(srcshift&0x78))&0x78);

    if(srcshift&0x40)__swap(shsrc[0],shsrc[1]);
    uint64 res=0;
    for(int i=0;i<8;i++)
        res|=((uint64)((uint8*)shsrc)[i+(((~srcshift)>>3)&7)+1])<<(i*8);

    srcd_0 = __rotl<uint8>(res&0xff,srcshift&7)|(res&(~0xff));
    srcd_1 = (res>>32);
}

void SRCSHIFT_test()
{
    uint32 d1, d2, d11, d22;
    for(uint64 i=0;i<128;i++)
    {
        for(int j=0;j<100000;j++)
        {
            uint32 m[4];
            for(int k=0;k<16;k++)((uint8*)m)[k]=qrand();

            SRCSHIFT(d1,d2,(i>>6)&1,m[0],m[1],m[2],m[3],i&63);
            SRCSHIFT_imp(d11,d22,(i>>6)&1,m[0],m[1],m[2],m[3],i&63);
            if(d1!=d11 || d2!=d22)
            {
                qDebug() << AString::fromInt(i&63,16) << AString::fromInt(d1,16) << AString::fromInt(d2,16) << ((i>>6)&1)
                            << AString::fromInt(d11,16) << AString::fromInt(d22,16)
                               << AString::fromInt(m[0],16) << AString::fromInt(m[1],16)
                                  << AString::fromInt(m[2],16) << AString::fromInt(m[3],16);
                break;
            }
        }
    }
}

void LFU(
            uint32 &lfu_0,
            uint32 &lfu_1,
            uint32 srcd_0,
            uint32 srcd_1,
            uint32 dstd_0,
            uint32 dstd_1,
            int lfc//[0..3]
        )
{
    uint32 srcd_0_n = ~srcd_0;
    uint32 srcd_1_n = ~srcd_1;
    uint32 dstd_0_n = ~dstd_0;
    uint32 dstd_1_n = ~dstd_1;

    uint32 lfunc_0=-(lfc&1);
    uint32 lfunc_1=-((lfc>>1)&1);
    uint32 lfunc_2=-((lfc>>2)&1);
    uint32 lfunc_3=-((lfc>>3)&1);

    uint32 trma_0 = ~(srcd_0_n & dstd_0_n & lfunc_0);
    uint32 trmb_0 = ~(srcd_0_n & dstd_0 & lfunc_1);
    uint32 trmc_0 = ~(srcd_0 & dstd_0_n & lfunc_2);
    uint32 trmd_0 = ~(srcd_0 & dstd_0 & lfunc_3);
    uint32 trma_1 = ~(srcd_1_n & dstd_1_n & lfunc_0);
    uint32 trmb_1 = ~(srcd_1_n & dstd_1 & lfunc_1);
    uint32 trmc_1 = ~(srcd_1 & dstd_1_n & lfunc_2);
    uint32 trmd_1 = ~(srcd_1 & dstd_1 & lfunc_3);

    lfu_0 = ~(trma_0 & trmb_0 & trmc_0 & trmd_0);
    lfu_1 = ~(trma_1 & trmb_1 & trmc_1 & trmd_1);
}

void ZEDSHIFT(
            uint32 &srczlo,
            uint32 &srczhi,
            uint32 srcz1lo,
            uint32 srcz1hi,
            uint32 srcz2lo,
            uint32 srcz2hi,
            int srcshift//[4..5]
        )
{
    uint16 srczw[7]={srcz1lo&0xffff,srcz1lo>>16,srcz1hi&0xffff,srcz1hi>>16,srcz2lo&0xffff,srcz2lo>>16,srcz2hi&0xffff};
    /// Align the source zed data with the destination
    srczlo = srczw[srcshift]|((uint32)srczw[srcshift+1]<<16);
    srczhi = srczw[srcshift+2]|((uint32)srczw[srcshift+3]<<16);
}

void CMP8_INT_imp(
                int &eqo,
                int a,//[0..7]
                int b//[0..7]
            )
{
    if(a==b)eqo=1;
    else eqo=0;
}

void DATACOMP(
            int &dcomp,//[0..7] data byte equal flags
            int cmpdst,// compare dest rather than source
            uint32 dstdlo,
            uint32 dstdhi,
            uint32 patdlo,
            uint32 patdhi,
            uint32 srcdlo,
            uint32 srcdhi
        )
{
    /// Select between source and dest
    uint32 tardlo = cmpdst?dstdlo:srcdlo;
    uint32 tardhi = cmpdst?dstdhi:srcdhi;
    uint8 patb[8]={patdlo&0xff,(patdlo>>8)&0xff,(patdlo>>16)&0xff,patdlo>>24,patdhi&0xff,(patdhi>>8)&0xff,(patdhi>>16)&0xff,patdhi>>24};
    uint8 tarb[8]={tardlo&0xff,(tardlo>>8)&0xff,(tardlo>>16)&0xff,tardlo>>24,tardhi&0xff,(tardhi>>8)&0xff,(tardhi>>16)&0xff,tardhi>>24};

    dcomp=0;
    for(int i=0;i<8;i++)
    {
        dcomp|=((patb[i]==tarb[i])?1:0)<<i;
    }
}

void ZEDCOMP(
            int &zcomp,//[0..3],
            uint32 srczplo,
            uint32 srczphi,
            uint32 dstzlo,
            uint32 dstzhi,
            int zmode//[0..2] Z comparator mode
        )
{
    /// Compare their values
    int zg[4],ze[4],zl[4];
    MAG_16_imp(zg[0]/*OUT*/,ze[0]/*OUT*/,zl[0]/*OUT*/,srczplo&0xffff/*IN*/,dstzlo&0xffff/*IN*/);
    MAG_16_imp(zg[1]/*OUT*/,ze[1]/*OUT*/,zl[1]/*OUT*/,srczplo>>16/*IN*/,dstzlo>>16/*IN*/);
    MAG_16_imp(zg[2]/*OUT*/,ze[2]/*OUT*/,zl[2]/*OUT*/,srczphi&0xffff/*IN*/,dstzhi&0xffff/*IN*/);
    MAG_16_imp(zg[3]/*OUT*/,ze[3]/*OUT*/,zl[3]/*OUT*/,srczphi>>16/*IN*/,dstzhi>>16/*IN*/);

    int _zl=zl[0]|(zl[1]<<1)|(zl[2]<<2)|(zl[3]<<3);
    int _ze=ze[0]|(ze[1]<<1)|(ze[2]<<2)|(ze[3]<<3);
    int _zg=zg[0]|(zg[1]<<1)|(zg[2]<<2)|(zg[3]<<3);

    int zlt = ~(_zl & (-(zmode&1)));
    int zet = ~(_ze & (-((zmode>>1)&1)));
    int zgt = ~(_zg & (-((zmode>>2)&1)));
    zcomp = (~(zlt & zet & zgt))&0xf;
}

void ZEDCOMP_imp(
            int &zcomp,//[0..3],
            uint32 srczplo,
            uint32 srczphi,
            uint32 dstzlo,
            uint32 dstzhi,
            int zmode//[0..2] Z comparator mode
        )
{
    zcomp=0;
    if(zmode&1)
    {
        if((srczplo&0xffff)<(dstzlo&0xffff))zcomp|=1;
        if((srczplo>>16)<(dstzlo>>16))zcomp|=2;
        if((srczphi&0xffff)<(dstzhi&0xffff))zcomp|=4;
        if((srczphi>>16)<(dstzhi>>16))zcomp|=8;
    }
    if(zmode&2)
    {
        if((srczplo&0xffff)==(dstzlo&0xffff))zcomp|=1;
        if((srczplo>>16)==(dstzlo>>16))zcomp|=2;
        if((srczphi&0xffff)==(dstzhi&0xffff))zcomp|=4;
        if((srczphi>>16)==(dstzhi>>16))zcomp|=8;
    }
    if(zmode&4)
    {
        if((srczplo&0xffff)>(dstzlo&0xffff))zcomp|=1;
        if((srczplo>>16)>(dstzlo>>16))zcomp|=2;
        if((srczphi&0xffff)>(dstzhi&0xffff))zcomp|=4;
        if((srczphi>>16)>(dstzhi>>16))zcomp|=8;
    }
}

void ZEDCOMP_test()
{
    int r1, r2;
    for(uint64 i=0;i<8;i++)
    {
        for(int j=0;j<100000;j++)
        {
            uint32 m[4];
            for(int k=0;k<16;k++)((uint8*)m)[k]=qrand();

            ZEDCOMP(r1,m[0],m[1],m[2],m[3],i);
            ZEDCOMP_imp(r2,m[0],m[1],m[2],m[3],i);
            if(r1!=r2)
            {
                qDebug() << AString::fromInt(i) << AString::fromInt(r1,16) << AString::fromInt(r2,16)
                               << AString::fromInt(m[0],16) << AString::fromInt(m[1],16)
                                  << AString::fromInt(m[2],16) << AString::fromInt(m[3],16);
                break;
            }
        }
    }
}

void DADDAMUX(
            uint16 &adda_0, uint16 &adda_1, uint16 &adda_2, uint16 &adda_3,
            uint32 dstd_0, uint32 dstd_1,
            uint32 srcd_0, uint32 srcd_1,
            uint32 patd_0, uint32 patd_1,
            uint32 srcz1_0, uint32 srcz1_1,
            uint32 srcz2_0, uint32 srcz2_1,
            int daddasel//[0..2] data adder input A selection
        )
{
    uint32 sello[4]={srcd_0,patd_0,srcz1_0,srcz2_0};
    uint32 selhi[4]={srcd_1,patd_1,srcz1_1,srcz2_1};
    uint32 addalo = (daddasel&4)?sello[daddasel&3]:dstd_0;
    uint32 addahi = (daddasel&4)?selhi[daddasel&3]:dstd_1;
    adda_0 = addalo;
    adda_1 = addalo>>16;
    adda_2 = addahi;
    adda_3 = addahi>>16;
}

void DADDBMUX(
            uint16 &addb_0, uint16 &addb_1, uint16 &addb_2, uint16 &addb_3,
            uint32 srcdlo,
            uint32 srcdhi,
            uint32 iinc,
            uint32 zinc,
            int daddbsel//[0..2]
        )
{
    uint16 word[4] = {iinc&0xffff, iinc>>16,zinc&0xffff,zinc>>16};
    addb_0 = (daddbsel&4)?word[daddbsel&3]:srcdlo;
    addb_1 = (daddbsel&4)?word[daddbsel&3]:(srcdlo>>16);
    addb_2 = (daddbsel&4)?word[daddbsel&3]:srcdhi;
    addb_3 = (daddbsel&4)?word[daddbsel&3]:(srcdhi>>16);
}

void LOCAL_MUX(
            uint32 &local_data_0, uint32 &local_data_1,	// data for addable registers
            uint32 &load_data_0, uint32 &load_data_1, // data for load only registers
            uint16 addq_0, uint32 addq_1, uint32 addq_2, uint32 addq_3,
            uint32 gpu_din,
            uint64 data,
            int blit_back, // blitter bus acknowledge
            int blit_breq,//[0..1] blitter bus request
            int daddq_sel
        )
{
    int blit_backb = (blit_breq&1) | (blit_breq>>1) | blit_back;

    load_data_0 = blit_backb?(data&0xffffffff):gpu_din;
    load_data_1 = blit_backb?(data>>32):gpu_din;

    local_data_0 = daddq_sel?(addq_0|((uint32)addq_1<<16)):load_data_0;
    local_data_1 = daddq_sel?(addq_2|((uint32)addq_3<<16)):load_data_1;
}

void DATA_MUX(
            uint64 &wdata,  //[0..63] BUS - co-processor rwrite data bus
            uint16 addq_0, uint16 addq_1, uint16 addq_2, uint16 addq_3,
            int big_pix,	// Pixel organisation is big-endian
            uint32 dstdlo,
            uint32 dstdhi,
            uint32 dstzlo,
            uint32 dstzhi,
            int data_sel,   //[0..1] source of write data
            int data_ena,   // enable write data onto read/write bus
            int dstart,     //[0..5] start of changed write data
            int dend,       //[0..5] end of changed write data
            int dbinh_,     //[0..7] byte oriented changed data inhibits
            uint32 lfu_0, uint32 lfu_1,
            uint32 patd_0, uint32 patd_1,
            int phrase_mode,// phrase write mode
            uint32 srczlo,
            uint32 srczhi
        )
{
    int phrase_mode_ = (~phrase_mode)&1;

    /// Generate an changed data mask
    int edis_ = dend?1:0;
    int e_coarse_ = ~(edis_ << ((dend>>3)&7));
    int e_coarse = (~e_coarse_)&1;
    int e_fine_ = ~(e_coarse << (dend&7));
    int s_coarse = 1 << ((dstart>>3)&7);
    int sfen_ = (~s_coarse)&1;
    int s_fine = (sfen_^1) << (dstart&7);

    int masktla = (s_coarse&1) & (e_coarse_&1);
    int maskt = (s_fine&1)| ((((e_coarse_>>1) & (masktla | (s_coarse>>1)))&1)<<8);

    int maskt_prev;

    do
    {
        maskt_prev=maskt;
        maskt = (maskt&(~0xfe)) | ((e_fine_ & ((maskt<<1) | s_fine))&0xfe);
        /// Produce a look-ahead on the ripple carry:
        ///masktla = s_coarse[0] . /e_coarse[0]
        maskt = (maskt&~(0x7e00)) | (((e_coarse_<<7) & ((maskt<<1) | (s_coarse<<7)))&0x7e00);
    }while(maskt!=maskt_prev);

    /// The bit terms are mirrored for big-endian pixels outside phrase
    ///mode.  The byte terms are mirrored for big-endian pixels in phrase
    ///mode.
    int mir_bit = phrase_mode_ & big_pix;
    int mir_byte = phrase_mode & big_pix;

    int masku = 0;
    if(mir_bit && !mir_byte)masku=__rev8(maskt);
    else if(!mir_bit && !mir_byte)masku=maskt&0xff;
    else if(!mir_bit && mir_byte)masku=(-((maskt>>14)&1))&0xff;

    if(!mir_byte) masku |= maskt&0x7f00;
    else masku|= ((maskt&1)<<14) |
            (((maskt>>8)&1)<<13) |
            (((maskt>>9)&1)<<12) |
            (((maskt>>10)&1)<<11) |
            (((maskt>>11)&1)<<10) |
            (((maskt>>12)&1)<<9) |
            (((maskt>>13)&1)<<8);

    /// The maskt terms define the area for changed data, but the byte
    ///inhibit terms can override these
    int mask=masku&(((-(dbinh_&1))&0xff)|((dbinh_<<7)&0x7f00));

    uint32 addql_0 = addq_0|(addq_1<<16);
    uint32 addql_1 = addq_2|(addq_3<<16);

    uint32 ddatlo_m[]={patd_0,lfu_0,addql_0,0};
    uint32 ddatlo = ddatlo_m[data_sel];
    uint32 ddathi_m[]={patd_1,lfu_1,addql_1,0};
    uint32 ddathi = ddathi_m[data_sel];

    int zed_sel = (data_sel & (data_sel>>1))&1;

    uint64 dat;
    if(zed_sel)dat=((srczlo&mask)|(dstzlo&(~mask)))&0xff;
    else dat=((ddatlo&mask)|(dstdlo&(~mask)))&0xff;

    uint32 ml[]={dstdlo, ddatlo, dstzlo, srczlo};
    uint32 mh[]={dstdhi, ddathi, dstzhi, srczhi};

    for(int i=0;i<3;i++)
    {
        dat |=  ml[((mask>>(8+i))&1)|(zed_sel<<1)] & (0xff<<(8+i*8));
    }
    for(int i=0;i<4;i++)
    {
        dat |=  ((uint64)(mh[((mask>>(11+i))&1)|(zed_sel<<1)] & (0xff<<(i*8))))<<32;
    }

    if(data_ena)wdata=dat;
}

void DATA_MUX_imp(
            uint64 &wdata,  //[0..63] BUS - co-processor rwrite data bus
            uint16 addq_0, uint16 addq_1, uint16 addq_2, uint16 addq_3,
            int big_pix,	// Pixel organisation is big-endian
            uint32 dstdlo,
            uint32 dstdhi,
            uint32 dstzlo,
            uint32 dstzhi,
            int data_sel,   //[0..1] source of write data
            int data_ena,   // enable write data onto read/write bus
            int dstart,     //[0..5] start of changed write data
            int dend,       //[0..5] end of changed write data
            int dbinh_,     //[0..7] byte oriented changed data inhibits
            uint32 lfu_0, uint32 lfu_1,
            uint32 patd_0, uint32 patd_1,
            int phrase_mode,// phrase write mode
            uint32 srczlo,
            uint32 srczhi
        )
{
    uint64 result=0, mask=0;
    uint64 src,dst=dstdlo|(((uint64)dstdhi)<<32);

    switch(data_sel)
    {
    case 0: src=patd_0|(((uint64)patd_1)<<32); break;
    case 1: src=lfu_0|(((uint64)lfu_1)<<32); break;
    case 2: src=addq_0|(((uint64)addq_1)<<16)|(((uint64)addq_2)<<32)|(((uint64)addq_3)<<48); break;
    default:
        src=srczlo|(((uint64)srczhi)<<32);
        dst=dstzlo|(((uint64)dstzhi)<<32);
        break;
    };

    int bit_start=(dstart>7)?8:(dstart&7);
    int bit_end=(dend>7 || !dend)?8:(dend&7);
    if(bit_end<bit_start){bit_end=8;}
    for(int i=bit_start;i<bit_end;i++)mask|=1<<i;

    int byte_start=dstart>>3;
    int byte_end=dend>>3;
    if((byte_end<byte_start) || !(dend))byte_end=8;
    if(!byte_start)byte_start++;
    for(int i=byte_start;i<byte_end;i++)mask|=((uint64)0xff)<<(i*8);

    if(big_pix)
    {
        if(phrase_mode)mask=__bswap64(mask&0xffffffffffffff00)|((mask&1)?((uint64)0xff<<56):0);
        else mask=(mask&0xffffffffffffff00)|__rev8(mask);
    }

    for(int i=0;i<8;i++)
    {
        if(!(dbinh_&(1<<i)))
        {
            mask&=~(((uint64)0xff)<<(i*8));
        }
    }

    result=(src&mask)|(dst&(~mask));

    if(data_ena)wdata=result;
}

void DATA_MUX_test()
{
    uint64 r1,r2;
    for(int i=0;i<0x1000000;i++)
    {
        uint32 m[14];
        for(int k=0;k<14*4;k++)((uint8*)m)[k]=qrand();

        for(int j=0;j<100;j++)
        {

            DATA_MUX(    r1,m[0],m[1],m[2],m[3],i&1,m[4],m[5],m[6],m[7],
                         (i>>1)&3,1,(i>>3)&0x3f,(i>>9)&0x3f,(i>>16)&0xff,
                         m[8],m[9],m[10],m[11],(i>>15)&1,m[12],m[13]);
            DATA_MUX_imp(r2,m[0],m[1],m[2],m[3],i&1,m[4],m[5],m[6],m[7],
                         (i>>1)&3,1,(i>>3)&0x3f,(i>>9)&0x3f,(i>>16)&0xff,
                         m[8],m[9],m[10],m[11],(i>>15)&1,m[12],m[13]);
            if(r1!=r2)
            {
                qDebug() << QString("big=%1, sel=%2, start=%3, end=%4, binh=%5, pthase=%6")
                        .arg(i&1).arg((i>>1)&3).arg((i>>3)&0x3f).arg((i>>9)&0x3f)
                        .arg((i>>16)&0xff).arg((i>>15)&1) << QString::number(r1,16) << QString::number(r2,16) << i;
                break;
            }
        }
    }
}

void ADDARRAY(
            uint16 &addq_0, uint16 &addq_1, uint16 &addq_2, uint16 &addq_3,
            uint16 adda_0, uint16 adda_1, uint16 adda_2, uint16 adda_3,
            uint16 addb_0, uint16 addb_1, uint16 addb_2, uint16 addb_3,
            int daddmode,//[0..2],
            //int clk,
            int reset_,
            int *carryq//[0-3]
        )
{
    /// Adder carry in is enabled in modes 001, 010, and 011
    int cinselt = (~(daddmode | (daddmode>>1)))&1;
    int cinsel = (~(cinselt | (daddmode>>2)))&1;
    /// The carry latches - carry is latched transiently
    //carryq[0-3](clk[0]) <= reset_ ? co[0-3] : 0;
    if(!reset_)carryq[0]=0;
    /// Carry in enables
    int cin = (carryq[0] & (-cinsel))&0xf;
    /// Eight bit mode in modes X10 and X11
    int eightbit = (daddmode>>1)&1;
    /// Saturate in modes X01, X10 and X11
    int sat = (daddmode | (daddmode>>1))&1;
    /// Prevent carry propagation from bits 11 to 12 in mode X11
    int hicinh = (daddmode & (daddmode>>1))&1;
    /// The adders
    int co[4];
    ADD16SAT_imp(addq_0/*IO*/,co[0]/*IO*/,adda_0,addb_0,cin&1,sat,eightbit,hicinh);
    ADD16SAT_imp(addq_1/*IO*/,co[1]/*IO*/,adda_1,addb_1,(cin>>1)&1,sat,eightbit,hicinh);
    ADD16SAT_imp(addq_2/*IO*/,co[2]/*IO*/,adda_2,addb_2,(cin>>2)&1,sat,eightbit,hicinh);
    ADD16SAT_imp(addq_3/*IO*/,co[3]/*IO*/,adda_3,addb_3,(cin>>3)&1,sat,eightbit,hicinh);

    carryq[0]=co[0]|(co[1]<<1)|(co[2]<<2)|(co[3]<<3);

}

void ADDAMUX(
            uint16 &adda_x,
            uint16 &adda_y,
            int addasel,//[0..2]
            uint16 a1_step_x,
            uint16 a1_step_y,
            uint16 a1_stepf_x,
            uint16 a1_stepf_y,
            uint16 a2_step_x,
            uint16 a2_step_y,
            uint16 a1_inc_x,
            uint16 a1_inc_y,
            uint16 a1_incf_x,
            uint16 a1_incf_y,
            int adda_xconst,//[0..2]
            int adda_yconst,
            int addareg,
            int suba_x,
            int suba_y
        )
{
    /// Multiplex the register terms
    uint16 addart_x[] = {a1_step_x,a1_stepf_x,a1_inc_x,a1_incf_x};
    uint16 addar_x = ((addasel>>2)&1)?a2_step_x:addart_x[addasel&3];

    uint16 addart_y[] = {a1_step_y,a1_stepf_y,a1_inc_y,a1_incf_y};
    uint16 addar_y = ((addasel>>2)&1)?a2_step_y:addart_y[addasel&3];
    /// Generate a constant value - this is a power of 2 in the range
    ///0-64, or zero.  The control bits are adda_xconst[0..2], when they
    ///are all 1  the result is 0.
    ///Constants for Y can only be 0 or 1
    uint16 addac_x = (1 << adda_xconst)&0x7f;
    uint16 addac_y = adda_yconst;
    /// Select between constant value and register value
    uint16 addas_x = addareg?addar_x:addac_x;
    uint16 addas_y = addareg?addar_y:addac_y;
    /// Complement these values (complement flag gives adder carry in)
    uint16 suba_x16 = -suba_x;
    uint16 suba_y16 = -suba_y;
    adda_x = suba_x16 ^ addas_x;
    adda_y = suba_y16 ^ addas_y;
}

void DATAMUX(
            uint16 &data_x,
            uint16 &data_y,
            uint32 gpu_din,
            uint16 addq_x,
            uint16 addq_y,
            int addqsel
        )
{
    data_x = addqsel?addq_x:(gpu_din&0xffff);
    data_y = addqsel?addq_y:(gpu_din>>16);
}

void ADDRADD(
            uint16 &addq_x,
            uint16 &addq_y,
            int a1fracld, // propagate address adder carry
            uint16 adda_x,
            uint16 adda_y,
            uint16 addb_x,
            uint16 addb_y,
            //int clk, // co-processor clock
            int modx, //[0..2]
            int suba_x,
            int suba_y,
            int *cxt_1,
            int *cyt_1
        )
{
    int ci_x_ = (~(cxt_1[0] ^ suba_x))&1;
    int ci_y_ = (~(cyt_1[0] ^ suba_y))&1;
    /// Perform the addition
    uint16 addqt_x;
    int co_x_,co_y_;
    FAS16_S_imp(addqt_x/*OUT*/,co_x_/*OUT*/,1,ci_x_,adda_x,addb_x);
    FAS16_S_imp(addq_y/*OUT*/,co_y_/*OUT*/,1,ci_y_,adda_y,addb_y);
    /// latch carry and propagate if required
    int cxt_0 = ((~co_x_)&1) & a1fracld;
    int cyt_0 = ((~co_y_)&1) & a1fracld;

    /// Mask low bits of X to 0 if required
    int maskbit=((1<<modx)>>1)&0x20;
    int masksel=((1<<modx)>>1)&0x1f;
    int temp;

    do
    {
        temp=maskbit;
        maskbit=(maskbit&0x20)|(masksel|(maskbit>>1));
    }while(temp!=maskbit);

    addq_x=addqt_x&(~maskbit);

    //by clock
    cxt_1[0] = cxt_0;
    cyt_1[0] = cyt_0;
}

void ADDRADD_imp(
            uint16 &addq_x,
            uint16 &addq_y,
            int a1fracld, // propagate address adder carry
            uint16 adda_x,
            uint16 adda_y,
            uint16 addb_x,
            uint16 addb_y,
            //int clk, // co-processor clock
            int modx, //[0..2]
            int suba_x,
            int suba_y,
            int *cxt_1,
            int *cyt_1
        )
{
    int ci_x_ = (~((cxt_1[0] & a1fracld) ^ suba_x))&1;
    int ci_y_ = (~((cyt_1[0] & a1fracld) ^ suba_y))&1;
    /// Perform the addition
    uint16 addqt_x;
    int co_x_,co_y_;
    FAS16_S_imp(addqt_x/*OUT*/,co_x_/*OUT*/,1,ci_x_,adda_x,addb_x);
    FAS16_S_imp(addq_y/*OUT*/,co_y_/*OUT*/,1,ci_y_,adda_y,addb_y);

    int maskbit=((1<<modx)&0x7e);
    if(maskbit)maskbit--;
    addq_x=addqt_x&(~maskbit);

    //by clock
    cxt_1[0] = ((~co_x_)&1);
    cyt_1[0] = ((~co_y_)&1);
}

void ADDRADD_test()
{
    int cx1=0,cy1=0,cx2=0,cy2=0;
    for(int i=0;i<0x40;i++)
    {
        uint16 x1,y1,x2,y2;

        for(int j=0;j<1000;j++)
        {
            uint16 m[4];
            for(int k=0;k<4*2;k++)((uint8*)m)[k]=qrand();

            ADDRADD(x1,y1,i&1,m[0],m[1],m[2],m[3],(i>>1)&7,(i>>4)&1,(i>>5)&1,
                    &cx1,&cy1);
            ADDRADD_imp(x2,y2,i&1,m[0],m[1],m[2],m[3],(i>>1)&7,(i>>4)&1,(i>>5)&1,
                    &cx2,&cy2);

            if(x1!=x2 || y1!=y2 || cx1!=cx2 || cy1!=cy2)
            {
                qDebug() << QString("x %1-%2, y %3-%4, cx %5-%6, cy %7=%8; frac=%9 mod=%10 sx=%11 sy=%12")
                            .arg(x1).arg(x2).arg(y1).arg(y2).arg(cx1).arg(cx2).arg(cy1).arg(cy2)
                            .arg(i&1).arg((i>>1)&7).arg((i>>4)&1).arg((i>>5)&1);
                break;
            }
        }
    }
}

void ADDRGEN(
            int &address,//'24' byte address
            int &pixa,//[0..2] bit part of address, un-pipe-lined
            uint16 a1_x,
            uint16 a1_y,
            int a1_base,//'21'
            int a1_pitch,//[0..1]
            int a1_pixsize,//[0..2]
            int a1_width,//[0..5]
            int a1_zoffset,//[0..1]
            uint16 a2_x,
            uint16 a2_y,
            int a2_base,//'21'
            int a2_pitch,//[0..1]
            int a2_pixsize,//[0..2]
            int a2_width,//[0..5]
            int a2_zoffset,//[0..1]
            int apipe,  // load address pipe-line latch
            //int clk,	// co-processor clock
            int gena2,	// generate A2 as opposed to A1
            int zaddr,	// generate Z address
            int *addressi //'24'
        )
{
    /// extra zeroes to cope with translate quirk of multiple inputs
    ///tied to the same thing in MX12B and MX16
    /// Multiplex between A1 and A2
    uint16 x,y;
    int width,pixsize,pitch,base,zoffset;
    if(gena2)
    {
        x=a2_x;
        y=a2_y&0xfff;
        width=a2_width;
        pixsize=a2_pixsize;
        pitch=a2_pitch;
        base=a2_base;
        zoffset=a2_zoffset;
    }
    else
    {
        x=a1_x;
        y=a1_y&0xfff;
        width=a1_width;
        pixsize=a1_pixsize;
        pitch=a1_pitch;
        base=a1_base;
        zoffset=a1_zoffset;
    }
    int width_4_ = (~width>>4)&1;
    int pitch_ = (~pitch)&3;

    /// Generate Y times mantissa width, with a pair of adders, and
    ///appropriate enables for the less significant positions.
    ///The result is ytm - Y times mantissa
    ///A FA332 is used here because the path was too slow.
    ///The alternative, two FAS16s is only 120 grids smaller.
    ///If only there was a FA316.  (2/48)
    int ym1 = (-((width>>1)&1)) & y;
    int ym2 = (-(width&1)) & y;
    uint32 ytmt;
    int temp_co;
    FA332_imp(ytmt/*OUT*/,temp_co/*OUT*/,0,ym2,ym1<<1,y<<2);
    int ytm = ytmt&0x7fff;

    /// ytm is now multiplied by an amount between 2^0 and 2^11, using
    ///shifts, this is with regard to the top bit, so the shift on ytm
    ///is actually between -2 and 9.
    int lowen = ((width>>4) | (width>>5))&1;
    int ya=0;
    ya |= lowen ? 0 : ( (((ytm&7)<<1) & (8>>((width>>2)&3))) ? 1 : 0  );
    ya |= (lowen ? 0 : ( ((ytm&0xf) & (8>>((width>>2)&3))) ? 1 : 0 )) << 1;
    ya |= (((width>>5)&1) ? 0 : ( (((ytm&0x1f)<<3)&(0x80>>((width>>2)&7))) ? 1 : 0 )) << 2;
    ya |= (((width>>5)&1) ? 0 : ( (((ytm&0x3f)<<2)&(0x80>>((width>>2)&7))) ? 1 : 0 )) << 3;
    ya |= (((width>>5)&1) ? 0 : ( (((ytm&0x7f)<<1)&(0x80>>((width>>2)&7))) ? 1 : 0 )) << 4;
    ya |= (((width>>5)&1) ? 0 : ( ((ytm&0xff)&(0x80>>((width>>2)&7))) ? 1 : 0 )) << 5;
    ya |= ((((ytm&0x1ff)<<7)&(0x8000>>((width>>2)&15))) ? 1 : 0) << 6;
    ya |= ((((ytm&0x3ff)<<6)&(0x8000>>((width>>2)&15))) ? 1 : 0) << 7;
    ya |= ((((ytm&0x7ff)<<5)&(0x8000>>((width>>2)&15))) ? 1 : 0) << 8;
    ya |= ((((ytm&0xfff)<<4)&(0x8000>>((width>>2)&15))) ? 1 : 0) << 9;
    ya |= ((((ytm&0x1ffe)<<3)&(0x8000>>((width>>2)&15))) ? 1 : 0) << 10;
    ya |= ((((ytm&0x3ffc)<<2)&(0x8000>>((width>>2)&15))) ? 1 : 0) << 11;
    ya |= ((((ytm&0x7ff8)<<1)&(0x8000>>((width>>2)&15))) ? 1 : 0) << 12;
    ya |= (((ytm&0x7ff0)&(0x8000>>((width>>2)&15))) ? 1 : 0) << 13;
    ya |= ((((ytm>>1)&0x3ff0)&(0x8000>>((width>>2)&15))) ? 1 : 0) << 14;
    ya |= ((((ytm>>2)&0x1ff0)&(0x8000>>((width>>2)&15))) ? 1 : 0) << 15;

    /// bits 16-19 use MX8G enabled for shifts 4-11, the low four and
    ///high four must be swapped
    int mid8en_ = (~((width>>4) ^ (width>>5)))&1;
    ya |= (mid8en_ ? 0 : ((((((ytm>>7)&15)<<4)|((ytm>>11)&15))&(0x80>>((width>>2)&7))) ? 1 : 0 )) << 16;
    ya |= (mid8en_ ? 0 : ((((((ytm>>8)&15)<<4)|((ytm>>12)&7))&(0x80>>((width>>2)&7))) ? 1 : 0 )) << 17;
    ya |= (mid8en_ ? 0 : ((((((ytm>>9)&15)<<4)|((ytm>>13)&3))&(0x80>>((width>>2)&7))) ? 1 : 0 )) << 18;
    ya |= (mid8en_ ? 0 : ((((((ytm>>10)&15)<<4)|((ytm>>14)&1))&(0x80>>((width>>2)&7))) ? 1 : 0 )) << 19;

    int tm4en_ = (~(width_4_ & (width>>5)))&1;
    ya |= (tm4en_ ? 0 : ((((ytm>>11)&15)&(8>>((width>>2)&3))) ? 1 : 0)) << 20;
    ya |= (tm4en_ ? 0 : ((((ytm>>12)&7)&(8>>((width>>2)&3))) ? 1 : 0)) << 21;
    ya |= (tm4en_ ? 0 : ((((ytm>>13)&3)&(8>>((width>>2)&3))) ? 1 : 0)) << 22;
    ya |= (((ytm>>14) & (width>>2) & (width>>3) & width_4_ & (width>>5))&1) << 23;

    uint16 pa_0_15;
    int pacy_15_;
    FAS16_S_imp(pa_0_15/*OUT*/,pacy_15_,1,1,ya&0xffff,x);
    int pacy=((~pacy_15_)&1)<<15;
    int pa=pa_0_15;
    for(int i=16;i<=19;i++)
    {
        int tmp=((ya>>i)&1)+(pacy>>(i-1));
        pa |= (tmp&1)<<i;
        pacy |= ((tmp>>1)&1) << i;
    }

    pa |= (((ya>>20) ^ (pacy>>19))&1) << 20;
    pacy |= (((pacy>>15) & (ya>>16) & (ya>>17) & (ya>>18) & (ya>>19) & (ya>>20))&1)<<20;

    for(int i=21;i<=22;i++)
    {
        int tmp=((ya>>i)&1)+(pacy>>(i-1));
        pa |= (tmp&1)<<i;
        pacy |= ((tmp>>1)&1) << i;
    }
    pa |= (((ya>>23)&1) + ((pacy>>22)&1)) << 23;

    /// now shift up to give a multiply by the pixel size
    pixa=0;
    pixa |= ((((((pa&0x1f)<<3) | ((pa&1)<<1)))&(0x80>>pixsize)) ? 1 : 0) << 4;
    pixa |= (((((pa&0xf)<<4))&(0x80>>pixsize)) ? 1 : 0) << 3;
    pixa |= (((((pa&7)<<5))&(0x80>>pixsize)) ? 1 : 0) << 2;
    pixa |= (((((pa&3)<<6))&(0x80>>pixsize)) ? 1 : 0) << 1;
    pixa |= (((((pa&1)<<7))&(0x80>>pixsize)) ? 1 : 0);

    int pixa_5_12[]={(pa>>5)&0xff,(pa>>4)&0xff,(pa>>3)&0xff,(pa>>2)&0xff,(pa>>1)&0xff,pa&0xff,(pa>>1)&0xff,pa&0xff};
    pixa |= pixa_5_12[pixsize] << 5;

    int pixa_13_24[]={(pa>>13)&0xfff,(pa>>12)&0xfff,(pa>>11)&0xfff,(pa>>10)&0xfff,(pa>>9)&0xfff,(pa>>8)&0xfff,(pa>>9)&0xfff,(pa>>8)&0xfff};
    pixa |= pixa_13_24[pixsize] << 13;

    pixa |= (((((pa>>18)&0x7c)|((pa>>20)&3))&(0x80>>pixsize)) ? 1 : 0) << 25;
    pixa |= (((((pa>>19)&0x3c)|((pa>>21)&3))&(0x80>>pixsize)) ? 1 : 0) << 26;

    /// add 1 to the stored value
    int pt=0;
    pt |= (pitch&1) & ((pitch_>>1)&1);
    pt |= (((pitch>>1) & pitch_)&1)<<1;

    int phradr=0;
    if(!pt)phradr|=(pixa>>6)&1;
    phradr |= (((((pixa>>6)&3)<<2)&(8>>pt)) ? 1 : 0) << 1;
    phradr |= (((((pixa>>6)&7)<<1)&(8>>pt)) ? 1 : 0) << 2;
    int phradr_3_20[]={(pixa>>9)&0x3ffff,(pixa>>8)&0x3ffff,(pixa>>7)&0x3ffff,0};
    phradr |= phradr_3_20[pt] << 3;

    /// The shifted up version for offset of 3
    int shupen = (pitch & (pitch>>1))&1;
    int shup = ((pixa>>6) & (-shupen))&0xfffff;

    /// the actual address is computed to phrase resolution, and the
    ///Z offset is added here, if enabled
    ///The bottom 3 bits add phradr, phradr shifted up 1, zoffset and base
    ///The higher bits do not add Z offset
    int za = (zoffset & (-zaddr))&3;
    int addr=0;

    int temp = (base&1) + (phradr&1) + (za&1);
    addr |= temp&1;
    int adcy0 = temp>>1;

    temp=((base>>1)&1) + ((phradr>>1)&1) + ((za>>1)&1);
    int addr1t = temp&1;
    int adcy1a = temp>>1;

    temp=addr1t + (shup&1) + adcy0;
    addr |= (temp&1)<<1;
    int adcy1b = temp>>1;

    temp=((base>>2)&1) + ((phradr>>2)&1) + adcy1a;
    int addr2t = temp&1;
    int adcy2a = temp>>1;

    temp = addr2t + ((shup>>1)&1) + adcy1b;
    addr |= (temp&1)<<2;
    int adcy2b = temp>>1;

    uint32 addr_3_20;
    int co_temp;
    FA332_imp(addr_3_20,co_temp,adcy2a|(adcy2b<<1),(shup>>2)&0x3ffff,(base>>3)&0x3ffff,(phradr>>3)&0x3ffff);
    addr|=addr_3_20<<3;

    int addrgen = (((pixa>>3)&7) | (addr<<3))&0xffffff;

    /// The address is pipe-lined here
    address = apipe?addrgen:addressi[0];
    addressi[0] = address;
    pixa&=7;
}

void ADDRGEN_imp(
            int &address,//'24' byte address
            int &pixa,//[0..2] bit part of address, un-pipe-lined
            uint16 a1_x,
            uint16 a1_y,
            int a1_base,//'21'
            int a1_pitch,//[0..1]
            int a1_pixsize,//[0..2]
            int a1_width,//[0..5]
            int a1_zoffset,//[0..1]
            uint16 a2_x,
            uint16 a2_y,
            int a2_base,//'21'
            int a2_pitch,//[0..1]
            int a2_pixsize,//[0..2]
            int a2_width,//[0..5]
            int a2_zoffset,//[0..1]
            int apipe,  // load address pipe-line latch
            //int clk,	// co-processor clock
            int gena2,	// generate A2 as opposed to A1
            int zaddr,	// generate Z address
            int *addressi //'24'
        )
{
    /// Multiplex between A1 and A2
    uint16 x,y;
    int width,pixsize,pitch,base,zoffset;
    if(gena2)
    {
        x=a2_x;
        y=a2_y&0xfff;
        width=a2_width;
        pixsize=a2_pixsize;
        pitch=a2_pitch;
        base=a2_base;
        zoffset=a2_zoffset;
    }
    else
    {
        x=a1_x;
        y=a1_y&0xfff;
        width=a1_width;
        pixsize=a1_pixsize;
        pitch=a1_pitch;
        base=a1_base;
        zoffset=a1_zoffset;
    }

    /// Generate Y times mantissa width, with a pair of adders, and
    ///appropriate enables for the less significant positions.
    ///The result is ytm - Y times mantissa
    ///A FA332 is used here because the path was too slow.
    ///The alternative, two FAS16s is only 120 grids smaller.
    ///If only there was a FA316.  (2/48)
    int ym1 = (-((width>>1)&1)) & y;
    int ym2 = (-(width&1)) & y;
    int ytm = (ym2+(ym1<<1)+(y<<2))&0x7fff;

    /// ytm is now multiplied by an amount between 2^0 and 2^11, using
    ///shifts, this is with regard to the top bit, so the shift on ytm
    ///is actually between -2 and 9.
    int ya=0;
    if(width<48)
    {
        if((width>>2)<2)ya=(uint64)ytm>>(2-(width>>2));
        else if((width>>2)>2)ya=(uint64)ytm<<((width>>2)-2);
        else ya=ytm;
        ya&=0xffffff;
    }
    int pa=(ya+x)&0x1ffffff;

    /// now shift up to give a multiply by the pixel size
    pixa=pa<<pixsize;
    if(pixsize==6)
    {
        pixa&=(~0x7fffff)<<4;
        pixa|=(pa&0x7fffff)<<4;
    }
    else if(pixsize==7)
    {
        pixa&=(~0x3fffff)<<5;
        pixa|=(pa&0x3fffff)<<5;
    }
    pixa&=0x7ffffff;

    /// add 1 to the stored value
    int phradr=(pixa>>6);
    if(pitch==1)phradr<<=1;
    else if(pitch==2)phradr<<=2;

    /// The shifted up version for offset of 3
    int shup = 0;
    if(pitch==3)shup=(pixa>>6)&0xfffff;

    /// the actual address is computed to phrase resolution, and the
    ///Z offset is added here, if enabled
    ///The bottom 3 bits add phradr, phradr shifted up 1, zoffset and base
    ///The higher bits do not add Z offset
    int za = (zoffset & (-zaddr))&3;
    int addr=za+base+(shup<<1)+phradr;

    int addrgen = (((pixa>>3)&7) | (addr<<3))&0xffffff;

    /// The address is pipe-lined here
    address = apipe?addrgen:addressi[0];
    addressi[0] = address;
    pixa&=7;
}

void ADDRGEN_test()
{
    int ai1=0, ai2=0;
    for(int i=0;i<0x4000;i++)
    {
        int a1,a2,p1,p2;
        for(int j=0;j<10000;j++)
        {
            uint32 m[3];
            for(int k=0;k<4*3;k++)((uint8*)m)[k]=qrand();

            ADDRGEN(a1,p1,
                    m[0],m[1],m[2]&0x1fffff,i&3,(i>>2)&7,(i>>5)&0x3f,(i>>11)&3,
                    m[0],m[1],m[2]&0x1fffff,i&3,(i>>2)&7,(i>>5)&0x3f,(i>>11)&3,
                    1,1,(i>>13)&1,&ai1);

            ADDRGEN_imp(a2,p2,
                    m[0],m[1],m[2]&0x1fffff,i&3,(i>>2)&7,(i>>5)&0x3f,(i>>11)&3,
                    m[0],m[1],m[2]&0x1fffff,i&3,(i>>2)&7,(i>>5)&0x3f,(i>>11)&3,
                    1,1,(i>>13)&1,&ai2);

            if(ai1!=ai2 || a1!=a2 || p1!=p2)
            {
                qDebug() << QString("pitch %1, pixsize %2, width %3, zoffset %4, zaddr %5; a<>%6-%7, p<>%8-%9")
                            .arg(i&3).arg((i>>2)&7).arg((i>>5)&0x3f).arg((i>>11)&3)
                            .arg((i>>13)&1).arg(QString::number(a1,16)).arg(QString::number(a2,16)).arg(p1).arg(p2);
                return;
            }
        }
    }
}

void BLITSTOP_imitation(
            int &gpu_dout,//[1] readable port bus
            int &stopped,// retract blitter bus request
            int &reset_,// reset to the rest of the blitter
            //clk[0]	=	clk/*IN*/,	/// co-processor clock
            int dwrite,//[1] inner loop dest write state, p-l stg 1
            uint32 gpu_din,// GPU data bus
            int nowrite,// suppress inner loop write operation
            int statrd,// blitter status read port
            int stopld,// collsion stop register load
            int xreset_,// external reset into the blitter
            int *stopen,
            int *collideb
            //int *drv_reset
        )
{
    int resume=0, abort=0;
    if(stopld)
    {
        if(gpu_din&1) resume = 1;
        if(gpu_din&2) abort=1;
        stopen[0]=(gpu_din&4)?1:0;
    }
    if(!xreset_)stopen[0]=0;

    int collide = stopen[0]&dwrite&nowrite&collideb[0]; //once: __-___
    collideb[0]=(~(stopen[0]&dwrite&nowrite))&1;

    reset_ = xreset_ | (stopped&abort);

    if((!stopped && !reset_ && collide) ||
            (stopped && !resume && !abort))
        stopped=xreset_;
    else stopped=0;

    if(statrd)gpu_dout=stopped;
}

void BLITSTOP(
            int &gpu_dout,//[1] readable port bus
            int &stopped,// retract blitter bus request
            int &reset_,// reset to the rest of the blitter
            //clk[0]	=	clk/*IN*/,	/// co-processor clock
            int dwrite,//[1] inner loop dest write state, p-l stg 1
            uint32 gpu_din,// GPU data bus
            int nowrite,// suppress inner loop write operation
            int statrd,// blitter status read port
            int stopld,// collsion stop register load
            int xreset_,// external reset into the blitter
            int *stopen,
            int *collideb,
            int *drv_reset
        )
{
    int resume_ =     (~(stopld & (gpu_din&1)))&1;
    int coll_abort_ = (~(stopld & (gpu_din>>1)))&1;
    int coll_abort = (~coll_abort_)&1;
    /// The collide signal goes active for one tick at the start of
    ///dwrite[1] when the appropriate conditions are met.
    ///collide = dwrite[1] . nowrite
    int collidea = nowrite & stopen[0] & dwrite;
    int collideb_ = (~collideb[0])&1;
    int collide = collidea & collideb_;
    /// The collision stop state machine
    int stt[3];
    int idle = (~(stopped | drv_reset[0]))&1;
    stt[0] = (~(idle & collide))&1;
    stt[1] = (~(stopped & resume_ & coll_abort_))&1;
    stt[2] = (~(stt[0] & stt[1]))&1;
    int drst = stopped & coll_abort;

    //flip-flops
    drv_reset[0] = xreset_ ? drst : 0;
    stopped = xreset_ ? stt[2] : 0;
    collideb[0] = collidea;
    stopen[0] = xreset_ ? (stopld ? ((gpu_din>>2)&1) : stopen[0]) : 0;

    /// Drive reset to the rest of the blitter from external reset or
    ///the drive reset state.
    int drv_reset_ = (~drv_reset[0])&1;
    reset_ = xreset_ & drv_reset_;

    /// make the status readable
    if(statrd)gpu_dout=stopped;
}

void COMP_CTRL(
            int &dbinh_,//[0..7] destination byte inhibit lines
            int &nowrite,// suppress inner loop write operation
            int bcompen,// bit selector inhibit enable
            int big_pix,// pixels are big-endian
            int bkgwren,// enable dest data write in pix inhibit
            //clk	=	clk/*IN*/,	/// co-processor clock
            int dcomp,//[0..7] output of data byte comparators
            int dcompen,// data comparator inhibit enable
            int icount,//[0..2] low bits of inner count
            int pixsize,//[0..2] destination pixel size
            int phrase_mode,// phrase write mode
            int srcd,//[0..7] bits to use for bit to byte expansion
            int step_inner,// inner loop advance
            int zcomp,//[0..3] output of word zed comparators
            int *bcompsel,//[0-2]
            int *bcompbitp
        )
{
    int bkgwren_ = (~bkgwren)&1;
    int phrase_mode_ = (~phrase_mode)&1;
    int pixsize_ = (~pixsize)&7;
    /// The bit comparator bits are derived from the source data, which
    ///will have been suitably aligned for phrase mode.  The contents of
    ///the inner counter are used to select which bit to use.
    ///When not in phrase mode the inner count value is used to select
    ///one bit.  It is assumed that the count has already occurred, so,
    ///7 selects bit 0, etc.  In big-endian pixel mode, this turns round,
    ///so that a count of 7 selects bit 7.
    ///In phrase mode, the eight bits are used directly, and this mode is
    ///only applicable to 8-bit pixel mode (2/34)
    int bcompselt = (icount ^ (-big_pix))&7;
    int bcompbit = (srcd&(0x80>>bcompselt))?1:0;
    int bcompbit_ = (~bcompbit)&1;
    /// pipe-line the count
    bcompbitp[0] = (srcd&(0x80>>bcompsel[0]))?1:0;
    if(step_inner)bcompsel[0]=bcompselt;
    int bcompbitp_ = (~bcompbitp[0])&1;
    /// For pixel mode, generate the write inhibit signal for all modes
    ///on bit inhibit, for 8 and 16 bit modes on comparator inhibit, and
    ///for 16 bit mode on Z inhibit
    ///Nowrite = bcompen . /bcompbit . /phrase_mode
    ///	+ dcompen . dcomp[0] . /phrase_mode . pixsize = 011
    ///	+ dcompen . dcomp[0..1] . /phrase_mode . pixsize = 100
    ///	+ zcomp[0] . /phrase_mode . pixsize = 100
    int nowt[5];
    nowt[0] = (~(bcompen & bcompbit_ & phrase_mode_))&1;
    nowt[1] = (~(dcompen & dcomp & phrase_mode_ & (pixsize_>>2) & pixsize & (pixsize>>1)))&1;
    nowt[2] = (~(dcompen & dcomp & (dcomp>>1) & phrase_mode_ & (pixsize>>2) & pixsize_ & (pixsize_>>1)))&1;
    nowt[3] = (~(zcomp & phrase_mode_ & (pixsize>>2) & pixsize_ & (pixsize_>>1)))&1;
    nowt[4] = (~(nowt[0] & nowt[1] & nowt[2] & nowt[3]))&1;
    nowrite = nowt[4] & bkgwren_;
    int winht = (~(bcompen & bcompbitp_ & phrase_mode_))&1;
    int winhibit = (~(winht & nowt[1] & nowt[2] & nowt[3]))&1;
    /// For phrase mode, generate the byte inhibit signals for eight bit
    ///mode 011, or sixteen bit mode 100
    ///dbinh\[0] =  pixsize[2] . zcomp[0]
    ///	 +  pixsize[2] . dcomp[0] . dcomp[1] . dcompen
    ///	 + /pixsize[2] . dcomp[0] . dcompen
    ///	 + /srcd[0] . bcompen
    ///Inhibits 0-3 are also used when not in phrase mode to write back
    ///destination data.
    int srcd_ = (~srcd)&0xff;
    int di0t[5];
    di0t[0] = (~((pixsize>>2) & zcomp))&1;
    di0t[1] = (~((pixsize>>2) & dcomp & (dcomp>>1) & dcompen))&1;
    di0t[2] = (~(srcd_ & bcompen))&1;
    di0t[3] = (~((pixsize_>>2) & dcomp & dcompen))&1;
    di0t[4] = (~(di0t[0] & di0t[1] & di0t[2] & di0t[3]))&1;
    dbinh_=0;
    dbinh_ |= (~(winhibit | (di0t[4] & phrase_mode)))&1;
    int di1t[3];
    di1t[0] = (~((pixsize_>>2) & (dcomp>>1) & dcompen))&1;
    di1t[1] = (~((srcd_>>1) & bcompen))&1;
    di1t[2] = (~(di0t[0] & di0t[1] & di1t[0] & di1t[1]))&1;
    dbinh_ |= ((~(winhibit | (di1t[2] & phrase_mode)))&1)<<1;
    int di2t[5];
    di2t[0] = (~((pixsize>>2) & (zcomp>>1)))&1;
    di2t[1] = (~((pixsize>>2) & (dcomp>>2) & (dcomp>>3) & dcompen))&1;
    di2t[2] = (~((srcd_>>2) & bcompen))&1;
    di2t[3] = (~((pixsize_>>2) & (dcomp>>2) & dcompen))&1;
    di2t[4] = (~(di2t[0] & di2t[1] & di2t[2] & di2t[3]))&1;
    dbinh_ |= ((~(winhibit | (di2t[4] & phrase_mode)))&1)<<2;
    int di3t[3];
    di3t[0] = (~((pixsize_>>2) & (dcomp>>3) & dcompen))&1;
    di3t[1] = (~((srcd_>>3) & bcompen))&1;
    di3t[2] = (~(di2t[0] & di2t[1] & di3t[0] & di3t[1]))&1;
    dbinh_ |= ((~(winhibit | (di3t[2] & phrase_mode)))&1)<<3;
    int di4t[5];
    di4t[0] = (~((pixsize>>2) & (zcomp>>2)))&1;
    di4t[1] = (~((pixsize>>2) & (dcomp>>4) & (dcomp>>5) & dcompen))&1;
    di4t[2] = (~((srcd_>>4) & bcompen))&1;
    di4t[3] = (~((pixsize_>>2) & (dcomp>>4) & dcompen))&1;
    di4t[4] = (~(di4t[0] & di4t[1] & di4t[2] & di4t[3]))&1;
    dbinh_ |= ((~(di4t[4] & phrase_mode))&1)<<4;
    int di5t[3];
    di5t[0] = (~((pixsize_>>2) & (dcomp>>5) & dcompen))&1;
    di5t[1] = (~((srcd_>>5) & bcompen))&1;
    di5t[2] = (~(di4t[0] & di4t[1] & di5t[0] & di5t[1]))&1;
    dbinh_ |= ((~(di5t[2] & phrase_mode))&1)<<5;
    int di6t[5];
    di6t[0] = (~((pixsize>>2) & (zcomp>>3)))&1;
    di6t[1] = (~((pixsize>>2) & (dcomp>>6) & (dcomp>>7) & dcompen))&1;
    di6t[2] = (~((srcd_>>6) & bcompen))&1;
    di6t[3] = (~((pixsize_>>2) & (dcomp>>6) & dcompen))&1;
    di6t[4] = (~(di6t[0] & di6t[1] & di6t[2] & di6t[3]))&1;
    dbinh_ |= ((~(di6t[4] & phrase_mode))&1)<<6;
    int di7t[3];
    di7t[0] = (~((pixsize_>>2) & (dcomp>>7) & dcompen))&1;
    di7t[1] = (~((srcd_>>7) & bcompen))&1;
    di7t[2] = (~(di6t[0] & di6t[1] & di7t[0] & di7t[1]))&1;
    dbinh_ |= ((~(di7t[2] & phrase_mode))&1)<<7;
}

void COMP_CTRL_imp(
            int &dbinh_,//[0..7] destination byte inhibit lines
            int &nowrite,// suppress inner loop write operation
            int bcompen,// bit selector inhibit enable
            int big_pix,// pixels are big-endian
            int bkgwren,// enable dest data write in pix inhibit
            //clk	=	clk/*IN*/,	/// co-processor clock
            int dcomp,//[0..7] output of data byte comparators
            int dcompen,// data comparator inhibit enable
            int icount,//[0..2] low bits of inner count
            int pixsize,//[0..2] destination pixel size
            int phrase_mode,// phrase write mode
            int srcd,//[0..7] bits to use for bit to byte expansion
            int step_inner,// inner loop advance
            int zcomp,//[0..3] output of word zed comparators
            int *bcompsel,//[0-2]
            int *bcompbitp
        )
{
    int bkgwren_ = (~bkgwren)&1;
    int phrase_mode_ = (~phrase_mode)&1;
    int pixsize_ = (~pixsize)&7;
    /// The bit comparator bits are derived from the source data, which
    ///will have been suitably aligned for phrase mode.  The contents of
    ///the inner counter are used to select which bit to use.
    ///When not in phrase mode the inner count value is used to select
    ///one bit.  It is assumed that the count has already occurred, so,
    ///7 selects bit 0, etc.  In big-endian pixel mode, this turns round,
    ///so that a count of 7 selects bit 7.
    ///In phrase mode, the eight bits are used directly, and this mode is
    ///only applicable to 8-bit pixel mode (2/34)
    int bcompselt = (icount ^ (-big_pix))&7;
    int bcompbit = (srcd&(0x80>>bcompselt))?1:0;
    int bcompbit_ = (~bcompbit)&1;
    /// pipe-line the count
    bcompbitp[0] = (srcd&(0x80>>bcompsel[0]))?1:0;
    if(step_inner)bcompsel[0]=bcompselt;
    int bcompbitp_ = (~bcompbitp[0])&1;
    /// For pixel mode, generate the write inhibit signal for all modes
    ///on bit inhibit, for 8 and 16 bit modes on comparator inhibit, and
    ///for 16 bit mode on Z inhibit
    ///Nowrite = bcompen . /bcompbit . /phrase_mode
    ///	+ dcompen . dcomp[0] . /phrase_mode . pixsize = 011
    ///	+ dcompen . dcomp[0..1] . /phrase_mode . pixsize = 100
    ///	+ zcomp[0] . /phrase_mode . pixsize = 100
    int nowt[5];
    nowt[0] = (~(bcompen & bcompbit_ & phrase_mode_))&1;
    nowt[1] = (~(dcompen & dcomp & phrase_mode_ & (pixsize_>>2) & pixsize & (pixsize>>1)))&1;
    nowt[2] = (~(dcompen & dcomp & (dcomp>>1) & phrase_mode_ & (pixsize>>2) & pixsize_ & (pixsize_>>1)))&1;
    nowt[3] = (~(zcomp & phrase_mode_ & (pixsize>>2) & pixsize_ & (pixsize_>>1)))&1;
    nowt[4] = (~(nowt[0] & nowt[1] & nowt[2] & nowt[3]))&1;
    nowrite = nowt[4] & bkgwren_;
    int winht = (~(bcompen & bcompbitp_ & phrase_mode_))&1;
    int winhibit = (~(winht & nowt[1] & nowt[2] & nowt[3]))&1;
    /// For phrase mode, generate the byte inhibit signals for eight bit
    ///mode 011, or sixteen bit mode 100
    ///dbinh\[0] =  pixsize[2] . zcomp[0]
    ///	 +  pixsize[2] . dcomp[0] . dcomp[1] . dcompen
    ///	 + /pixsize[2] . dcomp[0] . dcompen
    ///	 + /srcd[0] . bcompen
    ///Inhibits 0-3 are also used when not in phrase mode to write back
    ///destination data.
    int duty[16]={0,1,4,5,16,17,20,21,64,65,68,69,80,81,84,85};
    int p1=((~srcd)&(-bcompen))&0xff;
    int p2=((-((pixsize>>2)&1))&duty[zcomp])&0xff;
    int p3=((-((pixsize>>2)&1))&dcomp&(dcomp>>1)&(-dcompen))&0x55;
    int p4=((-((pixsize>>2)&1))&dcomp&(-dcompen))&0xff;
    dbinh_=(~(((p1|p2|p3|p4|(p2<<1)|(p3<<1))&(-phrase_mode))|((-winhibit)&0xf)))&0xff;
}

void COMP_CTRL_test()
{
    int bs1=0,bs2=0,bb1=0,bb2=0;
    for(uint64 i=0;i<0x100000000;i++)
    {
        int bh1,bh2,nw1,nw2;

        COMP_CTRL(bh1,nw1,i&1,(i>>1)&1,(i>>2)&1,(i>>3)&0xff,(i>>11)&1,(i>>12)&7,(i>>15)&7,(i>>18)&1,
                  (i>>19)&0xff,(i>>27)&1,(i>>28)&15,&bs1,&bb1);
        COMP_CTRL(bh2,nw2,i&1,(i>>1)&1,(i>>2)&1,(i>>3)&0xff,(i>>11)&1,(i>>12)&7,(i>>15)&7,(i>>18)&1,
                  (i>>19)&0xff,(i>>27)&1,(i>>28)&15,&bs2,&bb2);

        if(bh1!=bh2 || nw1!=nw2 || bs1!=bs2 || bb1!=bb2)
        {
            qDebug() << QString("bcompen %1, big_pix %2, bkgwren %3, dcomp %4, dcompen %5, "
                    "icount %6, pixsize %7, phrase_mode %8, srcd %9, step_inner %10, zcomp %11 | ")
                    .arg(i&1).arg((i>>1)&1).arg((i>>2)&1).arg((i>>3)&0xff).arg((i>>11)&1).arg((i>>12)&7)
                    .arg((i>>15)&7).arg((i>>18)&1).arg((i>>19)&0xff).arg((i>>27)&1).arg((i>>28)&15)
                     << bh1 << bh2 << "|" << nw1 << nw2 << "|" << bs1 << bs2 << "|" << bb1 << bb2;
            break;
        }
    }
}

void DCONTROL(
            int &daddasel,//[0..2] data adder input A selection
            int &daddbsel,//[0..2] data adder input B selection
            int &daddmode,//[0..2] data adder mode
            int &data_sel,//[0..1] select data to write
            int &daddq_sel,// select adder output in data path
            int &gourd,// enable gouraud data computation
            int &gourz,// enable gouraud zed computation
            int &patdadd,// pattern data gouraud add
            int &patfadd,// pattern fraction gouraud add
            int &srcz1add,// zed data gouraud add
            int &srcz2add,// zed fraction gouraud add
            int atick,//[0..1] data compute timing
            //clk[0]	=	clk/*IN*/,	/// co-processor clock
            int cmdld,// command register load
            int dwrite,// inner loop dest data write state
            int dzwrite,// inner loop dest zed write state
            int dzwrite1,// ... one pipe-lien stage on
            uint32 gpu_din,	// GPU data bus
            int srcdreadd,// timing of extra load for srcshade
            int srcshade,// use intensity increment on source data
            int *controlbits, int *dzwrite1d
        )
{
    /// Data update control bits
    if(cmdld) controlbits[0]=(gpu_din>>12)&0x3f;
    gourd = controlbits[0]&1;
    gourz = (controlbits[0]>>1)&1;
    int topben = (controlbits[0]>>2)&1;
    int topnen = (controlbits[0]>>3)&1;
    int patdsel = (controlbits[0]>>4)&1;
    int adddsel = (controlbits[0]>>5)&1;
    int gourd_ = (~gourd)&1;
    int gourz_ = (~gourz)&1;
    int topben_ = (~topben)&1;

    ///*  Data Adder Control  ****************************************
    int atickboth = (atick | (atick>>1))&1;
    /// Data adder control, input A selection
    ///000   Destination data
    ///100   Source data      - computed intensity fraction
    ///101   Pattern data     - computed intensity
    ///110   Source zed 1     - computed zed
    ///111   Source zed 2     - computed zed fraction
    ///Bit 0 =   dwrite  . gourd . atick[1]
    ///	+ dzwrite . gourz . atick[0]
    ///Bit 1 =   dzwrite . gourz . (atick[0] + atick[1])
    ///Bit 2 =   gourd
    ///	+ gourz
    ///	+ dwrite  . srcshade
    int shadeadd_ = (~(dwrite & srcshade))&1;
    int shadeadd = (~shadeadd_)&1;
    int dasel0t[2];
    dasel0t[0] = (~(dwrite & gourd & (atick>>1)))&1;
    dasel0t[1] = (~(dzwrite & gourz & atick))&1;
    daddasel = (~(dasel0t[0] & dasel0t[1])&1);
    daddasel |= (dzwrite & gourz & atickboth)<<1;
    daddasel |= (gourd | gourz | shadeadd)<<2;
    /// Data adder control, input B selection
    ///000	Source data
    ///100	Bottom 16 bits of I inc repeated four times
    ///101	Top 16 bits of I inc repeated four times
    ///110	Bottom 16 bits of Z inc repeated four times
    ///111	Top 16 bits of Z inc repeated four times
    ///Bit 0 =   dwrite  . gourd . atick[1]
    ///	+ dzwrite . gourz . atick[1]
    ///	+ dwrite  . srcshade
    ///Bit 1 =   dzwrite . gourz . (atick[0] + atick[1])
    ///Bit 2 =   dwrite  . gourd . (atick[0] + atick[1])
    ///	+ dzwrite . gourz . (atick[0] + atick[1])
    ///	+ dwrite  . srcshade
    int dbsel0t[2];
    dbsel0t[0] = (~(dwrite & gourd & (atick>>1)))&1;
    dbsel0t[1] = (~(dzwrite & gourz & (atick>>1)))&1;
    daddbsel = (~(dbsel0t[0] & dbsel0t[1] & shadeadd_))&1;
    daddbsel |= (dzwrite & gourz & atickboth)<<1;
    int dbsel2t[2];
    dbsel2t[0] = (~(dwrite & gourd & atickboth))&1;
    dbsel2t[1] = (~(dzwrite & gourz & atickboth))&1;
    daddbsel |= ((~(dbsel2t[0] & dbsel2t[1] & shadeadd_))&1)<<2;
    /// Data adder mode control
    ///X00	16-bit normal add
    ///001	16-bit saturating add with carry
    ///010	8-bit saturating add with carry, carry into top byte is
    ///	inhibited (YCrCb)
    ///011	8-bit saturating add with carry, carry into top byte and
    ///	between top nybbles is inhibited (CRY)
    ///101	16-bit saturating add
    ///110	8-bit saturating add, carry into top byte is inhibited
    ///111	8-bit saturating add, carry into top byte and between top
    ///	nybbles is inhibited
    ///The first four are used for Gouraud calculations, the latter three
    ///for adding source and destination data
    ///Bit 0 =   dzwrite . gourz . atick[1]
    ///	+ dwrite  . gourd . atick[1] . /topnen . /topben
    ///	+ dwrite  . gourd . atick[1] .  topnen .  topben
    ///	+ /gourd . /gourz . /topnen . /topben
    ///	+ /gourd . /gourz .  topnen .  topben
    ///	+ shadeadd . /topnen . /topben
    ///	+ shadeadd .  topnen .  topben
    ///
    ///Bit 1 =   dwrite . gourd . atick[1] . /topben
    ///	+ /gourd . /gourz .  /topben
    ///	+ shadeadd .  /topben
    ///Bit 2 =   /gourd . /gourz
    ///	+ shadeadd
    int dm0t[5];
    dm0t[0] = (~(dzwrite & gourz & (atick>>1)))&1;
    dm0t[1] = (~(topben ^ topnen))&1;
    dm0t[2] = (~(dwrite & gourd & (atick>>1) & dm0t[1]))&1;
    dm0t[3] = (~(gourd_ & gourz_ & dm0t[1]))&1;
    dm0t[4] = (~(shadeadd & dm0t[1]))&1;
    daddmode = (~(dm0t[0] & dm0t[2] & dm0t[3] & dm0t[4]))&1;
    int dm1t[3];
    dm1t[0] = (~(dwrite & gourd & (atick>>1) & topben_))&1;
    dm1t[1] = (~(gourd_ & gourz_ & topben_))&1;
    dm1t[2] = (~(shadeadd & topben_))&1;
    daddmode |= ((~(dm1t[0] & dm1t[1] & dm1t[2]))&1)<<1;
    daddmode |= (shadeadd | (gourd_ & gourz_))<<2;

    /// Data add load controls
    ///Pattern fraction (dest data) is loaded on
    ///	dwrite . gourd . atick[0]
    ///Pattern data is loaded on
    ///	dwrite . gourd . atick[1]
    ///Source z1 is loaded on
    ///	dzwrite . gourz . atick[1]
    ///Source z2 is loaded on
    ///	dzwrite . gourz . atick[0]
    ///Texture map shaded data is loaded on
    ///	srcdreadd . srcshade
    patfadd = (dwrite & gourd & atick)&1;
    patdadd = (dwrite & gourd & (atick>>1))&1;
    srcz1add = (dzwrite & gourz & (atick>>1))&1;
    srcz2add = (dzwrite & gourz & atick)&1;
    int srcshadd = srcdreadd & srcshade;
    daddq_sel = patfadd | patdadd | srcz1add | srcz2add | srcshadd;
    /// Select write data
    ///This has to be controlled from stage 1 of the pipe-line, delayed
    ///by one tick, as the write occurs in the cycle after the ack.
    ///00	pattern data
    ///01	lfu data
    ///10	adder output
    ///11	source zed
    ///Bit 0 =  /patdsel . /adddsel
    ///	+ dzwrite1d
    ///Bit 1 =   adddsel
    ///	+ dzwrite1d
    dzwrite1d[0] = dzwrite1;
    int dsel0t = (~(patdsel | adddsel))&1;
    data_sel = dzwrite1d[0] | dsel0t;
    data_sel |= (dzwrite1d[0] | adddsel)<<1;
}

void DCOUNT16(
                uint16 &count,
                uint16 data,
                int load,
                int ena
                //clk	=	clk/*IN*/,
                )
{
    int cnti = (~count)&1;
    cnti |= ((~((count>>1) ^ count))&1)<<1;
    int cry = (((count>>1) | count)&1)<<1;

    int cry_temp;
    do
    {
        cry_temp=cry;
        cnti = (cnti&(~(0x1fff<<2)))|(((~((count>>2) ^ (cry>>1)))&0x1fff)<<2);
        cry = (cry&(~(0x1fff<<2)))|((((count>>2) | (cry>>1))&0x1fff)<<2);
    }while(cry_temp!=cry);

    cnti |= ((~((count>>15) ^ (cry>>14)))&1)<<15;

    if(load)count=data;
    else if(ena)count=cnti;
}

void DCOUNT16_imp(
                uint16 &count,
                uint16 data,
                int load,
                int ena
                //clk	=	clk/*IN*/,
                )
{
    uint16 cnti=count-1;
    if(load)count=data;
    else if(ena)count=cnti;
}

void DCOUNT16_test()
{
    uint16 c1=0, c2=0;
    for(int i=0;i<0x20000;i++)
    {
        DCOUNT16(c1,0,0,1);
        DCOUNT16_imp(c2,0,0,1);
        if(c1!=c2){qDebug() << c1 << c2 << i;break;}
    }
}

void OUTER_CNT(
                int &outer0, // counter has reached zero
                //clk	=	clk/*IN*/,	/// co-processor clock
                int countld,// outer counter load
                uint32 gpu_din,	// GPU data bus
                int ocntena,	// outer count enable
                uint16 *dcount_reg
            )
{
    /// the counter
    DCOUNT16_imp(*dcount_reg,gpu_din>>16,countld,ocntena);
    /// detect count value of zero
    outer0 = dcount_reg[0]?0:1;
}

void INNER_CNT(
                uint32 &gpu_dout,//gpu_dout[16..31]/*TRI*/ readable port bus
                int &icount_,//[0..2] part of the counter value
                int &inner0,// counter has reached zero
                //clk	=	clk/*IN*/,	/// co-processor clock
                int countld,// inner counter load
                uint16 dstxp,// destination X address
                uint32 gpu_din,// GPU data bus
                int icntena,// inner count enable
                int ireload,// inner counter reload
                int phrase_mode,// phrase write mode
                int pixsize,//[0..2] destination pixel size
                int statrd,// read status port
                uint16 *cntval,
                int *cntlden,
                uint16 *icountt,
                int *underflow
            )
{
    int icount=icountt[0];

    int pixsize_ = (~pixsize)&7;
    int pixel8_ = (~(pixsize & (pixsize>>1) & (pixsize_>>2)))&1;
    int pixel8 = (~pixel8_)&1;
    int pixel16_ = (~(pixsize_ & (pixsize_>>1) & (pixsize>>2)))&1;
    int pixel16 = (~pixel16_)&1;
    int pixel32_ = (~(pixsize & (pixsize_>>1) & (pixsize>>2)))&1;
    int pixel32 = (~pixel32_)&1;

    /// negate the bottom three bits of the dest X
    int dstxp_ = (~dstxp)&7;
    int inct[3],inct_[3];
    inct[0] = (~dstxp_)&1;
    inct[1] = ((dstxp_>>1)&1) + (dstxp_&1);
    inct[2] = ((dstxp_>>2) ^ (inct[1]>>1))&1; inct[1]&=1;
    inct_[0]=(~inct[0])&1; inct_[1]=(~inct[1])&1; inct_[2]=(~inct[2])&1;

    /// inc0 = /phrase_mode + phrase_mode . inct[0]
    int inc0t = (~(phrase_mode & inct[0]))&1;
    int inc_ = inc0t & phrase_mode;
    /// inc1 = phrase_mode . ( pixsize 8 or 16 . inct[1]
    ///			+ pixsize 32 . inct = xx0)
    int inc1t[5];
    inc1t[1] = (~(pixel8_ & pixel16_))&1;
    inc1t[2] = (~(inc1t[1] & inct[1]))&1;
    inc1t[3] = (~(pixel32 & inct_[0]))&1;
    inc1t[4] = (~(inc1t[2] & inc1t[3]))&1;
    inc_ |= ((~(phrase_mode & inc1t[4]))&1)<<1;
    /// inc2 = phrase_mode . ( pixsize 8 . inct[2]
    ///			+ pixsize 16 . inct = x00)
    int inc2t[3];
    inc2t[0] = (~(pixel8 & inct[2]))&1;
    inc2t[1] = (~(pixel16 & inct_[0] & inct_[1]))&1;
    inc2t[2] = (~(inc2t[0] & inc2t[1]))&1;
    inc_ |= ((~(phrase_mode & inc2t[2]))&1)<<2;
    /// inc3 = phrase_mode . pixsize 8 . inct = 000
    inc_ |= ((~(phrase_mode & pixel8 & inct_[0] & inct_[1] & inct_[2]))&1)<<3;

    int count= inc_+(icount&0xf)+1;
    int carry = (count&0x10)>>1;
    int carry_temp;

    do
    {
        carry_temp=carry;
        count = (inc_+(icount&0xf)+1)&0xf;
        count |= ((~((carry>>3) ^ (icount>>4)))&0x3f)<<4;
        carry = (carry&(~(0x3f<<4))) | ((((carry>>3) | (icount>>4))&0x3f)<<4);
        count |= ((~((carry>>9) ^ (icount>>10)))&1)<<10;
        int cla10 = ((carry>>3) | (icount>>4) | (icount>>5) | (icount>>6) | (icount>>7) | (icount>>8) | (icount>>9) | (icount>>10))&1;
        count |= ((~(cla10 ^ (icount>>11)))&1)<<11;
        carry = (carry&(~(1<<11))) | (((cla10 | (icount>>11))&1)<<11);
        count |= ((~((carry>>11) ^ (icount>>12)))&7)<<12;
        carry = (carry&(~(7<<12))) | ((((carry>>11)|(icount>>12))&7)<<12);
        count |= ((~((carry>>14) ^ (icount>>15)))&1)<<15;
    }while(carry!=carry_temp);

    /// Select load value, count, or existing contents
    ///- the counter is loaded the tick after a new value is loaded
    int cntisel = ireload | cntlden[0];
    uint16 cnti=icount;
    if(cntisel)cnti=cntval[0];
    else if(icntena)cnti=count;

    /// Detect count value of zero.
    ///In pixel mode this is when the count has actaully reached zero.
    ///In phrase mode this means when the count has reached zero or
    ///underflowed.  Underflow is given by the top bit of the counter
    ///going from 0 to 1 when count is enabled.
    int icount_15_ = (~(icount>>15))&1;
    int uflowt = ((count>>15) & icount_15_)&1;

    //flip flop
    cntlden[0] = countld;
    if(icntena)underflow[0] = uflowt;
    icountt[0] = cnti;
    icount_=icountt[0]&7;

    /// counter value register
    if(countld)cntval[0]=gpu_din;

    int inner0t = icountt[0]?0:1;
    inner0 = inner0t | underflow[0];
    /// make the counter readable
    if(statrd)gpu_dout=(gpu_dout&0xffff)|(icountt[0]<<16);
}

void INNER_CNT_imp(
                uint32 &gpu_dout,//gpu_dout[16..31]/*TRI*/ readable port bus
                int &icount_,//[0..2] part of the counter value
                int &inner0,// counter has reached zero
                //clk	=	clk/*IN*/,	/// co-processor clock
                int countld,// inner counter load
                uint16 dstxp,// destination X address
                uint32 gpu_din,// GPU data bus
                int icntena,// inner count enable
                int ireload,// inner counter reload
                int phrase_mode,// phrase write mode
                int pixsize,//[0..2] destination pixel size
                int statrd,// read status port
                uint16 *cntval,
                int *cntlden,
                uint16 *icountt,
                int *underflow
            )
{
    int icount=icountt[0];

    int pixsize_ = (~pixsize)&7;
    int pixel8_ = (~(pixsize & (pixsize>>1) & (pixsize_>>2)))&1;
    int pixel8 = (~pixel8_)&1;
    int pixel16_ = (~(pixsize_ & (pixsize_>>1) & (pixsize>>2)))&1;
    int pixel16 = (~pixel16_)&1;
    int pixel32_ = (~(pixsize & (pixsize_>>1) & (pixsize>>2)))&1;
    int pixel32 = (~pixel32_)&1;

    /// negate the bottom three bits of the dest X
    int dstxp_ = (~dstxp)&7;
    int inct[3],inct_[3];
    inct[0] = (~dstxp_)&1;
    inct[1] = ((dstxp_>>1)&1) + (dstxp_&1);
    inct[2] = ((dstxp_>>2) ^ (inct[1]>>1))&1; inct[1]&=1;
    inct_[0]=(~inct[0])&1; inct_[1]=(~inct[1])&1; inct_[2]=(~inct[2])&1;

    /// inc0 = /phrase_mode + phrase_mode . inct[0]
    int inc0t = (~(phrase_mode & inct[0]))&1;
    int inc_ = inc0t & phrase_mode;
    /// inc1 = phrase_mode . ( pixsize 8 or 16 . inct[1]
    ///			+ pixsize 32 . inct = xx0)
    int inc1t[5];
    inc1t[1] = (~(pixel8_ & pixel16_))&1;
    inc1t[2] = (~(inc1t[1] & inct[1]))&1;
    inc1t[3] = (~(pixel32 & inct_[0]))&1;
    inc1t[4] = (~(inc1t[2] & inc1t[3]))&1;
    inc_ |= ((~(phrase_mode & inc1t[4]))&1)<<1;
    /// inc2 = phrase_mode . ( pixsize 8 . inct[2]
    ///			+ pixsize 16 . inct = x00)
    int inc2t[3];
    inc2t[0] = (~(pixel8 & inct[2]))&1;
    inc2t[1] = (~(pixel16 & inct_[0] & inct_[1]))&1;
    inc2t[2] = (~(inc2t[0] & inc2t[1]))&1;
    inc_ |= ((~(phrase_mode & inc2t[2]))&1)<<2;
    /// inc3 = phrase_mode . pixsize 8 . inct = 000
    inc_ |= ((~(phrase_mode & pixel8 & inct_[0] & inct_[1] & inct_[2]))&1)<<3;

    int count= icount-((~inc_)&0xf);

    /// Select load value, count, or existing contents
    ///- the counter is loaded the tick after a new value is loaded
    int cntisel = ireload | cntlden[0];
    uint16 cnti=icount;
    if(cntisel)cnti=cntval[0];
    else if(icntena)cnti=count;

    /// Detect count value of zero.
    ///In pixel mode this is when the count has actaully reached zero.
    ///In phrase mode this means when the count has reached zero or
    ///underflowed.  Underflow is given by the top bit of the counter
    ///going from 0 to 1 when count is enabled.
    int icount_15_ = (~(icount>>15))&1;
    int uflowt = ((count>>15) & icount_15_)&1;

    //flip flop
    cntlden[0] = countld;
    if(icntena)underflow[0] = uflowt;
    icountt[0] = cnti;
    icount_=icountt[0]&7;

    /// counter value register
    if(countld)cntval[0]=gpu_din;

    int inner0t = icountt[0]?0:1;
    inner0 = inner0t | underflow[0];
    /// make the counter readable
    if(statrd)gpu_dout=(gpu_dout&0xffff)|(icountt[0]<<16);
}

void INNER_CNT_test()
{
    uint16 cntval[2]={0};
    int cntlden[2]={0};
    uint16 icountt[2]={1000,1000};
    int underflow[2]={0};

    for(int i=0;i<0x80;i++)
    {
        uint32 gpu_dout[2]={0};
        int icount_[2]={0};
        int inner0[2]={0};

        for(int j=0;j<0x20000;j++)
        {
            INNER_CNT(gpu_dout[0],icount_[0],inner0[0],
                    0,i&7,0,1,0,(i>>3)&1,(i>>4)&7,0,
                    cntval,cntlden,icountt,underflow);
            INNER_CNT_imp(gpu_dout[1],icount_[1],inner0[1],
                    0,i&7,0,1,0,(i>>3)&1,(i>>4)&7,0,
                    cntval+1,cntlden+1,icountt+1,underflow+1);
            if(cntval[0]!=cntval[1] || cntlden[0]!=cntlden[1] || icountt[0]!=icountt[1] || underflow[0]!=underflow[1] ||
                    gpu_dout[0]!=gpu_dout[1] || icount_[0]!=icount_[1] || inner0[0]!=inner0[1])
            {
                qDebug() << icountt[0] << icountt[1] << i << j;
                return;
            }
        }
    }
}

uint32 BARREL32(
    int mux,//[0..1]/*IN*/,
    int sft,//[0..4]/*IN*/,
    uint32 a/*IN*/
){
    int mux_ = (~mux)&3;
    int sft_4_ = (~(sft>>4))&1;
    /// The barrel shifter
    uint32 b=a;
    if((sft>>4)&1) b=(a>>16)|(a<<16);
    uint32 c=b;
    if((sft>>3)&1) c=(b>>24)|(b<<8);
    uint32 d=c;
    if((sft>>2)&1) d=(c>>28)|(c<<4);
    uint32 e=d;
    if((sft>>1)&1) e=(d>>30)|(d<<2);
    uint32 f=e;
    if(sft&1) f=(e>>31)|(e<<1);
    /// Decode the shift count
    uint32 dcd = (1 << (sft&15)) & (((sft>>4)&1)?0:-1);
    dcd |= ((1 << (sft&15)) & (sft_4_?0:-1))<<16;
    /// look-ahead is employed
    uint32 tmask,rmask = dcd;
    do
    {
        tmask=rmask;
        rmask |= (rmask<<1);
    }while(rmask!=tmask);
    /// Generate the shift left mask
    ///This corresponds to the bits to be masked out
    ///i.e.
    ///Shift left 0  = mask none
    ///Shift left 1  = mask bit 0    = code 1
    ///Shift left 2  = mask bit 1-0  = code 2
    ///...
    ///Shift left 31 = mask bit 30-0 = code 31
    uint32 lmask=dcd>>1;
    do
    {
        tmask=lmask;
        lmask |= (lmask>>1);
    }while(lmask!=tmask);
    /// Output control
    int lsl = (mux_ & (mux_>>1))&1;
    int sr = mux&1;
    int asr_sign = (mux & (mux>>1) & (a>>31))&1;
    uint32 opt0 = ~(((-lsl) & lmask) | ((-sr) & rmask));
    uint32 opt1 = (-asr_sign) & rmask;
    return opt1 | (f & opt0);
}

uint32 BARREL32_imp(
    int mux,//[0..1]/*IN*/,
    int sft,//[0..4]/*IN*/,
    uint32 a/*IN*/
){
    switch(mux&3)
    {
    case 0: //lsl
        return a<<sft;
    case 1: //lsr
        if(!sft)return 0;
        return a>>(32-sft);
    case 2: //rot
        return __rotl<uint32>(a,sft);
    };
    if(!sft)sft=1;
    return ((int32)a)>>(32-sft);
}

void BARREL32_test()
{
    for(int i=0;i<0x80;i++)
    {
        for(int j=0;j<10000;j++)
        {
            uint32 m[1];
            for(int k=0;k<4;k++)((uint8*)m)[k]=qrand();

            uint32 r1=BARREL32(i&3,i>>2,m[0]);
            uint32 r2=BARREL32_imp(i&3,i>>2,m[0]);
            if(r1!=r2)
            {
                qDebug() << QString("mux=%1, shift=%2, val=%3, %4!=%5")
                            .arg(i&3).arg(i>>2).arg(QString::number(m[0],16))
                        .arg(QString::number(r1,16)).arg(QString::number(r2,16));
                return;
            }
        }
    }
}

uint32 divAlgo(uint32 dn, uint32 dr, uint32 &wk, bool fixpoint)
{
    //dn 32-bit dividend
    //dr 32-bit divisor
    uint32 qu=0; //32-bit quotient
    wk=0; //32-bit working value (initially 0)

    if(fixpoint)
    {
        wk=dn>>16;
        dn<<=16;
    }

    //loop 32 times
    for(int i=0;i<32;i++)
    {
        uint32 s=(wk>>31);
        //shift wk left 1, shifting in top bit of dn
        wk<<=1; wk|=dn>>31;
        //shift dn left 1
        dn<<=1;
        //if the previous result was negative then
            //add dr to wk
        //else	subtract dr from wk
        if(s)wk+=dr;
        else wk-=dr;
        //if the result is positive left shift 1 into the qu else 0
        qu<<=1; qu|=(~(wk>>31))&1;
    }
    return qu;
}

void MACOUNT(	int &maddr,//mtxaddr[2..11] OUT address
                int maddw, // add width rather than 1
                int mwidth//[0..3] matrix width
            )
{
    int maddw_ = (~maddw)&1;
    int inc = (mwidth | maddw_)&1;
    inc |= (mwidth & (-maddw))&0xe;

    maddr=(maddr+inc)&0x3ff;
}

void MCOUNT(    int *count, //[0-3]
                int &count1	/// counter has reached 1
                //clk	=	clk/*IN*/,	/// system clock
                //cnten	=	mcnten/*IN*/,	/// counter enable
                //cntld	=	mmult/*IN*/,	/// counter load
                //int mwidth	/// [0..3] matrix width
            )
{
    (*count)--;
    (*count)&=0xf;
    //cnti[0-3] <= {count[0-3],cntt[0-3],mwidth[0-3],mwidth[0-3]}[cnten@cntld];
    //count[0-3](clk) <= cnti[0-3];
    //count\[0] <= ~count[0];
    count1 = (*count)==1?1:0;
}

void R1COUNT(
                int &count//[0..5]	=	reghalf/*OUT*/ @ sysr1[0..4]/*OUT*/,	/// counter
                //clk	=	clk/*IN*/,	/// system clock
                //cnten	=	romold/*IN*/,	/// counter enable
                //cntld	=	mmult/*IN*/,	/// counter load
                //mr1[0..4]	=	instruction'16'{5..9}/*IN*/,	/// value to load
            )//count[0-5]
{
    count++;
    count&=0x3f;
    //!!! cnti[0] <= cntld ? 0 : {count[0],cntt[0]}[cnten];
    //cnti[1-5] <= {count[1-5],cntt[1-5],mr1[0-4],mr1[0-4]}[cnten@cntld];
    //count[0-5](clk) <= cnti[0-5];
}

int SUBSIZE(
                //int &sub,//'23'	=	subq'23'/*OUT*/,
                int a,//'23'	=	pc'23'/*IN*/,
                int b//[0..2]	=	qs\[0..2]/*IN*/,
            )
{
    int sub_0 = (a&1) + (b&1);
    int sub_1 = ((a>>1)&1) + ((b>>1)&1) + (sub_0>>1);
    int sub_2 = ((a>>2)&1) + ((b>>2)&1) + (sub_1>>1);

    int co=((sub_0>>1)&1)|(sub_1&2)|((sub_2<<1)&4);
    int sub=(sub_0&1)|((sub_1<<1)&2)|((sub_2<<2)&4);

    int temp;

    do
    {
        temp=co;
        sub = (((~((a>>3) ^ (co>>2)))&0x3f)<<3)|(sub&(~(0x3f<<3)));
        co = (((a>>3) | (co>>2))<<3)|(co&(~(0x3f<<3)));
        sub = (((~((a>>9) ^ (co>>8)))&1)<<9) | (sub&(~(1<<9)));
        co = ((( ((a&0x3f8)?1:0) | (co>>2))&1)<<9) | (co&(~(1<<9)));
        sub = (((~((a>>10) ^ (co>>9)))&0x3f)<<10) | (sub&(~(0x3f<<10)));
        co = ((((a>>10) | (co>>9))&0x3f)<<10) | (co&(~(0x3f<<10)));
        sub = (((~((a>>16) ^ (co>>15)))&1)<<16) | (sub&(~(1<<16)));
        co = ((( ((a&0x1fc00)?1:0) | (co>>9))&1)<<16) | (co&(~(1<<16)));
        sub = (((~((a>>17) ^ (co>>16)))&0x1f)<<17) | (sub&(~(0x1f<<17)));
        co = ((((a>>17) | (co>>16))&0x1f)<<17) | (co&(~(0x1f<<17)));
        sub = (((~((a>>22) ^ (co>>21)))&1)<<22) | (sub&(~(1<<22)));
    }while(temp!=co);

    return sub;
}

int SUBSIZE_imp(
                //int &sub,//'23'	=	subq'23'/*OUT*/,
                int a,//'23'	=	pc'23'/*IN*/,
                int b//[0..2]	=	qs\[0..2]/*IN*/,
            )
{
    return (a+b-8)&0x7fffff;
}

void SUBSIZE_test()
{
    for(int i=0;i<0x4000000;i++)
    {
        int r1=SUBSIZE(i&0x7fffff,(i>>22)&7);
        int r2=SUBSIZE_imp(i&0x7fffff,(i>>22)&7);
        if(r1!=r2)
        {
            qDebug() << QString::number(r1,16) << QString::number(r2,16)
                     << QString::number((i&0x7fffff),16) << ((i>>22)&7);
            break;
        }
    }
}

void SRCDGEN(
            int &locdent,	/// instruction source data select
            uint32 &locsrc,	/// source data from instruction
            uint32 program_count,	/// corrected program count value
            int srcdat, //[0..3]/*IN*/,	/// source data control bits
            int srcop   //'5'/*IN*/,	/// source operand field
        )
{
    int srcdat_ = (~srcdat)&0xf;
    int srcop_4_ = (~(srcop>>4))&1;
    /// enable for locally generated data
    locdent = srcdat?1:0;
    /// source operand sign in type 5
    int type5 = (srcdat==5)?1:0;
    /// all ones in type 3 or 6
    int oneselt = (srcdat ^ (srcdat>>2))&1;
    int onesel = (oneselt & (srcdat>>1) & (srcdat_>>3))&1;
    int topsrc = (onesel | ((srcop>>4) & type5))&1;
    /// Bit 7 can follow the top, or be a one if the operand is all
    ///zero in type 2
    int type2 = (srcdat==2)?1:0;
    int opzero = srcop==0?1:0;
    int srcb7 = type2?opzero:topsrc;
    /// Bit 6 can follow the top or be the top bit of the operand in
    ///type 2
    int srcb6 = type2?((srcop>>4)&1):topsrc;
    /// Bit 5 can follow the top, or be the second to top bit of the
    ///operand in type 2, or be 1 if the operand is zero in type 8
    int type8 = (srcdat==8)?1:0;
    int srcb5m[]={topsrc,opzero,(srcop>>3)&1,0};
    int srcb5 = srcb5m[type8|(type2<<1)];
    /// constant bits are set for type 6 (clear for type 4)
    int type6 = (srcdat==6)?1:0;
    /// constant is selected for types 4 and 6
    int constsel = (srcdat_ & (srcdat>>2) & (srcdat_>>3))&1;
    int xconst = (-type6)&0x1f;
    int opshft = (srcop&7)<<2;
    int botsrcm[]={srcop,xconst,opshft,opshft};
    int botsrc = botsrcm[constsel|(type2<<1)];
    /// therefore locally generated source data
    uint32 gensrc = botsrc|(srcb5<<5)|(srcb6<<6)|(srcb7<<7)|((-topsrc)&0xffffff00);
    /// Generate bit mask
    uint32 maskt_= (~((1 << (srcop&0xf)) & (((srcop>>4)&1)?0:-1)))&0xffff;
    maskt_ |= ((~((1 << (srcop&0xf)) & (srcop_4_?0:-1)))&0xffff)<<16;
    int type9 = (srcdat==9)?1:0;
    uint32 mask = maskt_ ^ (-type9);
    /// Data enabled here can be:
    ///0	the operand/constant data
    ///1	the bit mask
    ///2	the program counter
    ///sdsel0	= types 9 or 10
    ///sdsel1	= type 7
    ///
    int type7 = (srcdat==7)?1:0;
    int type10 = (srcdat==10)?1:0;
    int sdsel = type9 | type10 | (type7<<1);
    uint32 locsrcm[]={gensrc,mask,program_count,program_count};
    locsrc = locsrcm[sdsel];
}

uint32 bitget(uint32 val,int bit, int count=1)
{
    uint32 mask=(1<<count)-1;
    if(count>31 || count<=0)mask=-1;
    return (val>>bit)&mask;
}

void bitset(uint32 &dst,int bit,uint32 ready_src,int count=1)
{
    uint32 mask=(1<<count)-1;
    if(count>31 || count<=0)mask=-1;
    dst&=~(mask<<bit);
    dst|=(ready_src&mask)<<bit;
}

uint32 normi_fun(uint32 srcdp)
{
    /// The normalisation integer generator
    ///   -----------------------------------
    ///This is the logic that produces a value corresponding
    ///to how de-normalised an unsigned integer is.

    uint32 topset=0,inh=0,normi=0;
    uint32 tmp_topset,tmp_inh,tmp_normi;

    do
    {
        tmp_topset=topset;
        tmp_inh=inh;
        tmp_normi=normi;

        /// single out the top set bit
        bitset(topset,31,bitget(srcdp,31));
        bitset(inh,30, bitget(srcdp,30) | bitget(topset,31));
        bitset(topset,30, (~( (~(srcdp>>30)) | (topset>>31)))&1);
        bitset(inh,25, bitget(srcdp,25,29-25+1) | bitget(inh,26,30-26+1),29-25+1);
        bitset(topset,25, (~( (~(srcdp>>25)) | (inh>>26)))&0x1f, 29-25+1);
        bitset(topset,24, (~( (~(srcdp>>24)) | (inh>>25)))&1);
        bitset(inh,24, (srcdp&0xff000000)?1:0);
        bitset(inh,17, bitget(srcdp,17,23-17+1) | bitget(inh,18,24-18+1), 23-17+1);
        bitset(topset,17, (~( (~(srcdp>>17)) | (inh>>18)))&0x7f, 23-17+1);
        bitset(topset,16, (~( (~(srcdp>>16)) | (inh>>17)))&1);
        bitset(inh,16, ((srcdp&0xff0000) || (inh&(1<<24)))?1:0);
        bitset(inh,9, bitget(srcdp,9,15-9+1) | bitget(inh,10,16-10+1), 15-9+1);
        bitset(topset,9, (~( (~(srcdp>>9)) | (inh>>10)))&0x7f, 15-9+1);
        bitset(topset,8, (~( (~(srcdp>>8)) | (inh>>9)))&1);
        bitset(inh,8, ((srcdp&0xff00) || (inh&(1<<16)))?1:0);
        bitset(inh,1, bitget(srcdp,1,7-1+1) | bitget(inh,2,8-2+1),7-1+1);
        bitset(topset,1, (~( (~(srcdp>>1)) | (inh>>2)))&0x7f,7-1+1);
        bitset(topset,0, (~( (~srcdp) | (inh>>1)))&1);
        /// encode which the most significant bit set is
        bitset(normi,0, ((topset>>30)|(topset>>28)|(topset>>26)|(topset>>24)|(topset>>22)|(topset>>20)|(topset>>18)|(topset>>16)|(topset>>14)|(topset>>12)|(topset>>10)|(topset>>8)|(topset>>6)|(topset>>4)|(topset>>2)|topset)&1);
        bitset(normi,1, ((topset>>29)|(topset>>30)|(topset>>25)|(topset>>26)|(topset>>21)|(topset>>22)|(topset>>17)|(topset>>18)|(topset>>13)|(topset>>14)|(topset>>9)|(topset>>10)|(topset>>5)|(topset>>6)|(topset>>1)|(topset>>2))&1);
        bitset(normi,2, ((topset>>27)|(topset>>28)|(topset>>29)|(topset>>30)|(topset>>19)|(topset>>20)|(topset>>21)|(topset>>22)|(topset>>11)|(topset>>12)|(topset>>13)|(topset>>14)|(topset>>3)|(topset>>4)|(topset>>5)|(topset>>6))&1);
        bitset(normi,3, ((topset>>31)|(topset>>15)|(topset>>16)|(topset>>17)|(topset>>18)|(topset>>19)|(topset>>20)|(topset>>21)|(topset>>22)|topset|(topset>>1)|(topset>>2)|(topset>>3)|(topset>>4)|(topset>>5)|(topset>>6))&1);
        bitset(normi,4, ((normi>>5) & (inh>>7))&1);
        bitset(normi,5, (~(inh>>23))&1);
        normi=(normi&0x3f)|((-bitget(normi,5))<<6);

    }while(tmp_topset!=topset || tmp_inh!=inh || tmp_normi!=normi);

    return normi;
}

uint32 normi_imp(uint32 srcdp)
{
    uint32 rv = 0;

    if(!srcdp)return 0xffffffe0;

    if(srcdp&0xff000000)
    {
        while(srcdp&0xff000000)
        {
            srcdp>>=1; rv++;
        }
    }
    else
    {
        while(!(srcdp&0xff800000))
        {
            srcdp<<=1; rv--;
        }
    }
    return rv;
}

void normi_test()
{
    for(uint64 i=0;i<0x100000000;i++)
    {
        if(normi_fun(i)!=normi_imp(i))
        {
            qDebug() << QString::number(i,16) << QString::number(normi_fun(i),16) << QString::number(normi_imp(i),16);
            break;
        }
    }
}

uint8 alu_accum(int mulqpsgn, int alu_co, uint8 accum)
{
    int alu_co_ = (~alu_co)&1;
    int accuma_32,accuma_36, aco_[2],aco0_[2],aco1_[2],g[2],p[2];
    FA4CS(accuma_32,aco_[0],aco0_[0],aco1_[0],g[0],p[0],alu_co_,1,0,accum&15,(-mulqpsgn)&15);
    FA4CS(accuma_36,aco_[1],aco0_[1],aco1_[1],g[1],p[1],aco_[0],aco0_[0],aco1_[0],accum>>4,(-mulqpsgn)&15);
    return accuma_32|(accuma_36<<4);
}

uint8 alu_accum_imp(int mulqpsgn, int alu_co, uint8 accum)
{
    return accum+alu_co-mulqpsgn;
}

void alu_accum_test()
{
    for(int i=0;i<0x400;i++)
    {
        uint8 r1=alu_accum(i>>9,(i>>8)&1,i&0xff);
        uint8 r2=alu_accum_imp(i>>9,(i>>8)&1,i&0xff);
        if(r1!=r2)
        {
            qDebug() << QString("!!! %1!=%2, %3 %4 %5").arg(r1).arg(r2).arg(i>>9).arg((i>>8)&1).arg(i&0xff);
            break;
        }
    }
}

void SADD4(
            int &z,//[0..3]	=	rmwd2[8..11]/*IO*/,
            int a,//[0..3]	=	rmwd1[8..11]/*IN*/,
            int b//[0..3]	=	lbwd[8..11]/*IN*/,
        )
{
    //S0:=ADD4(s[0..3]/*OUT*/,co[3]/*OUT*/,a[0..3]/*IN*/,b[0..3]/*IN*/,0);
    int s=a+b;
    int overflow = ((s>>4) ^ (b>>3))&1;
    z = (overflow?(-((s>>4)&1)):s)&15;
}

int HA9(
            int a,//[0..8]	=	newdata[12..20]/*IN*/,
            int ci//	=	dc[11]/*IN*/,
        )
{
    int c = ci;
    int q[9];
    q[0] = (a ^ c)&1;
    int q1t = (~(a & c))&1;
    q[1] = (~((a>>1) ^ q1t))&1;
    int q2t = (~((a>>1) & a & c))&1;
    q[2] = (~((a>>2) ^ q2t))&1;
    int q3t = (~((a>>2) & (a>>1) & a & c))&1;
    q[3] = (~((a>>3) ^ q3t))&1;
    int q4t = (~(1 & 1 & 1 & (a>>3) & (a>>2) & (a>>1) & a & c))&1;
    q[4] = (~((a>>4) ^ q4t))&1;
    int q5t = (~(1 & 1 & (a>>4) & (a>>3) & (a>>2) & (a>>1) & a & c))&1;
    q[5] = (~((a>>5) ^ q5t))&1;
    int q6t = (~(1 & (a>>5) & (a>>4) & (a>>3) & (a>>2) & (a>>1) & a & c))&1;
    q[6] = (~((a>>6) ^ q6t))&1;
    int q7t = (~((a>>6) & (a>>5) & (a>>4) & (a>>3) & (a>>2) & (a>>1) & a & c))&1;
    q[7] = (~((a>>7) ^ q7t))&1;
    int q8t = (~((a>>7) & (a>>6) & (a>>5) & (a>>4) & (a>>3) & (a>>2) & (a>>1) & a & c))&1;
    q[8] = (~((a>>8) ^ q8t))&1;
    return q[0]|(q[1]<<1)|(q[2]<<2)|(q[3]<<3)|(q[4]<<4)|(q[5]<<5)|(q[6]<<6)|(q[7]<<7)|(q[8]<<8);
}

int HA9_imp(
            int a,//[0..8]	=	newdata[12..20]/*IN*/,
            int ci//	=	dc[11]/*IN*/,
        )
{
    return (a+ci)&0x1ff;
}

void HA9_test()
{
    for(int i=0;i<0x400;i++)
    {
        if(HA9_imp(i&0x1ff,i>>9)!=HA9(i&0x1ff,i>>9))
            qDebug() << "HA9 Error!!!!!!!!!!";
    }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
//    qDebug() << "ADDRCOMP_test()";
//    ADDRCOMP_test();

    //BARREL32_test();

//    for(int modx=0;modx<8;modx++)
//    {
//        int maskbit=((1<<modx)>>1)&0x20;
//        int masksel=((1<<modx)>>1)&0x1f;
//        int temp;

//        do
//        {
//            temp=maskbit;
//            maskbit=(maskbit&0x20)|(masksel|(maskbit>>1));
//        }while(temp!=maskbit);

//        qDebug() << modx << QString::number(~maskbit,16);
//    }

    //uint32 rm, q=divAlgo(1003<<16,9<<16,rm,true);
    //qDebug() << (q>>16) << (((int)rm));

    //HA9_test();

    parceNetLists("./TOM");
    //parceNetLists("./JERRY");
}

MainWindow::~MainWindow()
{

}

QStringList getAllFiles(QString path, const QStringList & nameFilters, QDir::Filters filters)
{
    QStringList rv;
    QDir dir(path);
    rv=dir.entryList(nameFilters,filters);

    for(int i=0;i<rv.size();i++)
        rv[i]=path+"/"+rv[i];

    QStringList list=dir.entryList(QDir::NoDotAndDotDot|QDir::Dirs);
    for(int i=0;i<list.size();i++)
    {
        rv+=getAllFiles(path+"/"+list[i],nameFilters,filters);
    }
    return rv;
}

void MainWindow::makeTranslationFile(QString path)
{
    AXObject tr;
    ATSet<AString> set;

    QStringList filt;
    filt << "*.cpp" << "*.h";

    QStringList src=getAllFiles(path,filt,QDir::Files);

    for(int i=0;i<src.size();i++)
    {
        ALexPreprocessor prep;
        prep.parce(src[i].toUtf8().data());
        set.insert(prep.findStringsForTranslation());
    }

    if(set.size())
    {
        for(int i=0;i<set.size();i++)
        {
            AXObject *text=tr.addItem("text");
            text->Item("origin")->setContent(set[i]());
            text->Item("translation")->setContent(set[i]());
        }
    }

    tr.setAttribute("target","PhoenixEmuProject");
    tr.save(path+"/translation.xml");

}



void MainWindow::parceNetLists(QString path)
{
    QStringList filt;
    filt << "*.NET";

    QStringList src=getAllFiles(path,filt,QDir::Files);

    NetListReader nr;

    nr.addDefine("TOSHIBA");
    nr.excludeImport("enumerates");
    nr.excludeImport("dsp_enum");

    for(int i=0;i<src.size();i++)
    {
        //qDebug() << src[i].toUtf8().data();
        nr.parce(src[i].toUtf8().data());
    }
    nr.syntaxParce();
    //nr.debugPrint();
    nr.resaveSyntax();

    nr.makePrimitive("MX6");
    nr.makePrimitive("ANR2P");
    nr.makePrimitive("ANR2");
    nr.makePrimitive("EN");
    nr.makePrimitive("ANR1");
    nr.makePrimitive("OAN1P");
    nr.makePrimitive("IVDM");
    nr.makePrimitive("AOR1");
    nr.makePrimitive("D38GH");
    nr.makePrimitive("ANR1P");
    nr.makePrimitive("AOR1P");
    nr.makePrimitive("MXI2");
    nr.makePrimitive("DECL38E");
    nr.makePrimitive("DECH38");
    nr.makePrimitive("DECH38EL");
    nr.makePrimitive("D38H");
    nr.makePrimitive("OND1");
    nr.makePrimitive("HS1");
    nr.makePrimitive("MX12B");
    nr.makePrimitive("OAN1");

    nr.makePrimitive("MAG_4");
    nr.makePrimitive("MAG_16");
    nr.makePrimitive("CMP8_INT");
    nr.makePrimitive("FA4CS");
    nr.makePrimitive("CG4");
    nr.makePrimitive("ADD16SAT");
    nr.makePrimitive("FAS16_S");
    nr.makePrimitive("FA332");
    nr.makePrimitive("ADD4");
    nr.makePrimitive("MAG4");
    nr.makePrimitive("MAG_15");
    nr.makePrimitive("ADDBMUX");
    nr.makePrimitive("ADDRCOMP");
    nr.makePrimitive("SRCSHIFT");
    nr.makePrimitive("LFU");
    nr.makePrimitive("ZEDSHIFT");
    nr.makePrimitive("DATACOMP");
    nr.makePrimitive("ZEDCOMP");
    nr.makePrimitive("DADDAMUX");
    nr.makePrimitive("DADDBMUX");
    nr.makePrimitive("LOCAL_MUX");
    nr.makePrimitive("DATA_MUX");
    nr.makePrimitive("ADDARRAY");
    nr.makePrimitive("ADDAMUX");
    nr.makePrimitive("DATAMUX");
    nr.makePrimitive("ADDRADD");
    nr.makePrimitive("ADDRGEN");
    nr.makePrimitive("BLITSTOP");
    nr.makePrimitive("COMP_CTRL");
    nr.makePrimitive("DCONTROL");
    nr.makePrimitive("DCOUNT16");
    nr.makePrimitive("OUTER_CNT");
    nr.makePrimitive("INNER_CNT");
    //nr.makePrimitive("BLIT");
    nr.makePrimitive("CMP6");
    nr.makePrimitive("AOR2");
    nr.makePrimitive("TBEN");
    nr.makePrimitive("TBENW");
    nr.makePrimitive("FA23");
    nr.makePrimitive("FA32_INT");
    nr.makePrimitive("CMP6I");
    nr.makePrimitive("ENP");
    nr.makePrimitive("D24H");
    nr.makePrimitive("MXI2P");
    nr.makePrimitive("D416G2L");
    nr.makePrimitive("GPU_RAM");
    nr.makePrimitive("ALU32");
    nr.makePrimitive("D416GH");
    nr.makePrimitive("BRL32");
    nr.makePrimitive("BRLSHIFT");
    nr.makePrimitive("SATURATE"); //diff in JERRY!!!
    nr.makePrimitive("MP16");
    nr.makePrimitive("DIVIDE");
    nr.makePrimitive("MACOUNT");
    nr.makePrimitive("MCOUNT");
    nr.makePrimitive("R1COUNT");
    nr.makePrimitive("SYSTOLIC");
    nr.makePrimitive("SUBSIZE");
    nr.makePrimitive("PC");
    nr.makePrimitive("PREFETCH");
    nr.makePrimitive("CNTE3");
    nr.makePrimitive("INTERRUPT");
    nr.makePrimitive("GPU_CTRL");
    nr.makePrimitive("GPU_CPU");
    nr.makePrimitive("SRCDGEN");
    nr.makePrimitive("ARITH");
    nr.makePrimitive("REGISTERS");
    nr.makePrimitive("MAG12");
    /*******************
void ACK_PIPE(
    latch	IO,
    latchd	IN,
    ack		IN,
    clk		IN,
    resetl	IN,
)q{
    q(clk) <= resetl ? ((q & (~ack)) | latchd) : 0;
    latch <= q & ack;
};
    ********************/
    nr.makePrimitive("ACK_PIPE"); //постщелчек

    /***************
void UPCNT(
    q	IO,
    co	IO,
    d	IN,
    clk	IN,
    ci	IN,
    ld	IN,
    resl	IN,
)q{
    q(clk) <= resl ? d2 : 0;
    d2 <= {d1,d}[ld];
    d1 <= q ^ ci;
    co <= ci & q;
};
    ****************/
    nr.makePrimitive("UPCNT"); //однобитный прибавляет ci
    nr.makePrimitive("UPCNT1"); //аналогично (без загрузки)
    nr.makePrimitive("UPCNTS");
    nr.makePrimitive("DNCNT"); //аналогично, только вычетает
    nr.makePrimitive("UDCNT1"); //однобитовый декремент-инкремент
    nr.makePrimitive("UDCNT");
    //nr.makePrimitive("GRAPHICS");
    //nr.makePrimitive("DSP");
    //nr.makePrimitive("OB");
    nr.makePrimitive("BD8T");
    nr.makePrimitive("BD4T");
    nr.makePrimitive("BD16T");
    nr.makePrimitive("BD2T");
    nr.makePrimitive("IBUF");
    nr.makePrimitive("STLATCH");
    nr.makePrimitive("DNCNTS");
    nr.makePrimitive("FJK2");
    nr.makePrimitive("FJKR");
    nr.makePrimitive("CREG11"); //проверяет на эквивалентность

    //пока не будем рассматривать работу с памятью
    //nr.makePrimitive("MEM");
    nr.makePrimitive("ABUS");
    nr.makePrimitive("DBUS");
    //nr.makePrimitive("MISC");
    nr.makePrimitive("CLK");
    nr.makePrimitive("SADD4");
    nr.makePrimitive("SADD8");
    nr.makePrimitive("HA9");
    nr.makePrimitive("CRYRGB");
    nr.makePrimitive("LBUF");
    nr.makePrimitive("IODEC");
    nr.makePrimitive("PIX");
    //nr.makePrimitive("VID");

    nr.makePrimitive("BT8");

    nr.compileElement("TOM");
}
