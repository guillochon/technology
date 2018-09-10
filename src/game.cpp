// Technological progress game

#include <SDL2/SDL.h>
#include <SDL/SDL_ttf.h>
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
  TTF_Font *Sans =
      TTF_OpenFont("Sans.ttf", 24); // this opens a font style and sets a size

  // Main simulation loop.
  auto start = clock::now();

  while (true) {
    auto dt = clock::now() - start;
    start = clock::now();
    lag += std::chrono::duration_cast<std::chrono::nanoseconds>(dt);

    while (lag >= tick) {
      lag -= tick;

      state.update();
    }

    std::cout << "$" << state.get_money() << std::endl;
  }
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
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  } else {
    // Create window
    window =
        SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if (window == NULL) {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    } else {
      // Get window surface
      screenSurface = SDL_GetWindowSurface(window);

      // Fill the surface white
      SDL_FillRect(screenSurface, NULL,
                   SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

      // Update the surface
      SDL_UpdateWindowSurface(window);

      // Simulate
      simulation(state, window);
    }
  }

  // Destroy window
  SDL_DestroyWindow(window);

  // Quit SDL subsystems
  SDL_Quit();

  return 0;
}
