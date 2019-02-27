/* This file is used for drawing out the time in clocks that actually use such silly constructs 
as readable times.  Stolen liberally from Print.h, matrix.h, matrix.cpp, etc etc... The small 
font used (4x7) is stored in globals.h, and contains basically just numbers, capital letters, 
and some puncuation at this point. Adding more valid characters is on the list of things to do... 

At some point. */

uint8_t cursor_x, cursor_y, textsize, textcolor;

void setCursor(uint8_t x, uint8_t y) {
	cursor_x = x;
	cursor_y = y;
}

void setTextColor(uint8_t c) {
	textcolor = c;
}

// draw a character
void drawCharSm(uint8_t x, uint8_t y, char c, uint16_t color) {
	for (uint8_t i = 0; i<4; i++) {
		uint8_t line = pgm_read_byte(fontSm + (c * 4) + i);
		for (uint8_t j = 0; j<8; j++) {
			if (line & 0x1) {
					matrix.drawPixel(x + i, y + j, color);
				    setDispPixel(x + i, y + j, 0, color);
			}
			line >>= 1;
		}
	}
}

size_t writeSmall(int c) {
	if (c == '\n') {
		cursor_y += 8;
		cursor_x = 0;
	}
	else if (c == '\r') {
		// skip em
	}
	else {
		drawCharSm(cursor_x, cursor_y, c, textcolor);
		cursor_x += 5;
	}

}

size_t printSm(const String &s)
{
	size_t n = 0;
	for (uint16_t i = 0; i < s.length(); i++) {
		n += writeSmall(s[i]);
	}
	return n;
}
