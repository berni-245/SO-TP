
#include <snake.h>

//S_WIDTH = S_HEIGHT
int grid[S_WIDTH][S_HEIGHT];

static int gameover, read, exit;
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
        s1[0].y = S_HEIGHT/2-3;
        s1[0].len=1;
        s1[0].color=S1;
        s1[0].score=0;
        //playertwo=1;
        growSnake(s1[0].x, s1[0].y, s1);
    }
    else{
        s2[0].x = S_WIDTH/2+3;
        s2[0].y = S_HEIGHT/2+3;
        s2[0].len=1;
        s2[0].score=0;
        s2[0].color=S2;
        growSnake(s2[0].x, s2[0].y, s2);
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
    input = buf[0].key;                                
    switch(input){
        case 'w': s1[0].diry=-1; s1[0].dirx=0; break;
        case 'a': s1[0].diry=0; s1[0].dirx=-1; break;
        case 's': s1[0].diry=1; s1[0].dirx=0; break;
        case 'd': s1[0].diry=0; s1[0].dirx=1; break;
        case 'x': gameover=1;
        case 'p': s1[0].diry=0; s1[0].dirx=0; break;
        case 'r': reset(); break;
        
        //case 'o': createSnake(0x00FFFF); break;
        
        case 'i': s2[0].diry=-1; s2[0].dirx=0; break;
        case 'j': s2[0].diry=0; s2[0].dirx=-1; break;
        case 'k': s2[0].diry=1; s2[0].dirx=0; break;
        case 'l': s2[0].diry=0; s2[0].dirx=1; break;
        
        case ESC: gameover=1; exit=1; break;
        default: break;
    }
    /*if ((*s2)!=0){
        switch(input){
        case 'i': s2[0].diry=-1; s2[0].dirx=0; break;
        case 'j': s2[0].diry=0; s2[0].dirx=-1; break;
        case 'k': s2[0].diry=1; s2[0].dirx=0; break;
        case 'l': s2[0].diry=0; s2[0].dirx=1; break;
        default: break;
        }
    }*/
}

void draw(){
    int i;
    for(int j=0; j<S_HEIGHT; j++){
        for (i=0; i<S_WIDTH; i++){
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
        if(s1[0].x==s1[i-1].x && s1[0].y==s1[i-1].y){
            gameover=1;
        }
    }

    if(s2[0].x == S_WIDTH-1 || s2[0].x==0 || s2[0].y==S_HEIGHT-1 || s2[0].y==0){
        gameover=1;
    }
    for(int i=s2[0].len; i>1; i--){
        if(s2[0].x==s2[i-1].x && s2[0].y==s2[i-1].y){
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
    } while (apple[0]==0 || apple[1]==0 && notOnSnake(apple[0], apple[1]));
    grid[apple[0]][apple[1]]=APPLE;
}

//0 if on snake | 1 if not
int notOnSnake(int x, int y){
    if(x==s1[0].tail_x && y==s1[0].tail_y)
        return 0;

    for (int i=0; i<s1[0].len; i++){
        if (x==s1[i].x && y==s1[i].y)
            return 0;
    }
    return 1;
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
    printChar((S_WIDTH-2)*SQUARE, 0, s1[0].score+'0'-1);
    printChar((S_WIDTH-2)*SQUARE, 0, s1[0].score/10+'0'-1);
    printChar((S_WIDTH-2)*SQUARE+1, 0, s1[0].score-10+'0'-1);
    
    setFontColor(0xFFFFFF);
    if(s1[0].score<10){
        printChar((S_WIDTH-2)*SQUARE, 0, s1[0].score+'0');
    }
    else{
        printChar((S_WIDTH-2)*SQUARE, 0, s1[0].score/10+'0');
        printChar((S_WIDTH-2)*SQUARE+1, 0, s1[0].score-10+'0');
    }
    /*if(s2[0].score<10){
        printChar((S_WIDTH)*SQUARE, 1, s2[0].score+'0');
    }
    else{
        printChar((S_WIDTH)*SQUARE, 1, s2[0].score/10+'0');
        printChar((S_WIDTH)*SQUARE+1, 1, s2[0].score-10+'0');
    }*/
}

void snake_main2(){
    gameover=0;
    exit=0;
    startGrid();
    createSnake(1);
    createSnake(2);
    appleGen(s1);
    snake_input();
    while(!gameover){
        sleep(50);
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
    gameover=0;
    clearScreen();
    snake_main2();
}
