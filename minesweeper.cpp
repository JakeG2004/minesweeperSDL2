#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace std;

class cell{
    public:
        char type = 'e'; //m = mine, e = empty
        char state = 'b'; //f = flagged, c = clicked, b = blank, q = question mark
        int proximity = 0; //Nearby mines
        SDL_Texture* cellTex;
        SDL_Rect box;
};

class border{
    public:
        SDL_Texture* borderTex;
        SDL_Rect box;
};

//Non-SDL Vars
const int SIZEX = 16;
const int SIZEY = 16;
const int MINES = 40;

bool fail = false;

int clicked = 0;

cell board[SIZEX][SIZEY];

//SDL Vars
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

//Number textures
SDL_Texture* tex0 = NULL;
SDL_Texture* tex1 = NULL;
SDL_Texture* tex2 = NULL;
SDL_Texture* tex3 = NULL;
SDL_Texture* tex4 = NULL;
SDL_Texture* tex5 = NULL;
SDL_Texture* tex6 = NULL;
SDL_Texture* tex7 = NULL;
SDL_Texture* tex8 = NULL;

//Block textures
SDL_Texture* emptyBoxTex = NULL;
SDL_Texture* flagTex = NULL;
SDL_Texture* redMineTex = NULL;
SDL_Texture* qMarkTex = NULL;
SDL_Texture* blankMineTex = NULL;

//bg tex
SDL_Texture* bgTex = NULL;

//background
border bg;
border info;

//Smiley
SDL_Texture* smileyDefaultTex = NULL;
SDL_Texture* smileyOTex = NULL;
SDL_Texture* smileyCurrentTex = NULL;
SDL_Texture* smileyWinTex = NULL;
SDL_Texture* smileyLoseTex = NULL;
border smiley;

SDL_Point mousePosition;

//SDL functions
void resizeBoard();
void init();
void close();
void loadMedia();
void boxClicked();
void placeFlag();
void initBoard();


//Non SDL Functions
void placeMines();
void restartGame();

int main(){

    srand(time(0));

    init();
    loadMedia();
    initBoard();

    placeMines();

    resizeBoard();

    while(true){
        SDL_Event e;
        //Event handler
        if (SDL_WaitEvent(&e)) {
            mousePosition.x = e.motion.x; 
            mousePosition.y = e.motion.y;

            //Exit on quit
            if(e.type == SDL_QUIT){
                break;
            }

            //Get mouse button
            if(e.type == SDL_MOUSEBUTTONDOWN){
                smiley.borderTex = smileyOTex;
                switch(e.button.button){
                    //Left click
                    case SDL_BUTTON_LEFT:
                        boxClicked();
                        break;
                    //Right click
                    case SDL_BUTTON_RIGHT:
                        placeFlag();
                        break;
                }
            }

            //Get mouse up
            if(e.type == SDL_MOUSEBUTTONUP)
                smiley.borderTex = smileyCurrentTex;

            //Detect window event
            if(e.type == SDL_WINDOWEVENT){
                switch(e.window.event){
                    //Window resized or size changed
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        resizeBoard();
                        break;
                }
            }
        }

        //Draw bg
        SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
        SDL_RenderClear(renderer);

        //Draw bg
        SDL_RenderCopy(renderer, bg.borderTex, NULL, &bg.box);
        SDL_RenderCopy(renderer, info.borderTex, NULL, &info.box);

        //Render smiley
        SDL_RenderCopy(renderer, smiley.borderTex, NULL, &smiley.box);

        //Draw array of boxes
        for(int y = 0; y < SIZEY; y++){
            for(int x = 0; x < SIZEX; x++){
                SDL_RenderCopy(renderer, board[x][y].cellTex, NULL, &(board[x][y].box));
            }
        }

        SDL_RenderPresent(renderer);
    }

    //Safely exit
    close();
    return 0;
}

