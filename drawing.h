#include "globals.h"
#define swap(a, b) { uint16_t t = a; a = b; b = t; }

void drawPixel(uint8_t x, uint8_t y, bool state, uint8_t num = 0) 
{
	setDispPixel(x, y, num, state);
}

// bresenham's algorithm - thx wikpedia
void drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1, bool state, uint8_t num = 0) {
	uint16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	uint16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;}

	for (; x0<=x1; x0++) {
		if (steep) {
			drawPixel(y0, x0, state, num);
		} else {
			drawPixel(x0, y0, state, num);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

// draw a rectangle
void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool state, uint8_t num = 0) {
	drawLine(x, y, x+w-1, y, state, num);
	drawLine(x, y+h-1, x+w-1, y+h-1, state, num);

	drawLine(x, y, x, y+h-1, state, num);
	drawLine(x+w-1, y, x+w-1, y+h-1, state, num);
}

// fill a rectangle
void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool state, uint8_t num = 0) {
	for (uint8_t i=x; i<x+w; i++) {
		for (uint8_t j=y; j<y+h; j++) {
			drawPixel(i, j, state, num);
		}
	}
}

void swapWorlds(uint8_t a, uint8_t b)
{
	if(a < 0 || a > 2 || b < 0 || b > 2) return;
	noInterrupts();
	uint8_t temp = worldIndex[a];
	worldIndex[a] = worldIndex[b];
	worldIndex[b] = temp;
	interrupts();
} 