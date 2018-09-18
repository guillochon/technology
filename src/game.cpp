// Technological progress game

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
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
  constexpr int base_hew = 1;

  int ew, hew;

  using clock = std::chrono::high_resolution_clock;

  std::chrono::nanoseconds lag(0ns);

  // Font stuff.
  TTF_Font *font = TTF_OpenFont("../fonts/OpenSans-Regular.ttf", 24);
  TTF_Font *small_font = TTF_OpenFont("../fonts/OpenSans-Regular.ttf", 16);

  if (font == NULL || small_font == NULL) {
    printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
  }

  // Main simulation loop.
  auto start = clock::now();

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  bool quit = false;

  SDL_Event e;

  long n_ticks = 0;

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
      n_ticks++;

      lag -= tick;

      if (n_ticks % 60 == 0) {
        std::string new_name = "t" + std::to_string(n_ticks);
        state.add_entity(new_name);
      }

      state.update();

      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderClear(renderer);

      const char *text = ("Year: " + state.date_str()).c_str();
      draw_text(renderer, font, text, 0, 0, true, false);

      int render_count = 0;
      std::vector<int> rgba(4);
      for (auto const &i : *state.entities_value()) {
        render_count++;
        double death_scale =
            (i.alive_value() ? 1.0
                             : (0.9 *
                                    std::max(Entity::corpse_lifetime -
                                                 i.time_since_death(),
                                             0.0) /
                                    Entity::corpse_lifetime +
                                0.1));
        if (i.age_value() < Entity::mating_age) {
          rgba = {0, 0, int(std::floor(death_scale * 255)), 255};
        } else if (i.age_value() > Entity::impotence_age) {
          rgba = {int(std::floor(death_scale * 255)), 0,
                  int(std::floor(death_scale * 255)), 255};
        } else {
          double mood_scale =
              0.5 * (1.0 + 2.0 / PI * std::atan(i.mood_value()));
          rgba = {int(std::floor(255 * death_scale * (1.0 - mood_scale))),
                  int(std::floor(255 * death_scale * mood_scale)), 0, 255};
        }

        hew = std::max(base_hew * i.current_mass(), 1.0);
        ew = 2 * hew;

        // DrawFilledCircle(renderer, i.x_value(), i.y_value(), hew);

        filledCircleRGBA(renderer, i.x_value(), i.y_value(), hew, rgba[0],
                         rgba[1], rgba[2], rgba[3]);

        // std::string status;
        //
        // if (!i.alive_value()) {
        //   status += "d";
        // } else {
        //   if (i.is_hungry())
        //     status += "h";
        //   if (i.will_mate())
        //     status += "m";
        // }
        //
        // if (status != "") {
        //   draw_text(renderer, small_font, status.c_str(), i.x_value(),
        //             i.y_value() + hew, true, true);
        // }
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
  constexpr double x_size = 1280 * 2;
  constexpr double y_size = 800 * 2;

  State state(100.0, 0l, x_size, y_size);

  for (int i = 0; i < 400; i++) {
    std::string name = std::to_string(i);
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
