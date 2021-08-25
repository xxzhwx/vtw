#include <assert.h>
#include <algorithm>
#include "astar/astar.h"

static const int kDefaultStepVal = 10;
static const int kDefaultObliqueVal = 14;

#define MIN(x,y) ((x)<(y) ? (x) : (y))
#define MAX(x,y) ((x)>(y) ? (x) : (y))
#define SAFE_DELETE(ptr) \
    if ((ptr) != nullptr) { \
        delete (ptr); \
        (ptr) = nullptr; \
    }

AStar::AStar() :
    stepVal_(kDefaultStepVal),
    obliqueVal_(kDefaultObliqueVal),
    width_(0),
    height_(0),
    canPass_(nullptr)
{
}

AStar::~AStar()
{
}

void AStar::Init(const Params &param)
{
    width_ = param.width;
    height_ = param.height;
    canPass_ = param.canPass;
    //mapping_.clear();
    //mapping_.resize(width_ * height_, nullptr);
}

void AStar::Clear()
{
    //for (auto &node : mapping_)
    //{
    //    SAFE_DELETE(node);
    //}

    for (auto &kv : mapping2_)
    {
        SAFE_DELETE(kv.second);
    }

    mapping2_.clear();
    openList_.clear();
    canPass_ = nullptr;
    width_ = 0;
    height_ = 0;
}

// 获取节点在二叉堆上的索引
bool AStar::__GetNodeIndex(std::vector<Node*> &heap, Node *node, size_t *index)
{
    *index = 0;
    const size_t size = heap.size();
    while (*index < size)
    {
        if (heap[*index]->pos == node->pos)
        {
            return true;
        }
        ++(*index);
    }
    return false;
}

// 二叉堆上滤
void AStar::__PercolateUp(std::vector<Node*> &heap, size_t index)
{
    size_t parent = 0;
    while (index > 0)
    {
        parent = (index - 1) / 2;
        if (heap[index]->f < heap[parent]->f)
        {
            std::swap(heap[index], heap[parent]);
            index = parent;
        }
        else
        {
            return;
        }
    }
}

inline uint16_t AStar::CalcGValue(Node *parent, const Vec2 &current)
{
    uint16_t g = current.Distance(parent->pos) == 2 ? obliqueVal_ : stepVal_;
    g += parent->g;
    return g;
}

inline uint16_t AStar::CalcHValue(const Vec2 &current, const Vec2 &end)
{
    uint16_t h = end.Distance(current);
    return h * stepVal_;
}

inline bool AStar::InOpenList(const Vec2 &pos, Node *&outNode)
{
    outNode = __GetMappingNode(pos);
    return outNode ? outNode->state == IN_OPENLIST : false;
}

inline bool AStar::InCloseList(const Vec2 &pos)
{
    Node *node = __GetMappingNode(pos);
    return node ? node->state == IN_CLOSELIST : false;
}

bool AStar::IsValidPos(const Vec2 &pos) const
{
    return (pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_);
}

bool AStar::CanPass(const Vec2 &pos) const
{
    return IsValidPos(pos) ? canPass_(pos) : false;
}

bool AStar::CanPass(const Vec2 &current, const Vec2 &destination, bool corner)
{
    if (IsValidPos(destination))
    {
        if (InCloseList(destination))
        {
            return false;
        }

        if (destination.Distance(current) == 1)
        {
            return canPass_(destination);
        }
        else if (corner)
        {
            return canPass_(destination)
                && CanPass(Vec2(destination.x, current.y))
                && CanPass(Vec2(current.x, destination.y));
        }
    }
    return false;
}

void AStar::FindCanPassNearbyNodes(const Vec2 &current, bool corner, std::vector<Vec2> *outList)
{
    Vec2 destination;
    const int maxRow = MIN(current.y + 1, height_ - 1);
    const int maxCol = MIN(current.x + 1, width_ -1);

    int rowIndex = MAX(current.y - 1, 0);
    while (rowIndex <= maxRow)
    {
        int colIndex = MAX(current.x - 1, 0);
        while (colIndex <= maxCol)
        {
            destination.Reset(colIndex, rowIndex);
            if (CanPass(current, destination, corner))
            {
                outList->push_back(destination);
            }
            ++colIndex;
        }
        ++rowIndex;
    }
}

void AStar::HandleFoundInOpenList(Node *current, Node *destination)
{
    uint16_t g = CalcGValue(current, destination->pos);
    if (g < destination->g)
    {
        destination->g = g;
        destination->f = destination->g + destination->h;
        destination->parent = current;

        size_t index = 0;
        if (__GetNodeIndex(openList_, destination, &index))
        {
            __PercolateUp(openList_, index);
        }
        else
        {
            assert(false);
        }
    }
}

void AStar::HandleNotFoundInOpenList(Node *current, Node *destination, const Vec2 &end)
{
    destination->parent = current;
    destination->g = CalcGValue(current, destination->pos);
    destination->h = CalcHValue(destination->pos, end);
    destination->f = destination->g + destination->h;

    __SetMappingNode(destination);

    destination->state = IN_OPENLIST;
    openList_.push_back(destination);
    std::push_heap(openList_.begin(), openList_.end(), NodeHeapCmp);
}

AStar::Node* AStar::__GetMappingNode(const Vec2 &pos) const
{
    //return mapping_[pos.y * width_ + pos.x];
    auto iter = mapping2_.find(pos.y * width_ + pos.x);
    return iter != mapping2_.end() ? iter->second : nullptr;
}

void AStar::__SetMappingNode(Node *node)
{
    //mapping_[node->pos.y * width_ + node->pos.x] = node;
    mapping2_[node->pos.y * width_ + node->pos.x] = node;
}

std::vector<AStar::Vec2> AStar::Find(const Params &param)
{
    std::vector<Vec2> paths;
    if (!param.IsValid())
    {
        assert(false);
        return paths;
    }

    Init(param);

    // 将起点放入开启列表
    Node *startNode = new Node(param.start);
    startNode->state = IN_OPENLIST;
    openList_.push_back(startNode);

    __SetMappingNode(startNode);

    // 寻路操作
    std::vector<Vec2> nearbyNodes;
    nearbyNodes.reserve(param.corner ? 8 : 4);

    while (!openList_.empty())
    {
        // 找出f值最小的节点（最小堆的根节点）
        Node *current = openList_.front();
        std::pop_heap(openList_.begin(), openList_.end(), NodeHeapCmp);
        openList_.pop_back();

        current->state = IN_CLOSELIST; // 放到关闭列表

        // 是否找到终点
        if (current->pos == param.end)
        {
            while (current->parent)
            {
                paths.push_back(current->pos);
                current = current->parent;
            }
            std::reverse(paths.begin(), paths.end());
            goto __end__;
        }

        // 查找周围可通过的节点
        nearbyNodes.clear();
        FindCanPassNearbyNodes(current->pos, param.corner, &nearbyNodes);

        // 计算周围节点的估值
        size_t index = 0;
        const size_t size = nearbyNodes.size();
        while (index < size)
        {
            Node *nextNode = nullptr;
            if (InOpenList(nearbyNodes[index], nextNode))
            {
                HandleFoundInOpenList(current, nextNode);
            }
            else
            {
                nextNode = new Node(nearbyNodes[index]);
                HandleNotFoundInOpenList(current, nextNode, param.end);
            }
            ++index;
        }
    }

__end__:
    Clear();
    return paths;
}
