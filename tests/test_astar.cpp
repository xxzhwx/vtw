﻿#include "tests/test.h"

#include "astar/astar.h"

void Test_AStar()
{
    // 地图数据
    /**
    左上角为(0,0)
    右下角为(9,9)
    从左上角到右下角不允许走斜角的路径为：
    (0,1),(1,1),(2,1),(2,0),(3,0),(4,0),(4,1),(4,2),(4,3),(5,3),
    (6,3),(6,2),(6,1),(6,0),(7,0),(8,0),(8,1),(8,2),(8,3),(8,4),
    (8,5),(7,5),(6,5),(5,5),(4,5),(3,5),(2,5),(2,4),(2,3),(1,3),
    (0,3),(0,4),(0,5),(0,6),(0,7),(1,7),(2,7),(3,7),(3,8),(3,9),
    (4,9),(5,9),(5,8),(5,7),(6,7),(7,7),(7,8),(8,8),(9,8),(9,9)
    */
    char map[10][10] =
    {
        {0,1,0,0,0,1,0,0,0,0},
        {0,0,0,1,0,1,0,1,0,1},
        {1,1,1,1,0,1,0,1,0,1},
        {0,0,0,1,0,0,0,1,0,1},
        {0,1,0,1,1,1,1,1,0,1},
        {0,1,0,0,0,0,0,0,0,1},
        {0,1,1,1,1,1,1,1,1,1},
        {0,0,0,0,1,0,0,0,1,0},
        {1,1,0,0,1,0,1,0,0,0},
        {0,0,0,0,0,0,1,0,1,0},
    };

    // 搜索参数
    AStar::Params param;
    param.width = 10;
    param.height = 10;
    param.corner = false;
    param.start = AStar::Vec2(0, 0);
    param.end = AStar::Vec2(9, 9);
    param.canPass = [&](const AStar::Vec2 &pos) {
        return map[pos.y][pos.x] == 0; // y是行，x是列
    };

    // 寻路
    AStar algorithm;
    auto path = algorithm.Find(param);
    int steps = 0;
    for (auto pos : path)
    {
        ++steps;
        printf("%2d: %u,%u\n", steps, pos.x, pos.y);
    }
}
