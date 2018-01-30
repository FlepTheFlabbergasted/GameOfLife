
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>

#define FPS 60
#define FRAME_TIME 1000 / FPS
uint32_t currentTime = 0, prevTime = 0;
time_t t;

//Screen dimension constants
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768 // 768

#define true 1
#define false 0

bool init();
bool loadMedia();
void closeSDL();

TTF_Font *points = NULL;
SDL_Texture *pointsTexture = NULL;
SDL_Surface *pointsSurface = NULL;

//SDL_Surface* loadSurface(char* filename);
SDL_Texture* loadTexture( char* filename);
//SDL_Texture* loadTexture2( char* filename);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;
//The window renderer
SDL_Renderer* gRenderer = NULL;
//Current displayed texture
SDL_Texture* gTexture = NULL;

//Main loop flag
bool quit = false;

typedef struct {
	int current;
	int next;
} cell;

typedef struct {
	cell **field;
	cell **workingField;
	int rows;
	int columns;
	int tileSize;
} game;

//Dimensions of the cells
void drawGrid(int rows, int columns, int width, int height);
//How many cells, how big
void drawCells(game *life, int rows, int columns, int width, int height);
//Kill all cells
void resetCells(game *life, int rows, int columns);
//current <- next and decide if they live of not
void evolveCells(game *life, int rows, int columns);
//Check how many neighbours a cell has
int checkNrOfNeighbours(game *life, int rows, int columns);
//Check if a cell is alive
int isAlive(game *life, int row, int column);
//Put the stuff i saved on the board
void resetToWorkingField( game *life );
//Get that shiet out of the way
void writeTtfText(int frames, int selectMode);
//Draw rectangle from mouse positions
SDL_Rect drawMouseRect(game *life, int lockMouseX, int lockMouseY, int mouseX, int mouseY);
//Select he cells you drew a rectangle around
void selectCells( game *life, game *savedCells, SDL_Rect staticMouseRect );
// Does what is says it does duh
void printSavedCells( game *life, game *savedCells, int startColumn, int startRow );

