#ifndef AXOBJECT_H
#define AXOBJECT_H

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
#include "avariant.h"

//в некоторой мере повторяет структуру XML, данные упорядоченны и быстро доступны
class AXObject
{
public:

    //создание и удаление метаобъекта
    AXObject();
    AXObject(const AXObject &val);
    AXObject(const AString &name);
    ~AXObject();
    AXObject& operator=(const AXObject &val);
    void clear(bool with_name=false);
    bool load(AString fname);
    bool save(AString fname, bool standalone=false);

    //работа с иерархией
    AString getTransformPathToMe(const AString &tattrname);
    AString getPathToMe(bool with_order=false);
    AString getPathToMe(AXObject *stop_node, bool with_order=false);
    AXObject* Parent(){return parent;}
    void moveBefor(AXObject *node, AXObject *befor);

    //работа с атрибутами
    AVariant& Attribute(AString name, const AVariant &defval=AVariant()) //создаст объект
    {
        if(!attributes.contains(name))
        {
            attributes[name]=defval;
            attr_order.append(name);
        }
        return attributes[name];
    }
    bool haveAttribute(AString name){return attributes.contains(name);}
    AXObject& setAttribute(AString name, AVariant val);
    void setAttributes(ATHash<AString, AVariant> &list);
    bool haveAttributes(ATHash<AString, AVariant> &list);
    void delAttribute(AString name);

    /**************************************************
      Обращение к элеметам в виде простейшего запроса
      имя<Q>/имя<Q>/имя<Q>
      <Q> - указывает номер в массиве или условие эквивалентности атрибутов
            пример условий:
                el<15> - 15-й элемент
                el<a=12, b="test"> - атрибуты a должны иметь соответствующее значение
                    при этом текст не может явно содержать ( , = > "  / ), задаются они через #юникод, как и сама #
    **************************************************/
    AXObject* Item(AString path); //дает указатель для быстрого доступа (или создаст объект[ы])
    AXObject* ExistedItem(AString path);
    ATArray<AXObject*> ItemList(AString path); //даст список объектов
    AXObject* addItem(const AXObject &val, AString path="");
    AXObject* addItem(const AString &name, AString path="");
    void delItems(AString path);
    static AString makeCode(charx val);
    static AString makeCodedString(AString val, bool with_apostrofs=false);

    //основные атрибуты объекта
    AString Name(){return __name;} //имя узла
    void setName(AString val){__name=val;}

    AVariant Content(const AVariant &defval=AVariant()) //текстовое поле
    {
        if(items.size())return AVariant();
        if(!__content.isValid())return defval;
        return __content;
    }
    void setContent(AVariant val);

    enum NodeType
    {
        EMPTY,
        SIMPLE,
        COMPLEX
    };

    int Type()
    {
        if(items.size())return COMPLEX;
        if(__content.isValid())return SIMPLE;
        return EMPTY;
    }

    //списки атрибутов и элементов
    ATArray<AString> listAttributes(){return attr_order;}
    ATArray<AString> listItemNames(){return items.keys();}
    ATArray<AXObject*> listItemsByName(AString name)
        {if(items.contains(name))return items[name];return ATArray<AXObject*>();}
    ATArray<AXObject*> listAllItems(){return item_order;}
    int countItems(AString name){if(!items.contains(name))return 0;return items[name].size();}

private:

    AXObject *parent;
    AString __name;
    AVariant __content; //если установить - дочерние элементы изчезают и наоборот
    ATHash<AString, AVariant> attributes;
    ATHash<AString, ATArray<AXObject*> > items;

    ATArray<AString> attr_order;
    ATArray<AXObject*> item_order;

    //парсинг запросов
    static AString parcePath(AString path, AString &next);
    //парсинг элемента пути, вернет: 0..N - индекс, -1 - условие
    static int parceCondition(AString &path_el, ATHash<AString,AVariant> &val_list);
    static AVariant parceConst(AString val);

};

///////////////////////////////////////////////////////////////////
bool _ameta_rxml_parcer(AXObject *obj, AString &data);
bool _ameta_rxml_streamer(AXObject *obj, AString &fname, bool standalone=false);

#endif // AXOBJECT_H
