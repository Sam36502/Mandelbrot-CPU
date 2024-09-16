#include "mandelbrot.h"


MBrot_View *MBrot_CreateView(int width, int height) {
	MBrot_View *view = SDL_malloc(sizeof(MBrot_View));

	view->screen_w = width;
	view->screen_h = height;
	view->pos_x = 0.0f;
	view->pos_y = 0.0f;
	view->zoom = 40.0f;
	view->julia_angle = 0.0f;
	view->julia_constant = 0.0f + 0.0f * I;
	view->iterations = 50;
	view->input_flags = 0x00;
	view->view_flags = 0x00;

	return view;
}

void MBrot_DestroyView(MBrot_View *view) {
	if (view == NULL) return;

	SDL_free(view);
}

bool MBrot_HandleTick(MBrot_View *view) {
	if (view == NULL) return false;

	static Uint64 t_last = 0;
	Uint64 t_now = SDL_GetTicks64();
	Uint64 elapsed = t_now - t_last;
	bool redraw = false;

	if (elapsed > TIME_INTERVAL) {
		t_last = t_now;

		// Smoothly zooms out to original zoom level
		if (view->view_flags & VIEW_FLAG_AUTO_ZOOM) {
			double factor = ZOOM_FACTOR;
			if (view->input_flags & INPUT_FLAG_SHIFT) factor = ZOOM_FACTOR_FAST;

			view->zoom *= 1/factor;
			if (view->zoom < 40.0f) {
				view->zoom = 40.0f;
				view->view_flags &= ~VIEW_FLAG_AUTO_ZOOM;
			}
			redraw = true;
		}

		// Smoothly rotates the Julia-set constant around (0 + 0i)
		if (view->view_flags & VIEW_FLAG_JULIA_ROT) {
			view->julia_angle += JULIA_SCROLL_VAL;
			if (view->julia_angle >= 2*M_PI) view->julia_angle -= 2*M_PI;

			redraw = true;
		}
	}

	return redraw;
}

bool MBrot_HandleEvent(MBrot_View *view, SDL_Event evt) {
	if (view == NULL) return false;
	bool redraw = false;

	switch (evt.type) {

		case SDL_KEYDOWN: {
			SDL_KeyCode kc = evt.key.keysym.sym;
			switch (kc) {
				case SDLK_LSHIFT:
				case SDLK_RSHIFT: view->input_flags |= INPUT_FLAG_SHIFT; break;
				case SDLK_LALT:
				case SDLK_RALT: view->input_flags |= INPUT_FLAG_ALT; break;
				case SDLK_F6: view->view_flags |= VIEW_FLAG_CROSSHAIR; break;
				default: break;
			}
			redraw = true;
		} break;

		case SDL_KEYUP: {
			SDL_KeyCode kc = evt.key.keysym.sym;
			switch (kc) {
				case SDLK_KP_PLUS: {
					if (view->iterations < 1000) view->iterations++;
				} break;
				case SDLK_KP_MINUS: {
					if (view->iterations > 0) view->iterations--;
				} break;

				case SDLK_LSHIFT:
				case SDLK_RSHIFT: view->input_flags &= ~INPUT_FLAG_SHIFT; break;
				case SDLK_LALT:
				case SDLK_RALT: view->input_flags &= ~INPUT_FLAG_ALT; break;
				case SDLK_F1: view->view_flags ^= VIEW_FLAG_SPECTRUM; break;
				case SDLK_F2: view->view_flags |= VIEW_FLAG_AUTO_ZOOM; break;
				case SDLK_F3: view->view_flags ^= VIEW_FLAG_JULIA; break;
				case SDLK_F4: view->view_flags ^= VIEW_FLAG_JULIA_ROT; break;
				case SDLK_F5: view->view_flags ^= VIEW_FLAG_FRM_TIME; break;
				case SDLK_F6: view->view_flags &= ~VIEW_FLAG_CROSSHAIR; break;
				default: break;
			};
			redraw = true;
		} break;

		case SDL_MOUSEBUTTONDOWN: view->input_flags |= INPUT_FLAG_MOUSE; break;
		case SDL_MOUSEBUTTONUP: view->input_flags &= ~INPUT_FLAG_MOUSE; break;

		case SDL_MOUSEMOTION: {
			if (view->input_flags & INPUT_FLAG_MOUSE) {
				double rel_x = (1/view->zoom) * evt.motion.xrel;
				double rel_y = -(1/view->zoom) * evt.motion.yrel;

				if (view->input_flags & INPUT_FLAG_ALT) {
					view->julia_constant += rel_x + rel_y * I;
				} else {
					view->pos_x -= rel_x;
					view->pos_y -= rel_y;
				}
			}

			redraw = true;
		} break;

		case SDL_MOUSEWHEEL: {
			if (view->view_flags & VIEW_FLAG_AUTO_ZOOM) break;
			double factor = ZOOM_FACTOR;
			if (view->input_flags & INPUT_FLAG_SHIFT) factor = ZOOM_FACTOR_FAST;

			if (evt.wheel.y < 0) {
				view->zoom *= 1/factor;
			} else {
				view->zoom *= factor;
			}

			if (view->zoom < 1) view->zoom = 1;
			redraw = true;
		} break;
	}

	return redraw;
}

