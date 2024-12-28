#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_surface.h>

#include "art/art.h"
#include "art/art_data.h"

#define SCREEN_WIDTH IMAGE_WIDTH
#define SCREEN_HEIGHT IMAGE_HEIGHT

uint32_t art_random() { return rand(); }

SDL_Surface *draw(struct Image *image) {
  SDL_Surface *surface =
      SDL_CreateRGBSurface(0, IMAGE_WIDTH, IMAGE_HEIGHT, 32, 0, 0, 0, 0);
  for (uint16_t y = 0; y < IMAGE_HEIGHT; y++)
    for (uint16_t x = 0; x < IMAGE_WIDTH; x++) {
      enum Color color = image_get(image, x, y);
      uint32_t *pixel =
          (uint32_t *)((uint8_t *)surface->pixels + y * surface->pitch +
                       x * surface->format->BytesPerPixel);
      switch (color) {
      case WHITE:
        *pixel = 0xFFD9DAD2;
        break;
      case BLACK:
        *pixel = 0xFF2D383A;
        break;
      case RED:
        *pixel = 0xFFB94E48;
        break;
      default:
        *pixel = 0x00000000;
        break;
      }
    }
  return surface;
}

static struct ArtData _data = {
    .forecast =
        {
            {.type = WEATHER_THUNDERSTORM,
             .temperature = -33,
             .hour = 23,
             .minute = 00},
            {.type = WEATHER_DRIZZLE,
             .temperature = -33,
             .hour = 12,
             .minute = 20},
            {.type = WEATHER_RAIN,
             .temperature = -33,
             .hour = 00,
             .minute = 40},
            {.type = WEATHER_SHOWER_RAIN,
             .temperature = -33,
             .hour = 07,
             .minute = 59},
        },
    .rain_density = 0,
    .clouds_count = 0,
    .minute = 6 * 60,
};

void core(SDL_Surface *screen_surface) {
  printf("clouds: %3d; rain: %4d; snow: %4d; hour: %02d:%02d\n",
         _data.clouds_count, _data.rain_density, _data.snow_density,
         _data.minute / 60, _data.minute % 60);

  struct Image *image = image_create();

  art_make(_data);
  art_draw(image);

  SDL_Rect dstrect = {
      .x = 0,
      .y = 0,
      .w = IMAGE_WIDTH,
      .h = IMAGE_HEIGHT,
  };

  SDL_Surface *surface = draw(image);
  SDL_BlitSurface(surface, NULL, screen_surface, &dstrect);
  SDL_FreeSurface(surface);

  image_destroy(image);
}

int main(int argc, char *args[]) {
  (void)argc;
  (void)args;
  art_init();
  srand(time(NULL));
  SDL_Window *window = NULL;
  SDL_Surface *screenSurface = NULL;
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
    return 1;
  }
  window = SDL_CreateWindow("hello_sdl2", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                            SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    fprintf(stderr, "could not create window: %s\n", SDL_GetError());
    return 1;
  }
  screenSurface = SDL_GetWindowSurface(window);
  SDL_FillRect(screenSurface, NULL,
               SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));

  core(screenSurface);

  SDL_UpdateWindowSurface(window);

  SDL_Event event;
  uint8_t quit = 0;
  while (!quit) {
    if (SDL_WaitEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = 1;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_q:
          quit = 1;
          continue;
        case SDLK_r:
          break;
        case SDLK_a:
          _data.clouds_count = (_data.clouds_count + 1) % 51;
          break;
        case SDLK_s:
          _data.rain_density = (_data.rain_density + 100) % 2100;
          break;
        case SDLK_d:
          _data.snow_density = (_data.snow_density + 100) % 2100;
          break;
        case SDLK_f:
          _data.minute = (_data.minute + 15) % (24 * 60);
          break;
        case SDLK_g:
          for (uint8_t i = 0; i < FORECAST_SIZE; i++) {
            _data.forecast[i].type =
                (_data.forecast[i].type + 1) % (WEATHER_WTF);
          }
          break;
        default:
          continue;
        }
        core(screenSurface);
        SDL_UpdateWindowSurface(window);
      } else {
        SDL_UpdateWindowSurface(window);
      }
    }
  }
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}