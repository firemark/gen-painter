#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_surface.h>

#include "art.h"

#define SCREEN_WIDTH 1304
#define SCREEN_HEIGHT 984

uint32_t art_random() { return rand(); }

SDL_Surface *draw(struct Image *image, uint16_t w, uint16_t h) {
  SDL_Surface *surface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
  for (uint16_t y = 0; y < h; y++)
    for (uint16_t x = 0; x < w; x++) {
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

void core(SDL_Surface *screen_surface) {
  struct Image image = {.offset.x = 0, .offset.y = 0};

  art_make(-999, 128);

  uint16_t W[2] = {648, 656};
  uint16_t H[2] = {492, 492};

  for (uint8_t y = 0; y < 2; y++) {
    for (uint8_t x = 0; x < 2; x++) {
      art_draw(&image);

      SDL_Rect dstrect = {
          .x = image.offset.x,
          .y = image.offset.y,
          .w = W[x],
          .h = H[y],
      };

      SDL_Surface *surface = draw(&image, W[x], H[y]);
      SDL_BlitSurface(surface, NULL, screen_surface, &dstrect);
      SDL_FreeSurface(surface);
      image.offset.x += W[x];
    }
    image.offset.y += H[y];
    image.offset.x = 0;
  }
}

int main(int argc, char *args[]) {
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

  SDL_version compiled;
  SDL_version linked;

  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);
  printf("We compiled against SDL version %d.%d.%d ...\n", compiled.major,
         compiled.minor, compiled.patch);
  printf("But we are linking against SDL version %d.%d.%d.\n", linked.major,
         linked.minor, linked.patch);

  SDL_Event event;
  uint8_t quit = 0;
  while (!quit) {
    if (SDL_WaitEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = 1;
      } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q) {
        quit = 1;
      } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {

        core(screenSurface);
        SDL_UpdateWindowSurface(window);
      }
    }
  }
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}