void resizeBoard(){
    int CELLSIZE;
    int BUFFERX;
    int BUFFERY;
    int HEADER;

    int width;
    int height;

    SDL_GetWindowSize(window, &width, &height);

    HEADER = height / 10;
    
    //Set cell size based on window size
    CELLSIZE = (height - HEADER) / (SIZEY + 1);
    if(width < height){
        CELLSIZE = width / (SIZEX + 1);
    }

    //Set buffer based on cell size
    BUFFERX = (width / 2) - (CELLSIZE * SIZEX / 2);

    //BUFFERY = (height - BUFFERX - (SIZEY * CELLSIZE));
    BUFFERY = ((height - HEADER) / 2) - (CELLSIZE * SIZEY / 2);

    //For every cell
    for(int y = 0; y < SIZEY; y++){
        for(int x = 0; x < SIZEX; x++){
            //Set scale and pos of box
            //board[x][y].box = {BUFFERX + (x * CELLSIZE), BUFFERY + (y * CELLSIZE) + HEADER, CELLSIZE, CELLSIZE};

            board[x][y].box = {BUFFERX + (x * CELLSIZE), height - ((y+1) * CELLSIZE) - (height - BUFFERY) / 20, CELLSIZE, CELLSIZE};
        }
    }

    //Set background
    bg.borderTex = bgTex;
    bg.box = {0, HEADER + BUFFERY, width, height - BUFFERY - HEADER};

    //Set info box
    info.borderTex = bgTex;
    info.box = {0, 0, width, HEADER + BUFFERY};

    //Smiley
    smiley.box = {(width - CELLSIZE) / 2, (HEADER + BUFFERY - CELLSIZE) / 2, CELLSIZE, CELLSIZE};
}

