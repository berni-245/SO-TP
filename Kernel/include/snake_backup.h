/*
#ifndef _SNAKE2_H
#define _SNAKE2_H

#include "videoDriver.h"
#include "timer.h"
#include "keyboard.h"
#define S_HEIGHT 30
#define S_WIDTH 30
#define MAX_LEN 20
#define SQUARE 15
#define S1 1
#define S2 2
#define APPLE 3
#define ESC 27

typedef struct bodyT{
    int x, y, len, score, dirx, diry, tail_x, tail_y;
    uint32_t color;
    //struct bodyT * tail;
}bodyT;

typedef bodyT snakeT[MAX_LEN];


void snake_main2();
void startGrid();
void createSnake();
void growSnake(int x, int y, snakeT s);
void snake_input();
void draw();
void moveSnake(snakeT s);
void collision();
void appleGen();
int eaten(snakeT s);
void displayScore();
void reset();

#endif
*/