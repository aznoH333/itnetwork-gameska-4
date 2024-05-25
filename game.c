/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   Example originally created with raylib 1.0, last time updated with raylib 1.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/
#include "gframework.c"
#include "raylib.h"
#include <stdio.h>


//------------------------------------------------------------------------------------
// Hud
//------------------------------------------------------------------------------------
bool hasGameStarted = false;
bool gameOver = false;
int score = 0;
char scoreDisplay[10];
int gameTime = 0;
void drawHud(){

    sprintf(scoreDisplay, "%i", score);


    if (!hasGameStarted){
        drawFancyText("Poletujici ptak", 100, 100, 60, YELLOW);
        drawFancyText("Zmackni mezernik", 220, 190, 20, WHITE);
    }

    if (gameOver){
        drawFancyText("Sukujes!", 180, 100, 60, YELLOW);
        drawFancyText("Tvoje skore je", 220, 190, 20, WHITE);
        drawFancyText(scoreDisplay, 380, 190, 20, WHITE);


        drawFancyText("Zmackni mezernik", 220, 260, 20, WHITE);


    }else {
        drawFancyText(scoreDisplay, 320, 20, 30, WHITE);

    }
}

void addScore(){
    score++;
}

//------------------------------------------------------------------------------------
// Pipes
//------------------------------------------------------------------------------------
struct Pipes{
    float x;
    int spaceBetweenPipes;
    int basePipeHeight;
    bool exists;
    bool scored;
};
typedef struct Pipes Pipes;

#define MAX_PIPE_COUNT 5
Pipes pipes[MAX_PIPE_COUNT];

int nextPipeIndex = 0;
void initPipes(float difficulity){
    Pipes p;

    p.x = 640.0f;
    p.spaceBetweenPipes = GetRandomValue(4, 5);
    p.basePipeHeight = GetRandomValue(0, 4);
    p.spaceBetweenPipes *= 32;
    p.basePipeHeight *= 32;
    p.exists = true;
    p.scored = false;

    pipes[nextPipeIndex] = p;
    nextPipeIndex++;
    nextPipeIndex %= MAX_PIPE_COUNT;
}

void drawPipe(Pipes* this){
    if (this->exists == false){
        return;
    }


    // draw top pipe
    for (int i = 0; i < this->basePipeHeight; i += 32){
        draw(5, this->x, i);
        draw(6, this->x + 32, i);
    }

    // draw bottom pipe
    for (int i = this->basePipeHeight + this->spaceBetweenPipes + 32; i < 360; i += 32){
        draw(5, this->x, i);
        draw(6, this->x + 32, i);
    }

    // draw top
    draw(7, this->x, this->basePipeHeight);
    draw(8, this->x + 32, this->basePipeHeight);

    // draw bottom
    draw(3, this->x, this->basePipeHeight + this->spaceBetweenPipes);
    draw(4, this->x + 32, this->basePipeHeight + this->spaceBetweenPipes);



}

//------------------------------------------------------------------------------------
// World
//------------------------------------------------------------------------------------
float worldDistance = 0.0f;
float worldOffset = 0.0f;
bool isWorldMoving = true;
bool generatePipes = false;
int nextPipeCounter = 0;
#define WORLD_SPEED 1.0f
#define WORLD_WIDTH 21
#define GROUND_START 328
void updateWorld(){

    if (isWorldMoving){

        worldDistance += WORLD_SPEED;
        worldOffset += WORLD_SPEED;
        if (worldOffset > 32){
            worldOffset -= 32;

            if (generatePipes){

                nextPipeCounter--;

                if (nextPipeCounter <= 0){
                    initPipes(10);
                    nextPipeCounter = GetRandomValue(5, 7);
                }
            }
        }
    }

    // pipes
    for (int i = 0; i < MAX_PIPE_COUNT; i++){
        Pipes* p = &pipes[i];

        p->x -= WORLD_SPEED * (isWorldMoving);

        if(p->scored == false && p->x <= 64 && p->exists){

            addScore();
            p->scored = true;

        }

        drawPipe(p);


    }

    // draw
    for (int i = 0; i < WORLD_WIDTH; i++){
        draw(2, i * 32 - worldOffset, GROUND_START);
    }
}

