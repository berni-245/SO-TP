#ifndef SNAKE_UTILS_H
#define SNAKE_UTILS_H

#include <colors.h>
#include <draw.h>
#include <keyboard.h>
#include <stdbool.h>
#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>
#include <time.h>

#define SNAKE_MAX_LEN 50
#define NAME_MAX_LEN 15
#define SCORE_MAX_DIGITS 5
#define CHAR_SPACE_UP 2
#define CHAR_SPACE_LEFT 3
#define ESC 27
#define UP (-1)
#define DOWN 1
#define LEFT (-1)
#define RIGHT 1
#define STOPPED 0

// typedef struct bodyT{
//     int x, y, len, score, dirx, diry, tail_x, tail_y;
//     int color;
// }bodyT;

typedef struct {
  int32_t x, y;
} Point;

typedef struct {
  Point body[SNAKE_MAX_LEN];
  int32_t len;
  int32_t score;
  int32_t dirX, dirY;
  int32_t color;
  char* name;
  int32_t nameLen;
  int32_t nameX, nameY;
  int32_t scoreX, scoreY;
} Snake;

typedef struct {
  int32_t rows;
  int32_t cols;
  int32_t cellSize;
  int32_t color;
  int32_t borderWidth;
  int32_t lineWidth;
  int32_t x0, y0;
  int32_t width, height;
} Grid;

void setGrid();
void drawGrid();
void fillGridCell(int32_t col, int32_t row);
void clearGridCell(int32_t col, int32_t row);

void setSnake(Snake* s, int32_t col, int32_t row, uint32_t color, char* name, int32_t scoreX, int32_t scoreY);
void growSnake(Snake* s);
bool moveInput();
bool specialKeyInput();
void drawSnake(Snake* s);
void moveSnake(Snake* s);
bool pointEquals(Point a, Point b);
bool snakeCollision(Snake*);
bool onSnake(Snake* s, Point p);
void appleGen();
int32_t eaten(Snake* s);
void updateScoreBoard();
void reset();
void changeDirections(Snake* snake, char input, char* movKeys);
void gameOver();

#endif
