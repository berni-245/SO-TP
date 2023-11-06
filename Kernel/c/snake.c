
#include <snake.h>


//S_WIDTH = S_HEIGHT
int grid[S_WIDTH][S_HEIGHT];

static int gameover, read, exit, playertwo=0;
static char input;
KeyStruct buf[1];
snakeT s1;
snakeT s2;

int apple[2]={0, 0};

void printSq(int x, int y, uint32_t hexColor){
    setStrokeWidth(0);
    setStrokeColor(hexColor);
    setFillColor(hexColor);
    fillRectangle(x * SQUARE, y * SQUARE, SQUARE, SQUARE);
}

void startGrid(){
    int i;
    for(int j=0; j<S_HEIGHT; j++){
        for(i=0; i<S_WIDTH; i++)
            if(i==0 || i==S_WIDTH-1 || j==0 || j==S_HEIGHT-1){
                grid[i][j]=1;
            }
            else{
                grid[i][j] = 0;
            }
    }
}


void createSnake(int player){
    if(player ==1){
        s1[0].x = S_WIDTH/2-3;
        s1[0].y = S_HEIGHT/2;
        s1[0].dirx=0;
        s1[0].diry=0;
        s1[0].len=2;
        s1[0].color=S1;
        s1[0].score=0;
    }
    else{
        s2[0].x = S_WIDTH/2+3;
        s2[0].y = S_HEIGHT/2;
        s2[0].len=2;
        s2[0].dirx=0;
        s2[0].diry=0;
        s2[0].score=0;
        s2[0].color=S2;
        playertwo=1;    
    }
}

void growSnake(int x, int y, snakeT s){
    if(s[0].len==MAX_LEN)
        return;
    int len = s[0].len++;
    s[len-1].x=x; 
    s[len-1].y=y; 
}

void snake_input(){

    read = readKbBuffer(buf, 1);
    char prev_input = input;
    input = buf[0].key;
    if(input==prev_input)
        return;

    switch(input){
        case 'w': 
            if(s1[0].diry==QUIETO){
                s1[0].diry=ARRIBA;
                s1[0].dirx=QUIETO;
            }
            break;

        case 'a':
            if(s1[0].dirx==QUIETO){
                s1[0].diry=QUIETO;
                s1[0].dirx=IZQUIERDA;
            } 
            break;
        case 's': 
            if(s1[0].diry==QUIETO){
                s1[0].diry=ABAJO;
                s1[0].dirx=QUIETO;
            }
            break;
        case 'd': 
            if(s1[0].dirx==QUIETO){
                s1[0].diry=QUIETO;
                s1[0].dirx=DERECHA;
            }
            break;

        case 'x': gameover=1; break;

        case 'r': reset(); break;
        
        case 'o': createSnake(2); break;
        
        case 'i':
            if(s2[0].diry==QUIETO){
                s2[0].diry=ARRIBA;
                s2[0].dirx=QUIETO;
            }
            break;
        case 'j': 
            if(s2[0].dirx==QUIETO){
                s2[0].diry=QUIETO;
                s2[0].dirx=IZQUIERDA;
            } 
            break;
        case 'k': 
            if(s2[0].diry==QUIETO){
                s2[0].diry=ABAJO;
                s2[0].dirx=QUIETO;
            } 
            break;
        case 'l': 
            if(s2[0].dirx==QUIETO){
                s2[0].diry=QUIETO;
                s2[0].dirx=DERECHA;
            }
            break;
        
        case ESC: gameover=1; exit=1; break;
        default: break;
    }
}

void draw(){
    for(int j=1; j<S_HEIGHT; j++){
        for (int i=1; i<S_WIDTH; i++){
            if(grid[i][j]==S1){
                printSq(i, j, 0xFFFFFF);
            }
            else if(grid[i][j]==S2){
                printSq(i, j, 0x00FFFF);
            }
            else if (grid[i][j]==APPLE){
                printSq(i, j, 0xFF0000);
            }
            else{
                printSq(i, j, 0x000000);
            }
        }
    }
    for (int i=0; i<S_WIDTH; i++){
        printSq(i, 0, 0xFFFFFF);
        printSq(0, i, 0xFFFFFF);
    }
}

