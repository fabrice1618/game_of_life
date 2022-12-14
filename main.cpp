#include <exception>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <ctime>

#define GRID_X 100
#define GRID_Y 50

#define CELL_ZONE 10
#define CELL_SIZE 8

#define CELL_COLOR_R 18
#define CELL_COLOR_G 14
#define CELL_COLOR_B 252

#define RANDOM_MAX 25
#define RANDOM_LIFE 6

enum game_state {
	GAME_LOOP,
	GAME_QUIT,
};

// Variables globales
char cell_grid[GRID_Y][GRID_X];
char next_grid[GRID_Y][GRID_X];

/*
Exception : InitError
*/
class InitError : public std::exception
{
    std::string msg;
public:
    InitError();
    InitError( const std::string & );
    virtual ~InitError() throw();
    virtual const char * what() const throw();
};

InitError::InitError() :
    exception(),
    msg( SDL_GetError() )
{
}

InitError::InitError( const std::string & m ) :
    exception(),
    msg( m )
{
}

InitError::~InitError() throw()
{
}

const char * InitError::what() const throw()
{
    return msg.c_str();
}

/*
Classe g?rant l'affichage
*/

class SDL
{
    SDL_Window * m_window;
    SDL_Renderer * m_renderer;
public:
    SDL( Uint32 flags = 0 );
    virtual ~SDL();
    void draw();
};


SDL::SDL( Uint32 flags )
{
    int screen_w = GRID_X * CELL_ZONE;
    int screen_h = GRID_Y * CELL_ZONE;

    if ( SDL_Init( flags ) != 0 )
        throw InitError();

    if ( SDL_CreateWindowAndRenderer( screen_w, screen_h, SDL_WINDOW_SHOWN,
                                      &m_window, &m_renderer ) != 0 )
        throw InitError();
}

SDL::~SDL()
{
    SDL_DestroyWindow( m_window );
    SDL_DestroyRenderer( m_renderer );
    SDL_Quit();
}

void SDL::draw()
{
    SDL_Rect cell;
    int cell_gap = (CELL_ZONE - CELL_SIZE) / 2;

    cell.w = CELL_SIZE;
    cell.h = CELL_SIZE;

    // Clear the window with a black background
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );

    // Show the window
    SDL_RenderPresent( m_renderer );

    // Couleur des cellules
    SDL_SetRenderDrawColor( m_renderer, CELL_COLOR_R, CELL_COLOR_G, CELL_COLOR_B, 255 );

    // Dessiner la grille
    for ( int y = 0; y < GRID_Y; y++ ) {
        for ( int x = 0; x < GRID_X; x++ ) {
            if (cell_grid[y][x] == 1) {
                cell.x = ( x * CELL_ZONE ) + cell_gap;
                cell.y = ( y * CELL_ZONE ) + cell_gap;

                SDL_RenderFillRect( m_renderer, &cell );
            }
        }
    }

    SDL_RenderPresent( m_renderer );
}


void grid_random()
{
    int alea;

    // On commence par initialiser le g?n?rateur de nombre pseudo-al?atoires.
    srand( time( NULL ) );

    for ( int y = 0; y < GRID_Y; y++ ) {
        for ( int x = 0; x < GRID_X; x++ ) {
            alea = rand() % RANDOM_MAX;
            if (alea < RANDOM_LIFE)
                cell_grid[y][x] = 1;
            else
                cell_grid[y][x] = 0;
        }
    }
}

int convert_x(int x)
{
    if (x<0) {
        x = x + GRID_X;
    } else {
        x = x % GRID_X;
    }

    return(x);
}

int convert_y(int y)
{
    if (y<0) {
        y = y + GRID_Y;
    } else {
        y = y % GRID_Y;
    }

    return(y);
}

int cell_count( int y, int x )
{
    int nb_vivante =    cell_grid[convert_y(y-1)][convert_x(x-1)] +
                        cell_grid[convert_y(y-1)][convert_x(x)] +
                        cell_grid[convert_y(y-1)][convert_x(x+1)] +
                        cell_grid[convert_y(y)][convert_x(x-1)] +
                        cell_grid[convert_y(y)][convert_x(x+1)] +
                        cell_grid[convert_y(y+1)][convert_x(x-1)] +
                        cell_grid[convert_y(y+1)][convert_x(x)] +
                        cell_grid[convert_y(y+1)][convert_x(x+1)] ;
    return(nb_vivante);
}

void next_state()
{
    int nb_autour;

    for (int y=0; y < GRID_Y; y++) {
        for (int x=0; x < GRID_X; x++) {
            // Compter le nombre de celulles vivantes autour
            nb_autour = cell_count(y, x);

            if (cell_grid[y][x] == 1) {
                // Si la celulle etait vivante
                if (nb_autour == 2 || nb_autour == 3) {
                    next_grid[y][x] = 1;
                } else {
                    next_grid[y][x] = 0;
                }
            } else {
                // Si la celulle etait morte
                if (nb_autour == 3) {
                    next_grid[y][x] = 1;
                } else {
                    next_grid[y][x] = 0;
                }
            }
        }
    }

}

int copy_grid()
{
    for (int y=0; y < GRID_Y; y++) {
        for (int x=0; x < GRID_X; x++) {
            cell_grid[y][x] = next_grid[y][x];
        }
    }

}

int main( int argc, char * argv[] )
{
    SDL_Event evenements;
    enum game_state state = GAME_LOOP;

    try
    {
        SDL sdl( SDL_INIT_VIDEO | SDL_INIT_TIMER );
        grid_random();

        while (state != GAME_QUIT) {
        	// Fermeture de la fenetre
            while ( (state != GAME_QUIT) && SDL_PollEvent(&evenements) ) {
                if ((evenements.type==SDL_WINDOWEVENT) && (evenements.window.event == SDL_WINDOWEVENT_CLOSE))
                    state = GAME_QUIT;
            }

            if (state != GAME_QUIT) {
                sdl.draw();
                next_state();
                copy_grid();
                SDL_Delay( 500 );
            }

        }

        return EXIT_SUCCESS;
    }
    catch ( const InitError & err )
    {
        std::cerr << "Error while initializing SDL:  "
                  << err.what()
                  << std::endl;
    }

    return EXIT_FAILURE;
}
