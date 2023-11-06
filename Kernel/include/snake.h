#ifndef _SNAKE_H
#define _SNAKE_H

#include <videoDriver.h>
#include <timer.h>
#include <keyboard.h>
#include <interruptions.h>
#define S_HEIGHT 50
#define S_WIDTH 50
#define MAX_LEN 20
#define SQUARE 15
#define S1 1
#define S2 2
#define APPLE 3
#define ESC 27
#define ARRIBA -1
#define ABAJO 1
#define IZQUIERDA -1
#define DERECHA 1
#define QUIETO 0

typedef struct bodyT{
    int x, y, len, score, dirx, diry, tail_x, tail_y;
    int color;
}bodyT;

typedef bodyT snakeT[MAX_LEN];

void snake_main2();
void startGrid();
void createSnake(int player);
void growSnake(int x, int y, snakeT s);
void snake_input();
void draw();
void moveSnake(snakeT s);
void collision();
void appleGen(snakeT s);
int eaten(snakeT s);
void displayScore();
void reset();

#endif