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

#include "axobject.h"
#include "afile.h"

AXObject::AXObject()
{
    parent=NULL;
    __name="root";
}

AXObject::~AXObject()
{
    if(parent) //есть родитель?
    { //чистим ссылки
        for(int i=0;i<parent->item_order.size();i++)
        {
            if(parent->item_order[i]==this)
            {
                parent->item_order.fastCut(i);
                break;
            }
        }
        if(parent->items.contains(__name))
        {
            ATArray<AXObject*> *list=&parent->items[__name];
            for(int i=0;i<list->size();i++)
            {
                if((*list)[i]==this)
                {
                    list->fastCut(i);
                    if(!list->size())parent->items.remove(__name);
                    break;
                }
            }
        }
    }
    clear(true);
}

AXObject::AXObject(const AXObject &val)
{
    parent=NULL;
    *this=val;
}

AXObject::AXObject(const AString &name)
{
    parent=NULL;
    __name=name;
}

AXObject& AXObject::operator=(const AXObject &val)
{
    clear();

    if(!parent)__name=val.__name;
    __content=val.__content;

    ATArray<ATArray<AXObject*> > list=val.items.values();
    ATArray<AString> listk=val.items.keys();
    for(int i=0;i<list.size();i++)
    {
        items.insert(listk[i],ATArray<AXObject*>());
        for(int j=0;j<list[i].size();j++)
        {
            AXObject *obj=new AXObject(*list[i][j]);
            obj->parent=this;
            items[listk[i]].append(obj);
            item_order.append(obj);
        }
    }

    ATArray<AVariant> atvlist = val.attributes.values();
    ATArray<AString> atklist = val.attributes.keys();
    for(int i=0;i<atvlist.size();i++)
    {
        //!!!deep copy needed???
        attributes.insert(atklist[i],atvlist[i]);
    }

    attr_order.append(val.attr_order);

    return *this;
}

bool AXObject::load(AString fname)
{
    AFile file(fname);
    if(!file.open(AFile::OReadOnly))return false;

    AString data = file.readText();
    file.close();

    clear();

    return _ameta_rxml_parcer(this,data);
}

bool AXObject::save(AString fname, bool standalone)
{
    return _ameta_rxml_streamer(this,fname,standalone);
}

void AXObject::moveBefor(AXObject *node, AXObject *befor)
{
    int inod=-1,ibef=-1;
    for(int i=0;i<item_order.size();i++)
    {
        if(item_order[i]==node)inod=i;
        if(item_order[i]==befor)ibef=i;
    }

    if(inod<0 || ibef<0)return;

    if(inod<=ibef)
    {
        for(int i=inod+1;i<ibef;i++)
        {
            item_order[i-1]=item_order[i];
        }
        item_order[ibef-1]=node;
    }
    else
    {
        for(int i=inod;i>ibef;i--)
        {
            item_order[i]=item_order[i-1];
        }
        item_order[ibef]=node;
    }
}

AString AXObject::getPathToMe(AXObject *stop_node, bool with_order)
{
    if(this==stop_node)return AString();

    if(parent)
    {
        AString rv=parent->getPathToMe(stop_node,with_order);
        if(!rv.isEmpty())rv+="/";
        if(!parent->items.contains(__name))return __name;
        ATArray<AXObject*> list=parent->items[__name];
        for(int i=0;i<list.size();i++)
        {
            if(list[i]==this)
            {
                rv+=__name;
                if(with_order)rv+="<"+AString::fromInt(i)+">";
                return rv;
            }
        }
        return __name;
    }
    else
    {
        return __name;
    }
}

AString AXObject::getTransformPathToMe(const AString &tattrname)
{
    if(!haveAttribute(tattrname))return AString();
    if(parent)
    {
        AString rv=parent->getTransformPathToMe(tattrname)+"/";
        if(!parent->items.contains(__name))return AString();
        ATArray<AXObject*> list=parent->items[__name];
        for(int i=0;i<list.size();i++)
        {
            if(list[i]==this)
            {
                rv+=Attribute(tattrname).toString();
                return rv;
            }
        }
        return AString();
    }
    else
    {
        return AString();
    }
}