int WinMain( int argc, char* args[] ){

	game life;
	game savedCells;
	life.tileSize = 8;
	life.rows = SCREEN_WIDTH / life.tileSize;
	life.columns = SCREEN_HEIGHT / life.tileSize;
	life.field = malloc( life.rows * sizeof(cell*) );
	life.workingField = malloc( life.rows * sizeof(cell*) );

	for(int i = 0; i < life.rows; i++){
		life.field[i] = malloc( life.columns * sizeof(cell) );
		life.workingField[i] = malloc( life.columns * sizeof(cell) );
	}

	if( life.field == NULL || life.workingField == NULL){
		printf("life.field - ERROR ALLOCATING MEMORY :c\n");
		return 0;
	}

	resetCells(&life, life.rows, life.columns);
	int lockCell = false;
	int lockCellLast = false;
	int reset = false, clearBoard = false;
	int selectMode = false;

	int mouseX, mouseY, lockMouseX, lockMouseY;
	int iClicked = false, iCanClick = true;
	SDL_Rect staticMouseRect;
	int frameCounter = 0, frames = 30;

	int pauseGame = true;
	srand( (unsigned) time(&t) );

	//Event handler
	SDL_Event e;

	//Start up SDL and create window
	if( !init() ){
		printf( "Failed to initialize!\n" );
	}
	else{
		//Load media
		if( !loadMedia() ){
			printf( "Failed to load media!\n" );
		}
		else{
			//While application is running

			while( !quit ){
				currentTime = SDL_GetTicks();
				if(currentTime - prevTime >= FRAME_TIME){
					frameCounter++;
					//Handle events on queue
					while( SDL_PollEvent( &e ) != 0 ){
						//User requests quit
						if( e.type == SDL_QUIT )
						{
							quit = true;
						}
						else if(e.type == SDL_KEYDOWN){
							switch(e.key.keysym.sym){
							case SDLK_SPACE:
								if(pauseGame == true ){
									pauseGame = false;
								}
								else{
									pauseGame = true;
								}
								break;
							case SDLK_s:
								selectMode = !selectMode;
								break;
							case SDLK_r:
								if( !selectMode ){
									reset = true;
								}
								break;
							case SDLK_BACKSPACE:
								if( !selectMode ){
									clearBoard = true;
								}
								break;
							case SDLK_KP_MINUS:
								frames--;
								break;
							case SDLK_KP_PLUS:
								frames++;
								break;
							case SDLK_v:
								SDL_GetMouseState(&mouseX, &mouseY);
								printSavedCells( &life, &savedCells, mouseX/life.tileSize, mouseY/life.tileSize );
								break;
							}
						}
						else if( e.type == SDL_MOUSEMOTION ){
							SDL_GetMouseState(&mouseX, &mouseY);
						}
						else if( e.type == SDL_MOUSEBUTTONDOWN){
							if( e.button.button == SDL_BUTTON_LEFT){
								if( pauseGame == true ){
									if( lockCell == false ){
										lockCell = true;
										lockCellLast = !life.field[mouseX/life.tileSize][mouseY/life.tileSize].current;
									}

									if( selectMode && iCanClick ){
										SDL_GetMouseState(&lockMouseX, &lockMouseY);
										iClicked = true;
										iCanClick = false;
									}
								}
							}
						}
						else if( e.type == SDL_MOUSEBUTTONUP){
							if( e.button.button == SDL_BUTTON_LEFT){
								lockCell = false;
								lockCellLast = false;

								if( selectMode ){
									iClicked = false;
									iCanClick = true;
									staticMouseRect = drawMouseRect(&life, lockMouseX, lockMouseY, mouseX, mouseY );

									selectCells( &life, &savedCells, staticMouseRect ); // DOOOOO EEEEEEEEET


								}
							}
						}
					}

					//Set screen clear color to black
					SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
					//Clear screen
					SDL_RenderClear( gRenderer );

					if( lockCell && !selectMode){
						life.field[mouseX/life.tileSize][mouseY/life.tileSize].current = lockCellLast;
						life.field[mouseX/life.tileSize][mouseY/life.tileSize].next = lockCellLast;

						//Save the stuff i clicked in
						life.workingField[mouseX/life.tileSize][mouseY/life.tileSize].current = lockCellLast;
						life.workingField[mouseX/life.tileSize][mouseY/life.tileSize].next = lockCellLast;
					}

					drawCells(&life, life.rows, life.columns, life.tileSize, life.tileSize);
					//drawGrid(life.rows, life.columns, life.tileSize, life.tileSize);

					if( pauseGame == true ){
						writeTtfText(frames, selectMode);
					}

					if( selectMode ){
						if( iClicked ){
							drawMouseRect(&life, lockMouseX, lockMouseY, mouseX, mouseY );
						}

						if( !iClicked && iCanClick){
							SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );
							SDL_RenderDrawRect( gRenderer, &staticMouseRect );
						}
					}

					//Update screen
					SDL_RenderPresent( gRenderer );

					if( frameCounter > frames && pauseGame == false){
						evolveCells(&life, life.rows, life.columns);
						frameCounter = 0;
					}

					if( !selectMode ){
						if( reset ){
							pauseGame = true;
							resetToWorkingField(&life);
							reset = false;
						}

						if( clearBoard ){
							resetCells(&life, life.rows, life.columns);
							clearBoard = false;
						}
					}

					prevTime = SDL_GetTicks();
				}
				else{
					SDL_Delay(FRAME_TIME - (currentTime-prevTime));
				}
			}
		}
	}

	closeSDL();

	return 0;
}

int isWithingBoard( int x, int y, int width, int height ){



	return 0;
}

void printSavedCells( game *life, game *savedCells, int tiledMouseY, int tiledMouseX ){

	//int i, j = -1;

	for( int x = 0; x < savedCells->columns; x++ ){
		for( int y = 0; y < savedCells->rows; y++ ){

			if( isWithingBoard(tiledMouseY + x, tiledMouseX + y, life->columns, life->rows) ){

				if( savedCells->field[y][x].current == true ){
					life->field[tiledMouseY + x][tiledMouseX + y].current = savedCells->field[y][x].current;
				}

			}
		}
	}
}

void selectCells( game *life, game *savedCells, SDL_Rect staticMouseRect ){

	/*	Ta in pos för rektanglen staticMouseRec.
	 *	Där finns rows, columns -> rows2, columns2.
	 *	Börja på rows och kör till rows2, samma med columns.
	 *	Lägg in rows,columns i en array.
	 *	När du skriver ut det du sparat, börja skriva ut på pos(row,column) där
	 *	musen är
	 */

	savedCells->columns = staticMouseRect.w/life->tileSize;
	savedCells->rows = staticMouseRect.h/life->tileSize;

	// Allocate savedCells->rows(nr of) rows, each row is a pointer to int
	savedCells->field = malloc( savedCells->rows * sizeof(cell*) );

	// For each row, allocate savedCells->columns(nr of) columns
	for ( int i = 0; i < savedCells->rows; i++ ){
		savedCells->field[i] = malloc( savedCells->columns * sizeof(cell) );
	}

	int i = 0, j = -1;
	for(int y = staticMouseRect.y/life->tileSize; y < (staticMouseRect.y/life->tileSize) + savedCells->rows; y++){
		j++;
		i = 0;
		for(int x = staticMouseRect.x/life->tileSize; x < (staticMouseRect.x/life->tileSize) + savedCells->columns; x++ ){
			savedCells->field[j][i].current = life->field[x][y].current;
			i++;
		}
	}

	for(int y = 0; y <savedCells->rows; y++){
		for(int x = 0; x < savedCells->columns; x++ ){
			printf("%d ", savedCells->field[y][x].current);
		}
		printf("\n");
	}

}

