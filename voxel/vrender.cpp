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

#include "vrender.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

template<typename T> inline T max(T a, T b)	{	return a>b? a : b;	}
template<typename T> inline T min(T a, T b)	{	return a<b? a : b;	}

namespace Render {

const int cx=64, cy=64, cz=64;

static uint8 data[cx][cy][cz];
static uint8 *frameBuffer = 0;
static int	width, height;

uint8	*getFrameBuffer()		{	return frameBuffer;		}
int		getFrameBufferSize()	{	return width*height;	}
int		getWidth()				{	return width;			}
int		getHeight()				{	return height;			}

static float posx, posy, posz, ang, xd, yd, xdd, cosang, sinang, incvx, incvy, vx, vy, incx, incy;
static	int	sx, xdir, ydir;

void	scanDown(int xscan, int yscan, int zscan, float d, int sy1, int sy2);
void	scanUp(int xscan, int yscan, int zscan, float d, int sy1, int sy2);

inline bool inside(int x, int y, int z) {
	return x>=0 && x<=cx+1 && y>=0 && y<=cy+1 && z>=0 && z<=cz+1;
}

inline uint8 getVoxel(int x, int y, int z) {
	if(x<1 || x>=cx+1 || y<1 || y>=cy+1 || z<1 || z>=cz+1)
		return 0;
	unsigned int v = data[x-1][y-1][z-1];
	if(v)
		return (v+1)*6;
	return 0;
}

inline void drawSegment(int sx, int sy1, int sy2, uint8 color) {
	uint8 *p0 = frameBuffer + sy1*width + sx;
	uint8 *p1 = frameBuffer + sy2*width + sx;
	while(p0 < p1) { 
		*p0 = color; 
		p0 += width; 
	}
}

void initRender(int w, int h) {
	width = w;
	height = h;
	xd = float(height);
	yd = float(height/2);
	xdd = 1.0f / xd;
	posx = -cx/2;
	posy = cy/2;
	posz = cz/2;
	ang = 0; 
	turn(0);

	frameBuffer = new uint8[width * height];
}

void destroyRender() {
	if(frameBuffer)
		delete frameBuffer;
}

void move(float dx, float dy, float dz)  { 
	posx +=  sinang*dx + cosang*dy; 
	posy += -cosang*dx + sinang*dy; 
	posz += dz;
}

void turn(float da)  { 
	ang += da;

	cosang = cosf(ang);
	sinang = sinf(ang);

	float f = 1.0f / (float)xd;
	incvx = -sinang * f; 
	incvy = cosang * f; 
}

void scanLine() {
	int xscan = (int)posx;	 
	int yscan = (int)posy; 
	int zscan = (int)posz;

	xdir = (vx>0)*2-1; 
	ydir = (vy>0)*2-1; 

	float xtemp = posx-xscan; 
	if (xdir > 0) 
		xtemp = 1.0f-xtemp;

	float ytemp = posy-yscan; 
	if (ydir > 0) 
		ytemp = 1.0f-ytemp;

	float d = xtemp*incy - ytemp*incx;

	int depth=128;
	for(; depth>0; depth--) {
		if(d < 0) { 
			if(inside(xscan+xdir, yscan, zscan)) 
				break;
			xscan += xdir;
			d += incy;
		} else {
			if(inside(xscan, yscan+ydir, zscan))
				break;
			yscan += ydir;
			d -= incx;
		}
	}	

	if(depth==0) {
		drawSegment(sx, 0, height, 0);
		return;
	}

	scanUp(xscan, yscan, zscan, d, height/2, 0);
	scanDown(xscan, yscan, zscan, d, height/2, height);
}

void render() {
	vx = cosang + sinang + incvx * 0.5f;
	vy = sinang - cosang + incvy * 0.5f;

	int d = height - width/2;
	vx += d*incvx;
	vy += d*incvy;


	for(sx=0; sx < width; sx++) {
		incx = fabsf(vx);
		incy = fabsf(vy);
		scanLine();
		vx += incvx;
		vy += incvy;
	}

}

void scanDown(int xscan, int yscan, int zscan, float d, int ymin, int ymax) {
	float hx, hy;

	int by = 0;

	if(d < 0) {
		xscan += xdir;
		d += incy;
		hx = (float)xscan; 
		if(xdir < 0) 
			hx += 1.0f;
		hy = vy*(hx-posx)/vx + posy;
		by += 2;
	} else {
		yscan += ydir;
		d -= incx;
		hy = (float)yscan; 
		if(ydir < 0) 
			hy += 1.0f;
		hx = vx*(hy-posy)/vy + posx;
	}

	if(!inside(xscan, yscan, zscan)) { 
		drawSegment(sx, ymin, ymax, 0);
		return;
	}

	float dist = cosang*(hx-posx) + sinang*(hy-posy);
	float f = xd/dist;

	int start = ymin;
	int startz = zscan;
	bool gap = false;

	for(int z=zscan; z<cz+2; ++z) {
		int sy2 = (int)((z-posz+1)*f + yd);
		if(sy2 < ymin)
			continue;

		if(sy2 - f - f > ymax)
			break;

		uint8 voxel = getVoxel(xscan, yscan, z);
		if(voxel) {
			int sy1 = (int)((z-posz+0)*f + yd);

			if(sy1 < ymax) {
				sy1 = max(sy1, ymin);
				sy2 = min(sy2, ymax);
				if(sy2 > sy1)
					drawSegment(sx, sy1, sy2, voxel+by );
			} else 
				sy1 = ymax;

			if(z-1 < 0 || !getVoxel(xscan, yscan, z-1)) {
				float thx, thy;
				if(d < 0) {
					thx = (float)(xscan); 
					if(xdir > 0) 
						thx += 1.0f;
					thy = vy*(thx-posx)/vx + posy;
				} else {
					thy = (float)(yscan); 
					if(ydir > 0) 
						thy += 1.0f;
					thx = vx*(thy-posy)/vy + posx;
				}
				float dist2 = cosang*(thx-posx) + sinang*(thy-posy);
				float f2 = xd/dist2;
				int tsy = (int)((z-posz-0)*f2 + yd);	 

				if(tsy > ymax)
					break;

				tsy = max(tsy, ymin);
				if(tsy<sy1) {
					drawSegment(sx, tsy, sy1, voxel+4);
					sy1 = tsy;
				}
			}

			if(gap) {
				if(sy1 > start)
					scanDown(xscan, yscan, startz, d, start, sy1);
				gap = false;
			}
			start = sy2;
			startz = z+1;
		} else {
			gap = true;
		}
	}
	if(ymax > start)
		scanDown(xscan, yscan, startz, d, start, ymax);

}

void scanUp(int xscan, int yscan, int zscan, float d, int ymax, int ymin) {
	float hx, hy; 

	int by = 0;

	if(d < 0) {
		xscan += xdir;
		d += incy;
		hx = (float)xscan; 
		if(xdir < 0) 
			hx += 1.0f;
		hy = vy*(hx-posx)/vx + posy;
		by+=2;
	} else {
		yscan += ydir;
		d -= incx;
		hy = (float)yscan; 
		if(ydir < 0) 
			hy += 1.0f;
		hx = vx*(hy-posy)/vy + posx;
	}

	if(!inside(xscan, yscan, zscan)) { 
		drawSegment(sx, ymin, ymax, 0);
		return;
	}

	float dist = cosang*(hx-posx) + sinang*(hy-posy);
	float f = xd/dist;

	int start = ymax;
	int startz = zscan;
	bool gap = false;

	for(int z=zscan; z>-0; --z) {
		int sy1 = (int)((z-posz+0)*f + yd);
		if(sy1 > ymax)
			continue;

		if(sy1 + f + f < ymin)
			break;

		uint8 voxel = getVoxel(xscan, yscan, z);
		if(voxel) {
			int sy2 = (int)((z-posz+1)*f + yd);

			if(sy2 > ymin) {
				sy1 = max(sy1, ymin);
				sy2 = min(sy2, ymax);
				if(sy2>sy1)
					drawSegment(sx, sy1, sy2, voxel+by );
			} else 
				sy2 = ymin;

			if(z+1>cz+2 || !getVoxel(xscan, yscan, z+1)) {	
				float thx, thy;
				if(d < 0) {
					thx = (float)(xscan); 
					if(xdir > 0) 
						thx += 1.0f;
					thy = vy*(thx-posx)/vx + posy;
				} else {
					thy = (float)(yscan); 
					if(ydir > 0) 
						thy += 1.0f;
					thx = vx*(thy-posy)/vy + posx;
				}
				float dist2 = cosang*(thx-posx) + sinang*(thy-posy);
				float f2 = xd/dist2;
				int tsy = (int)((z-posz+1)*f2 + yd);	 

				if(tsy < ymin)
					break;

				tsy = min(tsy, ymax);

				if(tsy > sy2) {
					drawSegment(sx, sy2, tsy, voxel+5);
					sy2 = tsy;
				}
			}

			if(gap) {
				if(sy2 < start)
					scanUp(xscan, yscan, startz, d, start, sy2);
				gap = false;
			}
			start = sy1;
			startz = z-1;
		} else {
			gap = true;
		}
	}
	if(ymin < start)
		scanUp(xscan, yscan, startz, d, start, ymin);
}

inline unsigned char randColor() {
	return rand()%31+1;
}

void buildScene1() {
	for(int x=0; x<cx; ++x)
		for(int y=0; y<cy; ++y)
			for(int z=0; z<cz; ++z) {
				if( ( ((x&7)==0) && ((y&7)==0) ) || ( ((x&7)==0) && ((z&7)==0) ) || ( ((z&7)==0) && ((y&7)==0) )  )
					data[x][y][z] = randColor();
				else
					data[x][y][z] = 0;
			}
}

void buildScene5() {
	for(int x=0; x<cx; ++x)
		for(int y=0; y<cy; ++y)
			for(int z=0; z<cz; ++z) {
				if( (x&3)==0 && (y&3)==0 && (z&3)==0)
					data[x][y][z] = randColor();
				else
					data[x][y][z] = 0;
			}
}

void buildScene2() {
	for(int x=0; x<cx; ++x)
		for(int y=0; y<cy; ++y)
			for(int z=0; z<cz; ++z) {
				if( (x&1) & (y&1) & (z&1))
					data[x][y][z] = randColor();
				else
					data[x][y][z] = 0;
			}
}


void buildScene3() {
	for(int x=0; x<cx; ++x)
		for(int y=0; y<cy; ++y) 
			for(int z=0; z<cz; ++z) 
				data[x][y][z] = 0;

	for(int x=0; x<cx; ++x)
		for(int y=0; y<cy; ++y) {
			data[x][y][cz-1] = randColor();
			data[x][y][0] = randColor();
		}

	for(int x=0; x<cx; ++x)
		for(int z=0; z<cz; ++z) {
			data[x][0][z] = randColor();
			data[x][cy-1][z] = randColor();
		}

	for(int y=0; y<cy; ++y)
		for(int z=0; z<cz; ++z) {
			data[cx-1][y][z] = randColor();
		}
}

void buildScene4() {
	for(int x=0; x<cx; ++x)
		for(int y=0; y<cy; ++y) 
			for(int z=0; z<cz; ++z) 
				data[x][y][z] = 0;

	for(int i=0; i<cx*cy*cz>>4; ++i)
		data[rand()%cx][rand()%cy][rand()%cz] = randColor();
}


void initData(int i) {
	memset(data, 0, sizeof(data));
	switch(i) {
		case 1:
			buildScene1();
			break;
		case 2:
			buildScene2();
			break;
		case 3:
			buildScene3();
			break;
		case 4:
			buildScene4();
			break;
		case 5:
			buildScene5();
			break;
	}
}


};