void stopWorld(){
    isWorldMoving = false;
    generatePipes = false;

}

bool collidesWithTerrain(float x, float y, float w, float h){
    if (y + h > GROUND_START || y < 0){
        return true;
    }else{
        // check pipes
        for (int i = 0; i < MAX_PIPE_COUNT; i++){
            Pipes* p = &pipes[i];

            if (p->exists
                && (checkBoxCollisions(x, y, w, h, p->x, 0, 64, p->basePipeHeight + 32)
                || checkBoxCollisions(x, y, w, h, p->x, p->basePipeHeight + p->spaceBetweenPipes, 64, 900) )){
                return true;
            }
        }

    }
    return false;
}

//------------------------------------------------------------------------------------
// Reset
//------------------------------------------------------------------------------------
void reset(){
    worldDistance = 0;
    gameOver = false;
    isWorldMoving = true;
    generatePipes = true;
    nextPipeCounter = 0;
    score = 0;

    // reset pipes
    for (int i = 0; i <MAX_PIPE_COUNT; i++){
        pipes[i].exists = false;

    }

}


//------------------------------------------------------------------------------------
// Player
//------------------------------------------------------------------------------------
void startGame(){
    hasGameStarted = true;
    isWorldMoving = true;
    generatePipes = true;

}


struct Player {
    float x;
    float y;
    float yV;
    int animationTimer;
    bool isDead;
    float rotation;
};
typedef struct Player Player;

Player initPlayer(){
    Player p;
    p.x = 64;
    p.y = 128;
    p.yV = 0.0f;
    p.rotation = 0.0f;
    p.animationTimer = 0;
    p.isDead = false;

    return p;
}

#define PLAYER_TERMINAL_VELOCITY 5.5f
#define MAX_ROTATION 70.0f
#define MIN_ROTATION -40.0f

void playerJump(Player* this){
    this->yV = -3.5;
    this->animationTimer = 20;
    this->rotation = MIN_ROTATION;

}

void updatePlayer(Player* this){
    if (hasGameStarted == false){
        draw(gameTime % 60 < 30, this->x, this->y);
        this->y += sin(gameTime / 30.0f);
        if (IsKeyPressed(KEY_SPACE) && this->animationTimer == 0){
            playerJump(this);
            startGame();
        }
    }
    else if (this->isDead == false){// alive

        if (collidesWithTerrain(this->x, this->y, 32, 32)){
            this->isDead = true;
            stopWorld();
            this->yV = 0.0f;
            this->rotation = 0.0f;
        }



        this->y += this->yV;
        if (this->yV < PLAYER_TERMINAL_VELOCITY){
            this->yV += 0.2f;
        }


        if (IsKeyPressed(KEY_SPACE) && this->animationTimer == 0){
            playerJump(this);
        }

        this->rotation += (this->rotation < MAX_ROTATION) * 1.5f;


        this->animationTimer -= this->animationTimer > 0;

        drawR(this->animationTimer == 0, this->x, this->y, this->rotation);

    }else {
        if (this->y + 16 < GROUND_START){
            this->y += this->yV;
            if (this->yV < PLAYER_TERMINAL_VELOCITY){
                this->yV += 0.2f;
            }
            this->rotation += (this->rotation < MAX_ROTATION) * 1.5f;
        }else {
            gameOver = true;

            if (IsKeyPressed(KEY_SPACE)){
                *this = initPlayer();
                reset();
            }
        }
        drawR(9, this->x, this->y, this->rotation);

    }


}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    initFramework();
    Player p = initPlayer();

    // Main game loop
    while (!WindowShouldClose())
    {
        
        fDrawBegin();

            ClearBackground(BLUE);

            updateWorld();
            updatePlayer(&p);
            drawHud();

        fDrawEnd();
        gameTime++;
        
    }

	disposeFramework();
    

    return 0;
}
