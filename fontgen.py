font_base = """#include <avr/io.h>
#include <avr/pgmspace.h> 
 
#ifndef FONT5X8_H
#define FONT5X8_H
/* USAGE
char c = 'a';
uint8_t index = pgm_read_byte(&index_list[c]);
uint8_t offset = pgm_read_byte(&offset_list[index]);
uint8_t width = pgm_read_byte(&widths[index]);
static uint8_t charData = 0;
for(int x=offset; x < width+offset; x++)
{
    charData = pgm_read_byte(&font[offset + x]);
    //use charData here
}
*/

//for each ascii code 0-128, index_list resolves it's value to it's index in
//the arrays offset_list[] and widths[]
static uint8_t index_list[] PROGMEM = { %INDEX_LIST% };

//gets the offset of a char in font[], given the index from above
static uint8_t offset_list[] PROGMEM = { %OFFSET_LIST% };

//gets the width in total bytes of a char, given the index from above
static uint8_t widths[] PROGMEM = { %WIDTHS% };

//stores the actual character data
static uint8_t font[] PROGMEM = {
%FONT_DATA%
};
#endif
"""

def getChar(c):
    if c < 32:
        return "SPECIAL"
    elif c == 32:
        return "Space"
    else:
        return chr(c)
    
font = open('font.txt')

default = int(font.readline())
print "Default Char: %d" % default

chars = {}
widths = {}
offsets = {}
offset = 0

while True:
    id = font.readline()

    if id.startswith("EOF"):
        break;
    split = id.split('-')

    code = int(split[0])
    width = int(split[1])
    char = [0]*width
    for y in range(8):
        line = font.readline()

        c = 0
        for x in range(len(line)):
            if line[x] == '0':
                char[x] |= (1 << y)
    chars[code] = char
    widths[code] = width
    offset += width
    offsets[code] = offset

char_count = len(chars)
print "Total Chars: %d" % char_count

count = 0
index_list = ""
offset_list = ""
width_list = ""
char_list = ""
offset = 0
#limiting to 0-127 to save memory
for i in range(128):
    if chars.has_key(i):
        index_list += "%d," % count
        offset_list += "%d," % offsets[i]
        width_list += "%d," % widths[i]
        char_list += "\t%s // %s\n" % ((''.join('0x%02x,' % x for x in chars[i])), getChar(i))
        count += 1
    else:
        index_list += "%d," % default

output = font_base.replace("%INDEX_LIST%", index_list).replace("%WIDTHS%", width_list).replace("%FONT_DATA%", char_list).replace("%OFFSET_LIST%", offset_list)
outfile = open("font.h", mode = 'w+')
outfile.write(output)
outfile.close();

print "Completed writing font.h"
