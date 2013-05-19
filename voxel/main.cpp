/*  
	Copyright (c) 2013, Alexey Saenko
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/ 

#include <SDL.h>
#include "vrender.h"

#undef main

using namespace Render;

int main (int argc, char *argv[]) {

	printf("64x64x64 Voxel demo\n\nARROWS and CTRL+ARROWS to moving\nESC to quit\n1-5 to select scene\n\n");
	initRender(800, 480);

	SDL_Surface *screen;
	int done=0;
	SDL_Event event;
	Uint8 *keystate;

	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(getWidth(), getHeight(), 8, SDL_HWSURFACE|SDL_HWPALETTE);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't init video mode: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Color colors[256];
	memset(colors, 0, sizeof(colors));
	for(int i=0; i<32; i++) {
		int idx = (i+1)*6;
		for(int j=0; j<6; j++) {
			colors[idx+j].r = 16+(i&3)*64 + j*8;
			colors[idx+j].g = 16+((i&4)>>2)*112 + j*8;
			colors[idx+j].b = 16+((i&24)>>3)*64 + j*8;
		}
	}

	SDL_SetColors(screen, colors, 0, 256);
 
	initData(4);
	while(!done) {
		static unsigned ticks = SDL_GetTicks();

		render();

		unsigned t = SDL_GetTicks(); 

		printf("fps = %f\r", 1000.0f / (t - ticks) );
		ticks = t;

		if( SDL_LockSurface(screen) == 0 ) {
			Uint8 *dst = (Uint8 *)screen->pixels;
			memcpy(dst, getFrameBuffer(), getFrameBufferSize());
			SDL_UnlockSurface(screen);
		}
		SDL_UpdateRect(screen, 0, 0, 0, 0);

		while( SDL_PollEvent(&event) ) {
			switch(event.type) {
				case SDL_QUIT:
					done = 1;
					break;
				case SDL_KEYDOWN:
					switch( event.key.keysym.sym ) {
						case SDLK_ESCAPE:
							done = true;
							break;
						case SDLK_1:
							initData(1);
							break;
						case SDLK_2:
							initData(2);
							break;
						case SDLK_3:
							initData(3);
							break;
						case SDLK_4:
							initData(4);
							break;
						case SDLK_5:
							initData(5);
							break;
					}
					break;
			}
		}

		keystate = SDL_GetKeyState(NULL);

		float f=0.5f;
		if (keystate[SDLK_LCTRL]) {
			if (keystate[SDLK_LEFT])
				move(f, 0, 0);
			if (keystate[SDLK_RIGHT])
				move(-f, 0 , 0);
			if (keystate[SDLK_UP])
				move(0, 0, -f);
			if (keystate[SDLK_DOWN])
				move(0, 0, f);
		} else {
			if (keystate[SDLK_UP])
				move(0, f, 0);
			if (keystate[SDLK_DOWN])
				move(0, -f, 0);
			if (keystate[SDLK_LEFT])
				turn(-0.02f);
			if (keystate[SDLK_RIGHT])
				turn(0.02f);
		}
	}

	return 0;
}



