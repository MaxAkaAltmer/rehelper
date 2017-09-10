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

#ifndef AT_GRAPH_H
#define AT_GRAPH_H

#include "at_hash.h"

template<class NodeID, class NodeData, class LinkMeta>
class ATGraph
{
public:
    ATGraph(){}
    ATGraph(const ATGraph &val)
    {
        nodes=val.nodes;
        connects=val.connects;
    }
    ~ATGraph(){}

    ATGraph& operator=(const ATGraph &val)
    {
        nodes=val.nodes;
        connects=val.connects;
        return *this;
    }

    void clear()
    {
        connects.clear();
        nodes.clear();
    }

    ATArray<NodeID> idArray()
    {
        return nodes.keys();
    }

    ////////////////////////////////////////////////////////////
    //работа с узлами
    void insert(const NodeID &id, const NodeData &data)
    {
        nodes.insert(id,data);
    }
    int nodeCount() const
    {
        return nodes.size();
    }
    int nodeIndex(const NodeID &id) const
    {
        return nodes.indexOf(id);
    }
    NodeData nodeData(int ind) const
    {
        return nodes.value(ind);
    }
    NodeData& operator[](const NodeID &id)
    {
        return nodes[id];
    }
    bool contains(const NodeID &id)
    {
        return nodes.contains(id);
    }
    NodeID nodeID(int ind) const
    {
        return nodes.key(ind);
    }    
    ATSet<NodeID> nodeNodes(const NodeID &val) const
    {
        return connects.keysWith(val);
    }
    ATGraph& remove(NodeID id)
    {
        nodes.removeMulty(id);
        connects.removeMulty(id);
        return *this;
    }

    ////////////////////////////////////////////////////////////
    //работа со связями
    int link(const NodeID &n1, const NodeID &n2, const LinkMeta &data)
    {
        return connects.insert(n1,n2,data);
    }
    bool isLinked(const NodeID &n1, const NodeID &n2)
    {
        return connects.contains(n1,n2);
    }
    int linkCount() const
    {
        return connects.size();
    }
    ATArray<NodeID> linkNodes(int ind) const
    {
        return connects.key(ind);
    }
    LinkMeta linkMeta(int ind) const
    {
        return connects.value(ind);
    }
    ATArray<LinkMeta> nodeLinks(const NodeID &id) const
    {
        return connects.valuesWith(id);
    }
    ATArray<int> nodeLinkIndexes(const NodeID &id) const
    {
        return connects.indexesWith(id);
    }
    LinkMeta& operator()(int ind)
    {
        return connects[ind];
    }

    LinkMeta& operator()(const NodeID &n1, const NodeID &n2)
    {
        return connects.value(n1,n2);
    }

private:

    ATHash<NodeID,NodeData> nodes;
    ATChaos<NodeID,LinkMeta> connects;

};

#endif // AT_GRAPH_H
