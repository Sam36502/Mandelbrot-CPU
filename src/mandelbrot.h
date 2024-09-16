//	
//		Handles most of the Mandelbrot-type code
//	
//		Also includes the Julia Sets
//	

#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stdbool.h>
#include <math.h>
#include <complex.h>
#include <SDL2/SDL.h>


#define CLR_MANDEL_IN_SET	0x00, 0x00, 0x00, 0xFF
#define CLR_JULIA_IS_CONST	0xFF, 0x00, 0x00, 0xFF

#define ZOOM_FACTOR 1.1f
#define ZOOM_FACTOR_FAST 1.5f
#define TIME_INTERVAL 50 // ms
#define THRESHOLD 2.0f // Range outside of which a point is no longer part of the set
#define JULIA_SCROLL_VAL 0.05f
#define JULIA_MAGNITUDE 0.3f
#define FRAME_TIME_AVG_N 10 // Number of frames to average across for performance measurement

#define INPUT_FLAG_MOUSE 0b00000001
#define INPUT_FLAG_SHIFT 0b00000010
#define INPUT_FLAG_ALT   0b00000100

#define VIEW_FLAG_JULIA     0b00000001
#define VIEW_FLAG_JULIA_ROT 0b00000010
#define VIEW_FLAG_SPECTRUM  0b00000100
#define VIEW_FLAG_FRM_TIME  0b00001000
#define VIEW_FLAG_AUTO_ZOOM 0b00010000
#define VIEW_FLAG_CROSSHAIR 0b00100000


typedef struct {
	int screen_w;
	int screen_h;
	double pos_x;
	double pos_y;
	double zoom;
	double julia_angle;
	complex julia_constant;
	int iterations;
	Uint8 input_flags;
	Uint8 view_flags;
} MBrot_View;


//	Create a new MBrot_View
//	
//	Returned View should be destroyed with `MBrot_DestroyView()`
MBrot_View *MBrot_CreateView(int width, int height);

//	Destroyes a previously created MBrot_View
//	
void MBrot_DestroyView(MBrot_View *view);

//	Handles view events that are called on a regular timer
//	
//	Returns whether a redraw has been triggered
bool MBrot_HandleTick(MBrot_View *view);

//	Handles SDL events for the given 
//	
//	Returns whether a redraw has been triggered
bool MBrot_HandleEvent(MBrot_View *view, SDL_Event evt);

//	Draws a given MBrot_View to an SDL_Renderer
//	
//	Draws all layers that are set active at once
//	E.g. Also draws the colour spectrum if that view flag is active
void MBrot_Draw(MBrot_View *view, SDL_Renderer *renderer);

#endif