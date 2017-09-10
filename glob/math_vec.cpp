/*****************************************************************************

Copyright (C) 2009 Гришин Максим Леонидович (altmer@arts-union.ru)
          (C) 2009 Maxim L. Grishin  (altmer@arts-union.ru)

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

#include "math_vec.h"

#include <math.h>
#include <string.h>

/*
int _matherrl (struct _exceptionl *a)  //обработка исключений математики
{
  if (a->type == DOMAIN)
  {
        if(!strcmp(a->name,"atan2l"))
        {
                a->retval = 0.0;
                return 1;
        }
  }
  return 0;
}
*/

/*int _matherr (struct _exception *a)  //обработка исключений математики
{
  if (a->type == DOMAIN)
  {
        if(!strcmp(a->name,"atan2"))
        {
                a->retval = 0.0;
                return 1;
        }
  }
  return 0;
}*/

#ifndef ANDROID_NDK
real80  _vm_cos(real80 val){return cosl(val);}
real80  _vm_acos(real80 val){return acosl(val);}
real80  _vm_asin(real80 val){return asinl(val);}
real80  _vm_sin(real80 val){return sinl(val);}
real80  _vm_sqrt(real80 val){return sqrtl(val);}
real80  _vm_atan2(real80 y, real80 x){return atan2l(y,x);}
real80  _vm_fmod(real80 x, real80 y){return fmodl(x,y);}
#endif

real64  _vm_cos(real64 val){return cos(val);}
real32  _vm_cos(real32 val){return cos(val);}

real64  _vm_acos(real64 val){return acos(val);}
real32  _vm_acos(real32 val){return acos(val);}

real64  _vm_asin(real64 val){return asin(val);}
real32  _vm_asin(real32 val){return asin(val);}

real64  _vm_sin(real64 val){return sin(val);}
real32  _vm_sin(real32 val){return sin(val);}

real64  _vm_sqrt(real64 val){return sqrt(val);}
real32  _vm_sqrt(real32 val){return sqrt(val);}

real64  _vm_atan2(real64 y, real64 x){return atan2(y,x);}
real32  _vm_atan2(real32 y, real32 x){return atan2(y,x);}

real64  _vm_fmod(real64 x, real64 y){return fmod(x,y);}
real32  _vm_fmod(real32 x, real32 y){return fmod(x,y);}