void moveSnake(snakeT s){
    int len = s[0].len;
    s[0].tail_x = s[len-1].x;
    s[0].tail_y = s[len-1].y;

    grid[s[0].tail_x][s[0].tail_y]=0;

    for(int i=len-1; i>0; i--){
        s[i].x=s[i-1].x;
        s[i].y=s[i-1].y;
    }
    s[0].x+=s[0].dirx;
    s[0].y+=s[0].diry;

    grid[s[0].x][s[0].y]=s[0].color;
}



void collision(){
    if(s1[0].x == S_WIDTH-1 || s1[0].x==0 || s1[0].y==S_HEIGHT-1 || s1[0].y==0){
        gameover=1;
    }
    for(int i=s1[0].len; i>1; i--){
        if(s1[0].x==s1[i].x && s1[0].y==s1[i].y){
            gameover=1;
        }
    }
    if(playertwo){
        if(s2[0].x == S_WIDTH-1 || s2[0].x==0 || s2[0].y==S_HEIGHT-1 || s2[0].y==0){
            gameover=1;
        }
        for(int i=s2[0].len; i>1; i--){
            if(s2[0].x==s2[i].x && s2[0].y==s2[i].y){
                gameover=1;
            }
        }
        //check if collision between snakes
        if(OnSnake(s2[0].x, s2[0].y, s1) || OnSnake(s1[0].x, s1[0].y, s2)){
            gameover=1;
        }
    }
}

unsigned short lfsr = 0xACE1u; 
unsigned bit; 
 
unsigned rand(){ 
    bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1; 
    return lfsr =  (lfsr >> 1) | (bit << 15); 
}

void appleGen(snakeT s){
    do{
        grid[s[0].x][s[0].y]=s[0].color;
        apple[0]=rand()%(S_WIDTH-1);
        apple[1]=rand()%(S_HEIGHT-1);
    } while (apple[0]==0 || apple[1]==0 || OnSnake(apple[0], apple[1], s1) 
                || OnSnake(apple[0], apple[1], s2));
    grid[apple[0]][apple[1]]=APPLE;
}

//0 if on snake | 1 if not
int OnSnake(int x, int y, snakeT s){

    for (int i=0; i<s[0].len; i++){
        if (x==s[i].x && y==s[i].y)
            return 1;
    }
    return 0;
}

int eaten(snakeT s){
    if (s[0].x==apple[0] && s[0].y==apple[1]){
        s[0].score++;
        int len=s[0].len;
        appleGen(s);
        growSnake(s[len-1].x, s[len-1].y,  s);
        return 1;
    }
    return 0;
}



void displayScore(){
    setFontColor(0x000000);
    if(s1[0].score<10){
        printChar(55, 0, s1[0].score+'0'-1);
    }
    else{
        printChar(55, 0, s1[0].score/10+'0'-1);
        printChar(55, 0, s1[0].score-10+'0'-1);
    }
    
    if(s2[0].score<10){
        printChar(55, 1, s2[0].score+'0'-1);
    }
    else{
        printChar(55, 1, s2[0].score/10+'0'-1);
        printChar(55, 1, s2[0].score-10+'0'-1);
    }

    setFontColor(0xFFFFFF);
    if(s1[0].score<10){
        printChar(55, 0, s1[0].score+'0');
    }
    else{
        printChar(55, 0, s1[0].score/10+'0');
        printChar(55, 0, s1[0].score-10+'0');
    }
    
    if(s2[0].score<10){
        printChar(55, 1, s2[0].score+'0');
    }
    else{
        printChar(55, 1, s2[0].score/10+'0');
        printChar(55, 1, s2[0].score-10+'0');
    }
}

void snake_main2(){
    gameover=0;
    exit=0;
    startGrid();
    createSnake(1);
    if(playertwo)
        createSnake(2);
    appleGen(s1);
    snake_input();
    while(!gameover){
        sleep(75);
        snake_input();
        moveSnake(s1);
        moveSnake(s2);
        draw();
        if(eaten(s1) || eaten(s2)){
            displayScore();
        }
        collision();
    }
    while(!exit)
        snake_input();
    return;
}

void reset(){
    clearScreen();
    snake_main2();
}
