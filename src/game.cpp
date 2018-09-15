// Technological progress game

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <ctime>
#include <iostream>
#include <random>

#include "constants.h"
#include "entity.h"
#include "state.h"
#include "technology.h"
#include "tree.h"

using namespace std::chrono_literals;

void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x,
               int y, bool fast = false, bool centerh = false) {
  SDL_Color color = {0xFF, 0xFF, 0xFF};
  SDL_Surface *surface;
  if (fast) {
    surface = TTF_RenderText_Solid(font, text, color);
  } else {
    surface = TTF_RenderText_Blended(font, text, color);
  }
  SDL_Texture *message = SDL_CreateTextureFromSurface(renderer, surface);

  int texW = 0;
  int texH = 0;
  SDL_QueryTexture(message, NULL, NULL, &texW, &texH);

  int dx = x;
  int dy = y;

  if (centerh)
    dx -= texW / 2;
  SDL_Rect dstrect = {dx, dy, texW, texH};

  SDL_RenderCopy(renderer, message, NULL, &dstrect);

  SDL_FreeSurface(surface);
  SDL_DestroyTexture(message);
}

void simulation(State &state, SDL_Window *window) {
  // Clock stuff.
  constexpr std::chrono::nanoseconds tick(16ms);
  constexpr int entity_width = 4;
  constexpr int ew = entity_width;
  constexpr int hew = ew / 2;
  constexpr double year = 86400.0 * 365.0;

  using clock = std::chrono::high_resolution_clock;

  std::chrono::nanoseconds lag(0ns);

  // Font stuff.
  TTF_Font *font = TTF_OpenFont("../fonts/OpenSans-Regular.ttf", 24);
  TTF_Font *small_font = TTF_OpenFont("../fonts/OpenSans-Regular.ttf", 14);

  if (font == NULL || small_font == NULL) {
    printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
  }

  // Main simulation loop.
  auto start = clock::now();

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

  bool quit = false;

  SDL_Event e;

  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      // User requests quit
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    auto dt = clock::now() - start;
    start = clock::now();
    lag += std::chrono::duration_cast<std::chrono::nanoseconds>(dt);

    // SDL_Delay(subtick);

    while (lag >= tick) {
      lag -= tick;

      state.update();

      // SDL_Surface *screenSurface = SDL_GetWindowSurface(window);
      //
      // SDL_FillRect(screenSurface, NULL,
      //              SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderClear(renderer);

      const char *text = ("Year: " + state.date_str()).c_str();
      draw_text(renderer, font, text, 0, 0, true, false);

      for (auto const &i : *state.entities_value()) {
        double death_scale =
            i.alive_value() ? 1.0
                            : std::max(year - i.time_since_death(), 0.0) / year;
        if (i.age_value() < year) {
          SDL_SetRenderDrawColor(renderer, 0, 0, death_scale * 255, 255);
        } else {
          double mood_scale =
              0.5 * (1.0 + std::atan(2.0 * i.mood_value() / PI));
          SDL_SetRenderDrawColor(
              renderer, std::floor(255 * death_scale * (1.0 - mood_scale)),
              std::floor(255 * death_scale * mood_scale), 0, 255);
        }

        SDL_Rect rect = {int(std::round(i.x_value() - hew)),
                         int(std::round(i.y_value() - hew)), ew, ew};
        SDL_RenderFillRect(renderer, &rect);

        std::string status;

        if (i.is_hungry())
          status += "h";
        if (!i.alive_value())
          status += "d";

        if (status != "") {
          draw_text(renderer, small_font, status.c_str(), i.x_value(),
                    i.y_value(), true, true);
        }
      }

      SDL_RenderPresent(renderer);
      SDL_UpdateWindowSurface(window);
    }
  }

  SDL_DestroyRenderer(renderer);

  TTF_CloseFont(font);
  TTF_CloseFont(small_font);
}

int main() {
  constexpr double x_size = 640 * 2;
  constexpr double y_size = 480 * 2;

  State state(100.0, 0l, x_size, y_size);

  for (int i = 0; i < 200; i++) {
    std::string name = std::to_string(i);
    std::cout << name << std::endl;
    state.add_entity(name);
  }

  // The window we'll be rendering to
  SDL_Window *window = NULL;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) >= 0 && TTF_Init() >= 0) {
    // Create window
    window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, std::round(x_size / 2),
                              std::round(y_size / 2),
                              SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
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