void init(){
    //Init libs
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    IMG_Init(IMG_INIT_PNG);

    //Define Vars
    window = SDL_CreateWindow( "Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 1000, SDL_WINDOW_RESIZABLE );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //Set min window size
    SDL_SetWindowMinimumSize(window, 300, 300);
}

void initBoard(){
    //Set every box texture to empty box
    for(int y=0 ;y<SIZEY; y++){
        for(int x=0; x<SIZEX; x++){
            board[x][y].cellTex = emptyBoxTex;
            board[x][y].type = 'e';
            board[x][y].proximity = 0;
            board[x][y].state = 'b';
        }
    }

    clicked = 0;
    fail = false;

    smileyCurrentTex = smileyDefaultTex;
    smiley.borderTex = smileyDefaultTex;
}

void loadMedia(){

    //Load textures
    //credit to Black Squirrel

    //Blocks
    emptyBoxTex = IMG_LoadTexture(renderer, "images/emptybox.png");
    flagTex = IMG_LoadTexture(renderer, "images/flag.png");
    redMineTex = IMG_LoadTexture(renderer, "images/red_mine.png");
    qMarkTex = IMG_LoadTexture(renderer, "images/qmark.png");
    blankMineTex = IMG_LoadTexture(renderer, "images/blankMine.png");

    //Numbers
    tex0 = IMG_LoadTexture(renderer, "images/0.png");
    tex1 = IMG_LoadTexture(renderer, "images/1.png");
    tex2 = IMG_LoadTexture(renderer, "images/2.png");
    tex3 = IMG_LoadTexture(renderer, "images/3.png");
    tex4 = IMG_LoadTexture(renderer, "images/4.png");
    tex5 = IMG_LoadTexture(renderer, "images/5.png");
    tex6 = IMG_LoadTexture(renderer, "images/6.png");
    tex7 = IMG_LoadTexture(renderer, "images/7.png");
    tex8 = IMG_LoadTexture(renderer, "images/8.png");

    //Smiley
    smileyDefaultTex = IMG_LoadTexture(renderer, "images/smileyDefault.png");
    smileyOTex = IMG_LoadTexture(renderer, "images/smileyO.png");
    smileyWinTex = IMG_LoadTexture(renderer, "images/smileyWin.png");
    smileyLoseTex = IMG_LoadTexture(renderer, "images/smileyLose.png");

    //bg
    bgTex = IMG_LoadTexture(renderer, "images/bg.png");
}

void placeMines(){
    int randx;
    int randy;
    for(int i=0; i<MINES; i++){

        //Find position that isn't already a mine
        do{

            randx = rand()%SIZEX;
            randy = rand()%SIZEY;

        }while(board[randx][randy].type == 'm');

        //Set mine
        board[randx][randy].type = 'm';

        //Update proximity of surrounding cells
        for(int y=randy-1; y<=randy+1; y++){
            for(int x=randx-1; x<=randx+1; x++){
                //Ensure in bounds
                if(x >= 0 && x < SIZEX && y >= 0 && y < SIZEY){
                    if(board[x][y].type != 'm')
                        board[x][y].proximity++;
                }

            }
        }
    }
}

void placeFlag(){
    for(int y=0; y<SIZEY; y++){
        for(int x = 0; x<SIZEX; x++){
            if (SDL_PointInRect(&mousePosition, &board[x][y].box) && fail == false){
                //If already clicked, return
                if(board[x][y].state == 'c'){
                    return;
                }

                //If box is flag
                if(board[x][y].state == 'f'){
                    board[x][y].cellTex = qMarkTex;
                    board[x][y].state = 'q';
                    return;
                //If box is question
                } else if(board[x][y].state == 'q'){
                    board[x][y].cellTex = emptyBoxTex;
                    board[x][y].state = 'b';
                    return;
                //If box is empty
                } else if(board[x][y].state == 'b'){
                    board[x][y].cellTex = flagTex;
                    board[x][y].state = 'f';
                    return;
                }
            }
        }
    }
}

void revealMines(){
    for(int y=0; y<SIZEY; y++){
        for(int x = 0; x<SIZEX; x++){
            if(board[x][y].type == 'm' && board[x][y].cellTex != redMineTex){
                board[x][y].cellTex = blankMineTex;
            }
        }
    }
}

void boxClicked(){

    //Smiley click
    if(SDL_PointInRect(&mousePosition, &smiley.box)){
        restartGame();
    }

    for(int y=0; y<SIZEY; y++){
        for(int x = 0; x<SIZEX; x++){
            if (SDL_PointInRect(&mousePosition, &board[x][y].box) && fail == false){
                //if flagged, dont let the player click
                if(board[x][y].state == 'f'){
                    return;
                }
                //if mine
                if(board[x][y].type == 'm'){
                    board[x][y].cellTex = redMineTex;
                    fail = true;
                    smileyCurrentTex = smileyLoseTex;
                    revealMines();
                    return;
                } else {
                    board[x][y].state = 'c';
                    clicked++;
                    //Fill cell textures according to proximity
                    switch(board[x][y].proximity){
                        case 0:
                            board[x][y].cellTex = tex0; 
                            break;
                        case 1:
                            board[x][y].cellTex = tex1; 
                            break;
                        case 2:
                            board[x][y].cellTex = tex2; 
                            break;
                        case 3:
                            board[x][y].cellTex = tex3; 
                            break;
                        case 4:
                            board[x][y].cellTex = tex4; 
                            break;
                        case 5:
                            board[x][y].cellTex = tex5; 
                            break;
                        case 6:
                            board[x][y].cellTex = tex6; 
                            break;
                        case 7:
                            board[x][y].cellTex = tex7; 
                            break;
                        case 8:
                            board[x][y].cellTex = tex8; 
                            break;
                    }
                    if(clicked == (SIZEX * SIZEY) - MINES){
                        smileyCurrentTex = smileyWinTex;
                        fail = true;
                    }
                }
            }
        }
    }
}

void close(){
    //Safely destroy and quit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void restartGame(){
    initBoard();
    placeMines();
}




