// Technological progress game

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <ctime>
#include <iostream>

#include "state.h"
#include "technology.h"
#include "tree.h"

using namespace std::chrono_literals;

void simulation(State state, SDL_Window *window) {
  // Clock stuff.
  constexpr std::chrono::nanoseconds tick(16ms);

  using clock = std::chrono::high_resolution_clock;

  std::chrono::nanoseconds lag(0ns);

  // Font stuff.
  TTF_Font *font = TTF_OpenFont("../fonts/OpenSans-Regular.ttf",
                                24); // this opens a font style and sets a size

  if (font == NULL) {
    printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
  }

  // Main simulation loop.
  auto start = clock::now();

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

  while (true) {
    auto dt = clock::now() - start;
    start = clock::now();
    lag += std::chrono::duration_cast<std::chrono::nanoseconds>(dt);

    while (lag >= tick) {
      lag -= tick;

      state.update();
    }

    // SDL_Surface *screenSurface = SDL_GetWindowSurface(window);

    // SDL_FillRect(screenSurface, NULL,
    //              SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

    SDL_Color color = {0xFF, 0x00, 0x00};
    const char *text = ("$" + std::to_string(state.get_money())).c_str();
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);

    SDL_Texture *message = SDL_CreateTextureFromSurface(
        renderer, surface); // now you can convert it into a texture

    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(message, NULL, NULL, &texW, &texH);
    SDL_Rect dstrect = {0, 0, texW, texH};

    SDL_RenderCopy(renderer, message, NULL, &dstrect);
    SDL_RenderPresent(renderer);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(message);
    SDL_UpdateWindowSurface(window);
  }

  SDL_DestroyRenderer(renderer);

  TTF_CloseFont(font);
}

int main() {

  State state(100.0);

  Technology new_tech = Technology(1.5);
  Node<Technology> tech_node = Node<Technology>(new_tech);
  std::cout << "Hello World!" << std::endl;
  std::cout << tech_node.get_data().get_cost() << std::endl;

  // The window we'll be rendering to
  SDL_Window *window = NULL;

  // The surface contained by the window
  SDL_Surface *screenSurface = NULL;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) >= 0 && TTF_Init() >= 0) {
    // Create window
    window =
        SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if (window == NULL) {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    } else {
      // Get window surface
      // screenSurface = SDL_GetWindowSurface(window);

      // Fill the surface white
      // SDL_FillRect(screenSurface, NULL,
      //              SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

      // Update the surface
      // SDL_UpdateWindowSurface(window);

      // Simulate
      simulation(state, window);
    }
  } else {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  }

  // Destroy window
  SDL_DestroyWindow(window);

  // Quit SDL subsystems
  TTF_Quit();
  SDL_Quit();

  return 0;
}
