#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <complex.h>
#include <math.h>
#include "mandelbrot.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256
#define CLR_BACKGROUND		0x00, 0x00, 0x00, 0xFF


// Basic error message utility
void err_msg(const char *msg);


// Global Variables
SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
MBrot_View *g_view = NULL;


int main(int argc, char* args[]) {

	// Initialisation
	if (SDL_Init(SDL_INIT_VIDEO) < 0) err_msg("Failed to initialise SDL");
	g_window = SDL_CreateWindow(
		"Mandelbrot rendering in software",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1024, 1024,
		SDL_WINDOW_SHOWN
	);
	if (g_window == NULL) err_msg("Failed to create Window");

	g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
	if (g_renderer == NULL) err_msg("Failed to create Renderer");
	if (SDL_RenderSetIntegerScale(g_renderer, true)) err_msg("Failed to set integer scaling");
	if (SDL_RenderSetLogicalSize(g_renderer, SCREEN_WIDTH, SCREEN_HEIGHT)) err_msg("Failed to set logical size");

	g_view = MBrot_CreateView(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Main Loop
	bool isRunning = true;
	bool redraw = true;
	SDL_Event curr_event;

	while (isRunning) {

		// Handle events
		int scode = SDL_WaitEventTimeout(&curr_event, 10);
		redraw |= MBrot_HandleTick(g_view);

		if (scode != 0) {
			redraw |= MBrot_HandleEvent(g_view, curr_event);

			if (curr_event.type == SDL_QUIT) isRunning = false;
		}

		// Draw Screen
		if (redraw) {
			SDL_SetRenderDrawColor(g_renderer, CLR_BACKGROUND);
			SDL_RenderClear(g_renderer);

			MBrot_Draw(g_view, g_renderer);

			SDL_RenderPresent(g_renderer);
			redraw = false;
		}

	}

	// Termination
	MBrot_DestroyView(g_view);
	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
	return 0;
}


void err_msg(const char *msg) {
	printf("[ERROR] %s: %s\n", msg, SDL_GetError());
	exit(1);
}