SDL_Rect drawMouseRect(game *life, int lockMouseX, int lockMouseY, int mouseX, int mouseY){

	//Set draw color to red
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );

	SDL_Rect mouseRect = { lockMouseX - (lockMouseX % life->tileSize),
						   lockMouseY - (lockMouseY % life->tileSize),
						   mouseX-lockMouseX - ((mouseX-lockMouseX) % life->tileSize),
						   mouseY-lockMouseY - ((mouseY-lockMouseY) % life->tileSize)};

	SDL_RenderDrawRect( gRenderer, &mouseRect );

	return mouseRect;
}

void writeTtfText(int frames, int selectMode){

	//Render ttf text
	char buffer[64] = { 0 };
	sprintf(buffer, "Paused");
	SDL_Color foregroundColor = { 255, 255, 255 };
	pointsSurface = TTF_RenderText_Solid(points, buffer, foregroundColor);
	pointsTexture = SDL_CreateTextureFromSurface(gRenderer, pointsSurface);
	SDL_Rect fpsTextDst = { 10, 10, 100, 40 };
	SDL_RenderCopy(gRenderer, pointsTexture, NULL, &fpsTextDst);

	SDL_DestroyTexture(pointsTexture);
	SDL_FreeSurface(pointsSurface);

	sprintf(buffer, "%d", frames);
	pointsSurface = TTF_RenderText_Solid(points, buffer, foregroundColor);
	pointsTexture = SDL_CreateTextureFromSurface(gRenderer, pointsSurface);
	SDL_Rect framesTextDst = { 10, 50, 30, 30 };
	SDL_RenderCopy(gRenderer, pointsTexture, NULL, &framesTextDst);

	SDL_DestroyTexture(pointsTexture);
	SDL_FreeSurface(pointsSurface );

	if( selectMode ){
		sprintf(buffer, "Select");
		pointsSurface = TTF_RenderText_Solid(points, buffer, foregroundColor);
		pointsTexture = SDL_CreateTextureFromSurface(gRenderer, pointsSurface);
		SDL_Rect framesTextDst = { 10, 75, 70, 30 };
		SDL_RenderCopy(gRenderer, pointsTexture, NULL, &framesTextDst);

		SDL_DestroyTexture(pointsTexture);
		SDL_FreeSurface(pointsSurface );
	}
	else if( !selectMode ){
		sprintf(buffer, "Draw");
		pointsSurface = TTF_RenderText_Solid(points, buffer, foregroundColor);
		pointsTexture = SDL_CreateTextureFromSurface(gRenderer, pointsSurface);
		SDL_Rect framesTextDst = { 10, 75, 70, 30 };
		SDL_RenderCopy(gRenderer, pointsTexture, NULL, &framesTextDst);

		SDL_DestroyTexture(pointsTexture);
		SDL_FreeSurface(pointsSurface );
	}

	return;
}


void resetToWorkingField( game *life ){

	for(int x = 0; x < life->rows; x++ ){
		for(int y = 0; y < life->columns; y++){
			life->field[x][y].current = life->workingField[x][y].current;
			life->field[x][y].next = life->workingField[x][y].next;
		}
	}

	return;
}

int isAlive(game *life, int row, int column){

	if( life->field[row][column].current == true ){
		return 1;
	}
	else{
		return 0;
	}

	return 0;
}

int checkNrOfNeighbours(game *life, int rows, int columns){

	int newColumn, newRow;
	int aliveCount = 0;

	for(int x = -1; x <= 1; x++){
		for(int y = -1; y <= 1; y++){
			newRow = x + rows;
			newColumn = y + columns;

			if(newColumn >= 0 && newColumn < life->columns
				&& !(newColumn == columns && newRow == rows)
				&& newRow >= 0 && newRow < life->rows
				){

				if( isAlive(life, newRow, newColumn) ){
					aliveCount++;
				}
			}
		}
	}

	return aliveCount;
}