AString AXObject::getPathToMe(bool with_order)
{
    if(parent)
    {
        AString rv=parent->getPathToMe(with_order)+"/";
        if(!parent->items.contains(__name))return __name;
        ATArray<AXObject*> list=parent->items[__name];
        for(int i=0;i<list.size();i++)
        {
            if(list[i]==this)
            {
                rv+=__name;
                if(with_order)rv+="<"+AString::fromInt(i)+">";
                return rv;
            }
        }
        return __name;
    }
    else
    {
        return __name;
    }
}

void AXObject::clear(bool with_name)
{
    attr_order.clear();
    item_order.clear();

    if(with_name)__name.clear();
    __content.clear();
    attributes.clear();
    ATArray<ATArray<AXObject*> > list=items.values();
    for(int i=0;i<list.size();i++)
    {
        for(int j=0;j<list[i].size();j++)
        {
            delete list[i][j];
        }
    }
    items.clear();
}

AString AXObject::parcePath(AString path, AString &next)
{
    int ind=path.indexOf('/');
    if(ind<0){next="";return path;}
    next=path.right(ind+1);
    return path.left(ind);
}

AVariant AXObject::parceConst(AString val)
{
    bool ok;
    if(val[0]!='\"') //число
    {        
        int ival=val.toInt<int>(10,&ok);
        if(ok)return ival;
        return AVariant();
    }
    else //строка
    {
        val=val.mid(1,val.size()-2);
        ATArray<AString> list = val.split('#');
        if(!list.size())return AVariant();
        AString rv=list[0];
        for(int i=1;i<list.size();i++)
        {
            if(list[i].size())
            {
                rv.append_unicode(list[i].left(4).toInt<uint32>(16,&ok));
                rv+=list[i].right(4);
            }
            else
            {
                i++;
                if(i<list.size())rv+="#"+list[i];
            }
        }
        return rv;
    }
}

int AXObject::parceCondition(AString &path_el, ATHash<AString, AVariant> &val_list)
{
    int start=path_el.indexOf('<');
    if(start<0)return 0;

    int end=path_el.indexOf('>',start+1);
    if(start==0 || end<0 || (start+1)==end)
    {
        path_el=path_el.left(start);
        return 0;
    }

    AString tmp=path_el.mid(start+1,end-start-1);
    path_el=path_el.left(start);

    //проверяем на индекс
    bool ok;
    int index=tmp.toInt<int>(10,&ok);
    if(ok)return index+1;

    //парсим на условия
    ATArray<AString> list = tmp.split(',',true);
    if(!list.size())return 0;

    for(int i=0;i<list.size();i++)
    {
        ATArray<AString> av = list[i].split('=');
        if(av.size()!=2 || av[0].isEmpty())continue;
        val_list.insert(av[0].simplified(),parceConst(av[1].trimmed()));
    }

    if(!val_list.size())return 0;
    return -1; //проверять атрибуты
}

AString AXObject::makeCode(charx val)
{
    return "#"+AString::fromIntFormat(val,4,16);
}

AString AXObject::makeCodedString(AString val, bool with_apostrofs)
{
    AString rv, rep(",=>\"/#");

    for(int i=0;i<val.size();i++)
    {
        if(rep.contains(val[i]))rv+=makeCode((uint8)val[i]);
        else rv.append(val[i]);
    }
    if(with_apostrofs)return "\""+rv+"\"";
    return rv;
}

void AXObject::delAttribute(AString name)
{
    for(int i=0;i<attr_order.size();i++)
    {
        if(attr_order[i]==name)
        {
            attr_order.fastCut(i);
            break;
        }
    }
    attributes.remove(name);
}

AXObject& AXObject::setAttribute(AString name, AVariant val)
{
    if(!attributes.contains(name))
    {
        attr_order.append(name);
    }
    attributes[name]=val;

    return *this;
}

void AXObject::setAttributes(ATHash<AString,AVariant> &list)
{
    for(int i=0;i<list.size();i++)
        setAttribute(list.keys()[i],list.values()[i]);
}

bool AXObject::haveAttributes(ATHash<AString,AVariant> &list)
{
    for(int i=0;i<list.size();i++)
    {
        AVariant attr=Attribute(list.keys()[i]);
        AVariant que=list.values()[i];
        if(attr!=que)return false;
    }
    return true;
}

