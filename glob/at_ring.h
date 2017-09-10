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

#ifndef AT_RING_H
#define AT_RING_H

template <class T>
class ATRing
{
 private:
        T *buff;
        volatile int len,up,down;
 public:
        ATRing(){buff=0;len=1;up=down=0;}
        ATRing(int size){buff=new T[size+1];len=size+1;up=down=0;}
        ~ATRing(){if(buff)delete []buff;}

        void Resize(int size)
        {
            if(buff)delete []buff;
            buff=new T[size+1];len=size+1;up=down=0;
        }

        int Read(T *data, int size)
        {
            int cnt=Size();
            if(cnt>size)cnt=size;
            for(int i=0;i<cnt;i++)
            {
                data[i]=Get();
            }
            return cnt;
        }

        bool WriteBlock(const T *block, int size)
        {
            if(Allow()<size)return false;
            for(int i=0;i<size;i++)
            {
                buff[up]=block[i];
                up=(up+1)%len;
            }
            return true;
        }

        int Limit(){return len-1;}

        void forcedPush(const T &val) //не для многопоточного применения!
        {
            if(!Allow())Get();
            buff[up]=val;
            up=(up+1)%len;
        }

        bool Push(const T &val)
        {
            if(!Allow())return false;
            buff[up]=val;
            up=(up+1)%len;
            return true;
        }

        int Size() const
        {
            volatile int tup=up,tdown=down;
            if(tup>=tdown)return tup-tdown;
            return len-tdown+tup;
        }

        //размер линейного блока для вычитывания
        int blockSizeToRead()
        {
            volatile int tup=up,tdown=down;
            if(tup>=tdown)return tup-tdown;
            return len-tdown;
        }

        //размер линейного блока для записи
        int blockSizeToWrite()
        {
            volatile int tup=up,tdown=down;
            if(tup>=tdown)return len-tup-(tdown?0:1);
            return tdown-tup-1;
        }

        //указатель на начало данных
        T* startPoint()
        {
            return &buff[down];
        }

        //указатель на место записи
        T* afterPoint()
        {
            return &buff[up];
        }

        T Get()
        {
         T rv;
            rv=buff[down];
            if(!Size())return rv;
            down=(down+1)%len;
            return rv;
        }

        int Allow()
        {
            if(!buff)return 0;
            volatile int tup=up,tdown=down;
            if(tup<tdown)return tdown-tup-1;
            return len-tup+tdown-1;
        }
        T& operator[](int ind){return buff[(down+ind)%len];}
        void Free(int size=-1)
        {
            if(size<0)down=up;
            else down=(down+size)%len;
        }
        void Added(int size)
        {
            up=(up+size)%len;
        }
};

#endif // AT_RING_H
