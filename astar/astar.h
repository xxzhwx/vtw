#pragma once
#include <stdint.h>
#include <functional>
#include <vector>
#include <map>

/**
* A星寻路算法原理参考：
* http://www.cppblog.com/christanxw/archive/2006/04/07/5126.html
* 
* 可优化：
* #.使用对象池或内存池优化节点的内存分配
* #.使用专门的堆容器代替目前的堆实现或优化 __GetNodeIndex 避免里面的遍历
*/

class AStar final
{
public:
    /**
     * 二维向量
     */
    struct Vec2
    {
        uint16_t x;
        uint16_t y;

        Vec2() : x(0), y(0) {}
        Vec2(uint16_t x, uint16_t y) : x(x), y(y) {}

        void Reset(uint16_t x, uint16_t y)
        {
            this->x = x;
            this->y = y;
        }

        // 曼哈顿距离
        int Distance(const Vec2 &o) const
        {
            return abs(o.x - x) + abs(o.y - y);
        }

        bool operator==(const Vec2 &o) const
        {
            return x == o.x && y == o.y;
        }
    };

    //typedef std::function<bool(const Vec2&)> CanPassFunc;
    using CanPassFunc = std::function<bool(const Vec2&)>;

    /**
     * 搜索参数
     */
    struct Params
    {
        bool corner; //允许拐角
        uint16_t width; //地图宽度
        uint16_t height; //地图高度
        Vec2 start; //起点坐标
        Vec2 end; //终点坐标
        CanPassFunc canPass; //是否可通过

        Params() : corner(false), width(0), height(0) {}

        bool IsValid() const
        {
            return (canPass != nullptr
                && width > 0 && height > 0
                && end.x >= 0 && end.x < width
                && end.y >= 0 && end.y < height
                && start.x >= 0 && start.x < width
                && start.y >= 0 && start.y < height);
        }
    };

private:
    /**
     * 路径节点状态
     */
    enum NodeState
    {
        UNKNOWN, //未知
        IN_OPENLIST, //在开启列表（待搜索）
        IN_CLOSELIST, //在关闭列表（已搜索）
    };

    /**
     * 路径节点
     */
    struct Node
    {
        uint16_t f; // f = g + h
        uint16_t g; // 与起点的距离
        uint16_t h; // 与终点的估算距离
        Vec2 pos; // 节点的位置
        NodeState state; // 节点的状态
        Node* parent; // 父节点

        Node(const Vec2 &pos)
            : f(0), g(0), h(0), pos(pos), state(UNKNOWN), parent(nullptr)
        {
        }
    };

    using NodeHeapCmpType = std::function<bool(const Node *a, const Node *b)>;
    NodeHeapCmpType NodeHeapCmp = [](const Node *a, const Node *b)->bool {
        return a->f > b->f;
    };

public:
    AStar();
    ~AStar();

public:
    std::vector<Vec2> Find(const Params &param);

private:
    void Init(const Params &param);
    void Clear();

private:
    static bool __GetNodeIndex(std::vector<Node*> &heap, Node *node, size_t *index);
    static void __PercolateUp(std::vector<Node*> &heap, size_t index);

    uint16_t CalcGValue(Node *parent, const Vec2 &current);
    uint16_t CalcHValue(const Vec2 &current, const Vec2 &end);

    bool InOpenList(const Vec2 &pos, Node *&outNode);
    bool InCloseList(const Vec2 &pos);
    bool IsValidPos(const Vec2 &pos) const;
    bool CanPass(const Vec2 &pos) const;
    bool CanPass(const Vec2 &current, const Vec2 &destination, bool corner);

    void FindCanPassNearbyNodes(const Vec2 &current, bool corner, std::vector<Vec2> *outList);
    void HandleFoundInOpenList(Node *current, Node *destination);
    void HandleNotFoundInOpenList(Node *current, Node *destination, const Vec2 &end);

    Node* __GetMappingNode(const Vec2 &pos) const;
    void __SetMappingNode(Node *node);

private:
    int stepVal_; // 到相邻正交格子的g值
    int obliqueVal_; // 到相邻斜角格子的g值

    uint16_t width_;
    uint16_t height_;
    CanPassFunc canPass_;
    //std::vector<Node*> mapping_;
    std::map<size_t, Node*> mapping2_;
    std::vector<Node*> openList_; // 按节点f值比较的最小堆
};