AXObject* AXObject::ExistedItem(AString path)
{
    AString next;
    AXObject * rv;

    path=parcePath(path,next);

    if(path.isEmpty())return this;

    //проверка условия
    ATHash<AString, AVariant> val_list;
    int index=parceCondition(path,val_list);
    if(path.isEmpty())return this;
    if(!index) //безусловно
    {
        if(!items.contains(path))
        {
            return NULL;
        }
        else
        {
            rv=items[path][0];
        }
    }
    else if(index>0) //индексация
    {
        if(index>items[path].size())
        {
            return NULL;
        }
        rv=items[path][index-1];
    }
    else //условия атрибутов
    {
        if(!items.contains(path))
        {
            return NULL;
        }
        else
        {
            ATArray<AXObject*>  list=items[path];
            for(int i=0;i<list.size();i++)
            {
                if(list[i]->haveAttributes(val_list))
                    return list[i]->ExistedItem(next);
            }
            return NULL;
        }
    }
    return rv->ExistedItem(next);
}

AXObject* AXObject::Item(AString path)
{
    AString next;
    AXObject * rv;

    path=parcePath(path,next);

    if(path.isEmpty())return this;

    //проверка условия
    ATHash<AString, AVariant> val_list;
    int index=parceCondition(path,val_list);
    if(path.isEmpty())return this;
    if(!index) //безусловно
    {
        if(!items.contains(path))
        {
            rv=new AXObject(path);
            rv->parent=this;
            items[path].append(rv);
            item_order.append(rv);
        }
        else
        {
            rv=items[path][0];
        }
    }
    else if(index>0) //индексация
    {
        while(index>items[path].size())
        {
            rv=new AXObject(path);
            rv->parent=this;
            items[path].append(rv);
            item_order.append(rv);
        }
        rv=items[path][index-1];
    }
    else //условия атрибутов
    {
        if(!items.contains(path))
        {
            rv=new AXObject(path);
            rv->parent=this;
            items[path].append(rv);
            item_order.append(rv);
        }
        else
        {
            ATArray<AXObject*>  list=items[path];
            for(int i=0;i<list.size();i++)
            {
                if(list[i]->haveAttributes(val_list))return list[i]->Item(next);
            }
            rv=new AXObject(path);
            rv->parent=this;
            items[path].append(rv);
            item_order.append(rv);
        }
        rv->setAttributes(val_list);
    }
    return rv->Item(next);
}

ATArray<AXObject *> AXObject::ItemList(AString path)
{
    AString next;
    ATArray<AXObject*> rv;

    path=parcePath(path,next);
    ATHash<AString, AVariant> val_list;
    int index=parceCondition(path,val_list);

    if(path.isEmpty())return rv;

    if(!index) //безусловно
    {
        if(items.contains(path))
        {
            ATArray<AXObject*> list=items[path];
            for(int i=0;i<list.size();i++)
            {
                if(next.isEmpty())rv.append(list[i]);
                else rv.append(list[i]->ItemList(next));
            }
        }
    }
    else if(index>0) //индексация
    {
        if(items.contains(path))
        {
            if(index<=items[path].size())
            {
                if(next.isEmpty()) rv.append(items[path][index-1]);
                else rv.append(items[path][index-1]->ItemList(next));
            }
        }
    }
    else //условия атрибутов
    {
        if(items.contains(path))
        {
            ATArray<AXObject*> list=items[path];
            for(int i=0;i<list.size();i++)
            {
                if(list[i]->haveAttributes(val_list))
                {
                    if(next.isEmpty())rv.append(list[i]);
                    else rv.append(list[i]->ItemList(next));
                }
            }
        }
    }
    return rv;

}

AXObject* AXObject::addItem(const AXObject &val, AString path)
{
    AXObject *tmp,*obj=new AXObject(val);
    tmp=Item(path);
    obj->parent=tmp;
    tmp->items[obj->Name()].append(obj);
    tmp->item_order.append(obj);
    return obj;
}

AXObject* AXObject::addItem(const AString &name, AString path)
{
    AXObject *tmp,*obj=new AXObject(name);
    tmp=Item(path);
    obj->parent=tmp;
    tmp->items[obj->Name()].append(obj);
    tmp->item_order.append(obj);
    return obj;
}

