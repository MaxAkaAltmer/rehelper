
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

#ifndef AT_STRING_UTILS_HEADER_DEFINITION
#define AT_STRING_UTILS_HEADER_DEFINITION  "A... Template String Utils v.1.0.7"

#define ATS_MINSIZE (sizeof(int)*8)
#define _ats_upsize(x) ( ((x+1)+((x)>>1))<ATS_MINSIZE ? ATS_MINSIZE : ((x+1)+((x)>>1)) )

template <class T>
unsigned int _ats_strlen(const T *Str)
{
    if(!Str)return 0;

    unsigned int i=0;
    while(Str[i]){i++;};
    return i;
}

template <class T>
void _ats_memcpy(T *dst, const T *src,int num)
{
    int i;
    if(dst<src)  //в случае перекрытий следует копировать с нужной стороны
    {
            for(i=0;i<num;i++)dst[i]=src[i];
    }
    else if(dst>src)
    {
            for(i=num-1;i>=0;i--)dst[i]=src[i];
    }
}

template <class T>
int _ats_memcmp(const T *dst, const T *src,int num)
{
    int i;
    for(i=0;i<num;i++)
    {
            if(dst[i]<src[i])return -1;
            if(dst[i]>src[i])return 1;
    }
    return 0;
}

template <class T>
int _ats_strcmp(const T *dst, const T *src)
{
    int i=0;
    while(dst[i] && src[i])
    {
            if(dst[i]<src[i])return -1;
            if(dst[i]>src[i])return 1;
            i++;
    }
    if(dst[i]<src[i])return -1;
    if(dst[i]>src[i])return 1;
    return 0;
}

template <class T>
void _ats_memset(T *dst, T c, int num)
{
    for(int i=0;i<num;i++)dst[i]=c;
}


#endif

