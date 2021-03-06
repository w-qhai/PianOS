#pragma once
#include "../lib/constants.h"
#include "layer.h"

/**
 * @brief 颜色代码
 * 0 黑
 * 1 蓝
 * 2 绿
 * 3 暗蓝
 * 4 红
 * 5 紫
 * 6 暗黄
 * 7 浅灰
 * 8 深灰
 * 9 浅亮蓝
 * a 亮绿
 * b 亮蓝
 * c 亮红
 * d 亮紫
 * e 黄
 * f 白色
 */
#define Black 0
#define Blue 1
#define Green 2
#define DarkBlue 3
#define Red 4
#define Purple 5
#define DarkYellow 6
#define LightGrey 7
#define DarkGrey 8
#define LightBlue 9
#define LightGreen 10
#define LightBlue2 11
#define LightRed 12
#define LightPurple 13
#define Yellow 14
#define White 15

struct Rect {
    int x, y;
    int w, h;
};

struct Point {
    int x, y;
};


void fill_box(int x, int y, int w, int h, int color, lm::Layer* layer);
void fill_box(Rect rect, int color, lm::Layer* layer);
void draw_cursor(int x, int y, lm::Layer* layer);
void draw_desktop(lm::Layer* layer);
