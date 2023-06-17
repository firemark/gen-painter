#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_surface.h>
#include <stdint.h>
#include <stdio.h>

#include "art.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 960


SDL_Surface *draw(struct Image *image) {
  SDL_Surface *surface =
      SDL_CreateRGBSurface(0, IMAGE_WIDTH, IMAGE_HEIGHT, 32, 0, 0, 0, 0);
  for (uint8_t y = 0; y < IMAGE_HEIGHT; y++)
    for (uint8_t x = 0; x < IMAGE_WIDTH; x++) {
      enum Color color = image_get(image, x, y);
      uint32_t *pixel =
          (uint32_t *)((uint8_t *)surface->pixels + y * surface->pitch +
                       x * surface->format->BytesPerPixel);
      switch (color) {
      case WHITE:
        *pixel = 0xFFFFFFFF;
        break;
      case BLACK:
        *pixel = 0xFF000000;
        break;
      case RED:
        *pixel = 0xFFFF0000;
        break;
      default:
        *pixel = 0x00000000;
        break;
      }
    }
  return surface;
}

void core(SDL_Surface *screen_surface) {
  struct Image image = {.x_offset = 0, .y_offset = 0};

  art_make();

  for (uint8_t y = 0; y < 4; y++) {
    for (uint8_t x = 0; x < 4; x++) {
      art_draw(&image);

      SDL_Rect rect = {.x = image.x_offset * 4,
                       .y = image.y_offset * 4,
                       .w = IMAGE_WIDTH * 4,
                       .h = IMAGE_HEIGHT * 4};

      SDL_Surface *surface = draw(&image);
      SDL_BlitScaled(surface, NULL, screen_surface, &rect);
      SDL_FreeSurface(surface);
      image.x_offset += IMAGE_WIDTH;
    }
    image.y_offset += IMAGE_HEIGHT;
    image.x_offset = 0;
  }
}

int main(int argc, char *args[]) {
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
      }
    }
  }
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}