// Internal function to draw the mandelbrot/julia set
static void __draw_fractal(MBrot_View *view, SDL_Renderer *renderer) {
	int screen_height = view->screen_w;
	int screen_width = view->screen_h;

	if (view->view_flags & VIEW_FLAG_SPECTRUM) screen_height -= 5;
	if (view->view_flags & VIEW_FLAG_FRM_TIME) screen_width -= 5;

	// Figure out which pixel is closest to the current julia constant
	int jp_x, jp_y = 0;
	bool show_julia_const = (view->view_flags & VIEW_FLAG_JULIA) && (view->input_flags & INPUT_FLAG_ALT);
	if (show_julia_const) {
		jp_x = (int)((creal(view->julia_constant) - view->pos_x) * view->zoom) + view->screen_w/2;
		jp_y = view->screen_h - ((int)((cimag(view->julia_constant) - view->pos_y) * view->zoom) + view->screen_h/2);
	}

	for (int y=0; y<screen_height; y++) {
		for (int x=0; x<screen_width; x++) {

			// If the current pixel is the constant, give it a fixed colour
			if (show_julia_const && (x == jp_x || y == jp_y)) {
				SDL_SetRenderDrawColor(renderer, CLR_JULIA_IS_CONST);
				SDL_RenderDrawPoint(renderer, x, y);
				continue;
			}

			double world_x = ((double) (x-view->screen_w/2) / view->zoom) + view->pos_x;
			double world_y = ((double)(view->screen_h - y - view->screen_h/2) / view->zoom) + view->pos_y;

			complex z = world_x + world_y * I;
			complex c;

			if (view->view_flags & VIEW_FLAG_JULIA_ROT) {
				c = view->julia_constant * (cosl(view->julia_angle) + sinl(view->julia_angle) * I);
			} else if (view->view_flags & VIEW_FLAG_JULIA) {
				c = view->julia_constant;
			} else {
				c = world_x + world_y * I;
			}

			SDL_SetRenderDrawColor(renderer, CLR_MANDEL_IN_SET);
			for (int i=0; i<view->iterations; i++) {

				z = z*z + c;
				double distance = sqrt(creall(z) * creall(z) + cimagl(z) * cimagl(z));
				if (distance > THRESHOLD) {
					double brightness = (double) i/view->iterations;
					SDL_SetRenderDrawColor(
						renderer,
						brightness * 0xFF,
						fabs(brightness - 0.5f) * 0xFF,
						(1.0f - brightness) * 0xFF,
						0xFF
					);
					break;
				}

			}

			SDL_RenderDrawPoint(renderer, x, y);

		}
	}
}

static void __draw_crosshair(MBrot_View *view, SDL_Renderer *renderer) {
	SDL_SetRenderDrawColor(renderer, 0xFF, 0x40, 0x40, 0xFF);
	int hw = view->screen_w/2;
	int hh = view->screen_h/2;
	SDL_RenderDrawLine(renderer, 0, hh, hw - 4, hh);
	SDL_RenderDrawLine(renderer, hw, 0, hw, hh - 4);
	SDL_RenderDrawLine(renderer, view->screen_w, hh, hw + 4, hh);
	SDL_RenderDrawLine(renderer, hw, view->screen_h, hw, hh + 4);
}

static void __draw_frame_time(MBrot_View *view, SDL_Renderer *renderer, double avg_frame_time) {
	int screen_height = view->screen_h;
	if (view->view_flags & VIEW_FLAG_SPECTRUM) screen_height -= 5;

	double max_mspf = 100.0f;
	double proportion = avg_frame_time / max_mspf;
	if (proportion > 1.0f) proportion = 1.0f;
	int barheight = proportion * screen_height;

	SDL_SetRenderDrawColor(
		renderer,
		proportion * 0xFF,
		1-proportion * 0xFF,
		0x40,
		0xFF
	);
	for (int y=0; y<barheight; y++) {
		SDL_RenderDrawLine(renderer, view->screen_w-4, screen_height-y, view->screen_w, screen_height-y);
	}
}

static void __draw_spectrum(MBrot_View *view, SDL_Renderer *renderer) {
	for (int x=0; x<view->screen_w; x++) {
		float screen_n = (float) x / view->screen_w;
		SDL_SetRenderDrawColor(
			renderer,
			screen_n * 0xFF,
			fabs(screen_n - 0.5f) * 0xFF,
			(1.0f - screen_n) * 0xFF,
			0xFF
		);
		SDL_RenderDrawLine(renderer, x, view->screen_h, x, view->screen_h-4);
	}
}

void MBrot_Draw(MBrot_View *view, SDL_Renderer *renderer) {
	if (view == NULL) return;

	Uint64 t_start = SDL_GetTicks64();

	__draw_fractal(view, renderer);

	// Calculate average frame time
	static double render_time_avg = 0.0f;
	double render_time = (double)(SDL_GetTicks64() - t_start);
	render_time_avg -= render_time_avg / FRAME_TIME_AVG_N;
	render_time_avg += render_time / FRAME_TIME_AVG_N;

	if (view->view_flags & VIEW_FLAG_SPECTRUM) __draw_spectrum(view, renderer);
	if (view->view_flags & VIEW_FLAG_CROSSHAIR) __draw_crosshair(view, renderer);
	if (view->view_flags & VIEW_FLAG_FRM_TIME) __draw_frame_time(view, renderer, render_time_avg);
}