void evolveCells(game *life, int rows, int columns){

	int neighbours;

	// Decide if they live or not
	for(int a = 0; a < rows; a++){
		for(int b = 0; b < columns; b++){

			neighbours = checkNrOfNeighbours(life, a, b);
			//printf("(%d, %d) %d\n", a, b, neighbours);

			if( life->field[a][b].current == true){
				switch( neighbours ){
				case 0:
				case 1:
					life->field[a][b].next = false;
					break;
				case 2:
				case 3:
					life->field[a][b].next = true;
					break;
				case 4:
				default:
					life->field[a][b].next = false;
					break;
				}
			}
			else{
				if( neighbours == 3 ){
					life->field[a][b].next = true;
				}
			}
		}
	}

	for(int a = 0; a < rows; a++){
		for(int b = 0; b < columns; b++){
			life->field[a][b].current = life->field[a][b].next;
		}
	}

	return;
}


void drawGrid(int rows, int columns, int width, int height){

	//Set line color to white
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

	for(int i = 0; i < rows; i++){
		SDL_RenderDrawLine(gRenderer, i * width, 0, i * width, SCREEN_HEIGHT);
	}

	for(int i = 0; i < rows; i++){
		SDL_RenderDrawLine(gRenderer, 0, i * height, SCREEN_WIDTH, i * height);
	}

	return;
}

void drawCells(game *life, int rows, int columns, int width, int height){

	for(int a = 0; a < rows; a++){
		for(int b = 0; b < columns; b++){
			SDL_Rect cellWall;
			cellWall.x =  a*width,
			cellWall.y =  b*height,
			cellWall.w = width,
			cellWall.h = height;

			if(life->field[a][b].current == false){
				SDL_SetRenderDrawColor( gRenderer, 0x66, 0x66, 0x66, 0xFF );
				SDL_RenderDrawRect(gRenderer, &cellWall);
			}
			else{
				SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0xFF );
				SDL_RenderFillRect(gRenderer, &cellWall);

				SDL_SetRenderDrawColor( gRenderer, 0x66, 0x66, 0x66, 0xFF );
				SDL_RenderDrawRect(gRenderer, &cellWall);
			}

		}
	}

	return;
}

void resetCells(game *life, int rows, int columns){

	for(int a = 0; a < rows; a++){
		for(int b = 0; b < columns; b++){
			life->field[a][b].current = false;
			life->field[a][b].next = false;

			life->workingField[a][b].current = false;
			life->workingField[a][b].next = false;
		}
	}

	return;
}


bool init(){

	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		success = false;
	}
	else{

		//Create window
		gWindow = SDL_CreateWindow( "Game of life - Press space to start/pause and +/- to speed up/down. Clear with backspace and reset with r", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL ){
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
			success = false;
		}
		else{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
			   int imgFlags = IMG_INIT_PNG;
			   if( !( IMG_Init( imgFlags ) & imgFlags ) )
			   {
				   printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
				   success = false;
			   }

			  if(TTF_Init() < 0) {
					  printf("TTF Init failed: %s\n", TTF_GetError());
					  success = false;
				  } else {
					  points = TTF_OpenFont("arialbd.ttf", 15);
				  if(points == NULL) {
					  printf("Error loading font. %s\n", TTF_GetError());
					  success = false;
				  }
			  }

			   //Get window surface
			   gScreenSurface = SDL_GetWindowSurface( gWindow );

			}
		}
	}

	return success;
}


bool loadMedia()
{
    //Loading success flag
    bool success = true;

    //Load splash image
   /* gTexture = loadTexture( "space.jpg" );
    if( gTexture == NULL )
    {
        printf( "Unable to load image %s! SDL Error: %s\n", "interesting", SDL_GetError() );
        success = false;
    }*/

    return success;
}

/*SDL_Surface* loadSurface(char* filename) {

	//The final optimized image
	SDL_Surface* optimizedSurface = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( filename);

	if( loadedSurface == NULL ) {
		printf( "Unable to load image %s! SDL_image Error: %s\n", filename, IMG_GetError() );
	} else {
		//Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);

		if( optimizedSurface == NULL ) {
			printf( "Unable to optimize image %s! SDL Error: %s\n", filename, SDL_GetError() );
		}
		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	return optimizedSurface;
}*/

SDL_Texture* loadTexture( char* filename){

    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( filename );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", filename, IMG_GetError() );
    }
    else
    {

      //  SDL_SetColorKey(loadedSurface, true, SDL_MapRGB(loadedSurface->format,255,255,255));

        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", filename, SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return newTexture;
}

/*
SDL_Texture* loadTexture2( char* filename){

    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( filename );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", filename, IMG_GetError() );
    }
    else
    {

        SDL_SetColorKey(loadedSurface, true, SDL_MapRGB(loadedSurface->format,0,0,0));

        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", filename, SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return newTexture;
}
*/

void closeSDL(){

	//Free loaded image
	SDL_DestroyTexture( gTexture );
	SDL_DestroyTexture( pointsTexture );
	gTexture = NULL;
	pointsTexture = NULL;

	//Free global font
	TTF_CloseFont( points );
	points = NULL;

	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	quit = true;
}