void AXObject::delItems(AString path)
{
    ATArray<AXObject*> list=ItemList(path);
    for(int i=0;i<list.size();i++)
    {
        delete list[i];
    }
}

void AXObject::setContent(AVariant val)
{
    ATArray<ATArray<AXObject*> > list=items.values();
    for(int i=0;i<list.size();i++)
    {
        for(int j=0;j<list[i].size();j++)
        {
            delete list[i][j];
        }
    }
    __content=val;
}

///////////////////////////////////////////////////////////////////////////
/// \brief _ameta_rxml_parcer
/// \param obj
/// \param data
/// \return
///
///

#include "rxml/tinyxml.h"

bool _ameta_rxml_parcer_recu(AXObject *obj, TiXmlElement *el)
{
    if(!el)return true;
    AString node_name=el->Value();
    if(node_name.isEmpty())return true;

    obj->setName(node_name);

    TiXmlAttribute *attr = el->FirstAttribute();

    //позаботимся о корректном порядке аттрибутов
    ATHash<uint64,AString> attribs_names;
    ATHash<uint64,AString> attribs_values;
    while(attr)
    {

        AString attr_name=attr->Name();
        if(attr_name.isEmpty())break;

        attribs_names.insert((((uint64)attr->Row())<<32)|attr->Column(),attr_name);
        AString attr_val=attr->Value();
        attribs_values.insert((((uint64)attr->Row())<<32)|attr->Column(),attr_val);

        attr=attr->Next();
    }
    ATArray<uint64> keys=attribs_names.keys();
    for(int i=0;i<keys.size();i++)
    {
        obj->setAttribute(attribs_names[keys[i]],attribs_values[keys[i]]);
    }

    //грузим потомков
    bool no_child=true;
    TiXmlElement *child=el->FirstChildElement();
    while(child)
    {
        AString child_name=child->Value();
        if(!_ameta_rxml_parcer_recu(obj->addItem(child_name),child))
        {
            return false;
        }
        child = child->NextSiblingElement();
        no_child=false;
    }

    if(no_child)
    {
        //грузим текстовое содержимое
        AString text = el->GetText();
        if(!el->isStrongClosed())
        {
            obj->setContent(text);
        }

    }

    return true;
}

bool _ameta_rxml_parcer(AXObject *obj, AString &data)
{
    TiXmlDocument doc;

    doc.Parse(data());
    if(doc.ErrorId())
    {
        return false;
    }

    return _ameta_rxml_parcer_recu(obj,doc.FirstChildElement());
}

/////////////////////////////////////////////////////////////////////////////////////

bool _ameta_rxml_streamer_recu(AXObject *obj, TiXmlElement *el)
{
    ATArray<AString> list=obj->listAttributes();
    for(int i=0;i<list.size();i++)
    {
        el->SetAttribute(list[i](),obj->Attribute(list[i]).toString()());
    }

    if(obj->Type()==AXObject::SIMPLE)
    {
        TiXmlText *text=new TiXmlText(obj->Content().toString()());
        el->LinkEndChild(text);
    }
    else
    {
        ATArray<AXObject*> items=obj->listAllItems();
        for(int i=0;i<items.size();i++)
        {
            TiXmlElement *element = new TiXmlElement( items[i]->Name()() );
            _ameta_rxml_streamer_recu(items[i],element);
            el->LinkEndChild(element);
        }
    }

    return true;
}

bool _ameta_rxml_streamer(AXObject *obj, AString &fname, bool standalone)
{
    TiXmlDocument doc;

    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "utf-8", standalone?"yes":"" );
    TiXmlElement * element = new TiXmlElement( obj->Name()() );

    if(!_ameta_rxml_streamer_recu(obj,element))return false;

    doc.LinkEndChild( decl );
    doc.LinkEndChild( element );

    TiXmlPrinter printer;

    doc.Accept(&printer);

    // Create a std::string and copy your document data in to the string
    AString str = printer.CStr();

    if(!str.size())return false;

    AFile hand(fname);

    if(hand.open(AFile::OTruncate|AFile::OWriteOnly))
    {
        hand.write(str(),str.size());
    }

    return false;
}
