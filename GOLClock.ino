/*
* Conway's "Life" Clock works backwards, showing "random" display that reassembles to clock face... still in progress!
* It also does other modes, some of which are still being figured out and refined.
* Adapted from the Life example
* on the Processing.org site
*
* Further adapted by Josh Ward
* 
*/
#include <Wire.h>
#include "HT1632.h"
#include "RTClib.h"

RTC_DS1307 RTC;

#define DATA 12
#define WR   13
#define CS   4
#define DELAY 500

HT1632LEDMatrix matrix = HT1632LEDMatrix(DATA, WR, CS);

#include "globals.h"
#include "drawing.h"
#include "numbers.h"
#include "settime.h"
uint8_t BRIGHT = 1;

void setup() {
	timeRef = 0;

	//Set PORTD2 and PORTD3 as inputs
	DDRD &= ~(_BV(PIND2) | _BV(PIND3));
	//Enable set/change button pullups
	PORTD |= (_BV(PIND2) | _BV(PIND3));
	//Enable I2C pullups - probably not really necessary since twi.h does this for you
	PORTC |= (_BV(PINC4) | _BV(PINC5));

	//see setResetDisable(bool) for info
	DDRD &= ~(_BV(PIND5)); //Set PIND5 as input initially
	PORTD |= (_BV(PIND5)); //Set high

	//check for buttons held down at power up
	bSave = BUTTON_STATE;

	Serial.begin(115200);
	Wire.begin();
	RTC.begin();

	pinMode(17, OUTPUT); //These two are ONLY used while it's on the Arduino. When moved to actual device, delete these!
	pinMode(16, OUTPUT);

	pinMode(8, INPUT);
	pinMode(9, INPUT);
	pinMode(10, INPUT);
	pinMode(11, INPUT);
	digitalWrite(2, HIGH);
	digitalWrite(3, HIGH);
	digitalWrite(8, HIGH);
	digitalWrite(9, HIGH);
	digitalWrite(10, HIGH);
	digitalWrite(11, HIGH);

	digitalWrite(17, HIGH); //These two are ONLY used while it's on the Arduino. When moved to actual device, delete these!
	digitalWrite(16, LOW);

	if (digitalRead(11) == LOW)
		BRIGHT = 1;
	else if (digitalRead(11) == HIGH)
		BRIGHT = 5;

	matrix.begin(HT1632_COMMON_16NMOS); 
	matrix.setBrightness(BRIGHT);
	matrix.fillScreen();
	delay(500);
	matrix.clearScreen();

	uint8_t hour = 0;
	uint8_t hourten = 0;
	uint8_t hourone = 0;
	uint8_t minute = 0;
	uint8_t minuteten = 0;
	uint8_t minuteone = 0;
	uint8_t top = 0;
	uint8_t egg = 0;

	//Grab current time and set to compile time if RTC is uniitialized

	dt_now = RTC.now();

	if(dt_now.unixtime() <= 946684860) //RTC not initialized and oscillator disabled
	{
		dt_now = DateTime(__DATE__, __TIME__);
		RTC.adjust(dt_now);
	}

	setInterrupts();
	setupDisplayTimer();
}

void loop() {
	if(curState == STATE_CLOCK)
	{  
		static DateTime now;
		now = RTC.now();
		dt_now = RTC.now();
		if (digitalRead(11) == HIGH)
			BRIGHT = 1;
		else if (digitalRead(11) == LOW)
			BRIGHT = 15;
		matrix.setBrightness(BRIGHT);
		if ((digitalRead(8) == HIGH) && (digitalRead(9) == HIGH) && (digitalRead(10) == HIGH)) //000
			forwardClock(now);
		else if ((digitalRead(8) == HIGH) && (digitalRead(9) == HIGH) && (digitalRead(10) == LOW)) //001
			staticClock(now);
		else if ((digitalRead(8) == HIGH) && (digitalRead(9) == LOW) && (digitalRead(10) == HIGH)) //010
			meltClock(now);
		else if ((digitalRead(8) == HIGH) && (digitalRead(9) == LOW) && (digitalRead(10) == LOW)) //011
			brightClock(now);
		else if ((digitalRead(8) == LOW) && (digitalRead(9) == HIGH) && (digitalRead(10) == HIGH)) //100
			backwardClock(now);
		else if ((digitalRead(8) == LOW) && (digitalRead(9) == HIGH) && (digitalRead(10) == LOW)) //101
			chartClock(now);
		else if ((digitalRead(8) == LOW) && (digitalRead(9) == LOW) && (digitalRead(10) == HIGH)) //110
			binaryClock(now);
		else if ((digitalRead(8) == LOW) && (digitalRead(9) == LOW) && (digitalRead(10) == LOW)) //111
			epochClock(now);
	}
	else if(curState == STATE_MANUAL_SET)
	{
		matrix.setTextSize(1);    // size 1 == 8 pixels high
		matrix.setTextColor(1);   // 'lit' LEDs
		if(timeReady)
		{
			adjustFromSetVal();
			curState = STATE_CLOCK; 
		}
		else if(setChanged)
		{

			//This is where all the time setting code goes!
			switch(curSet) {
			case SET_YR:
				matrix.clearScreen(); 
				matrix.setCursor(0, 0);   // start at top left, with one pixel of spacing
				matrix.print("Year ");
				matrix.setCursor(0, 8);   // next line, 8 pixels down
				matrix.print(setValues[SET_YR]);
				matrix.writeScreen();
				break;
			case SET_MON:
				matrix.clearScreen(); 
				matrix.setCursor(0, 0);   // start at top left, with one pixel of spacing
				matrix.print("Mon ");
				matrix.setCursor(0, 8);   // next line, 8 pixels down
				matrix.print(setValues[SET_MON]);
				matrix.writeScreen();
				break;
			case SET_DAY:
				matrix.clearScreen(); 
				matrix.setCursor(0, 0);   // start at top left, with one pixel of spacing
				matrix.print("Day ");
				matrix.setCursor(0, 8);   // next line, 8 pixels down
				matrix.print(setValues[SET_DAY]);
				matrix.writeScreen();
				break;
			case SET_HR:
				matrix.clearScreen(); 
				matrix.setCursor(0, 0);   // start at top left, with one pixel of spacing
				matrix.print("Hour ");
				matrix.setCursor(0, 8);   // next line, 8 pixels down
				matrix.print(setValues[SET_HR]);
				matrix.writeScreen();  
				break;
			case SET_MIN:
				matrix.clearScreen(); 
				matrix.setCursor(0, 0);   // start at top left, with one pixel of spacing
				matrix.print("Min ");
				matrix.setCursor(0, 8);   // next line, 8 pixels down
				matrix.print(setValues[SET_MIN]);
				matrix.writeScreen(); 
				break; 
			}
			setChanged = false;
		}
		else if(setCancel)
		{
			curState = STATE_CLOCK;
		}

	}
}

void backwardClock(DateTime now) {
	DateTime future = now.unixtime() + 60; //it's gonna be the future soon
	displayTime(future);
	for (int bigsteps = 29; bigsteps > -1; bigsteps--) {
		// 32 steps is the most I can fit in a minute right now.  
		//save the time back into world 0
		for (uint8_t y = 0; y < matrix.height(); y++) { 
			for (uint8_t x = 0; x < matrix.width(); x++) {
				setDispPixel(x, y, 0, getDispPixel(x, y, 2));
			}
		}
		cycle(bigsteps); // Play the game of life "bigsteps" number of times 
	} //end for loop

} //end backwardClock



void forwardClock(DateTime now) {
	displayTime(now);
	for (uint8_t y = 0; y < matrix.height(); y++) { 
		for (uint8_t x = 0; x < matrix.width(); x++) {
			setDispPixel(x, y, 0, getDispPixel(x, y, 2));
		}
	}

	displayFrame();
	delay(4000);
	for (uint8_t y = 0; y < matrix.height(); y++) { 
		for (uint8_t x = 0; x < matrix.width(); x++) {
			setDispPixel(x, y, 0, getDispPixel(x, y, 2));
		}
	}
	while(1) { 
		//save the time back into world 0

		for (uint8_t y = 0; y < matrix.height(); y++) { 
			for (uint8_t x = 0; x < matrix.width(); x++) {
				// Default is for cell to stay the same
				setDispPixel(x, y, 1, getDispPixel(x, y, 0));
				uint8_t count = neighbours(x, y); 

				if (count == 3 && !(getDispPixel(x, y, 0))) {
					// A new cell is born
					setDispPixel(x, y, 1, 1);
				} 

				if ((count < 2 || count > 3) && getDispPixel(x, y, 0)) {
					// Cell dies
					setDispPixel(x, y, 1, 0);
				}
			}
		}
		// Copy next generation into place
		for (uint8_t y = 0; y < matrix.height(); y++) { 
			for (uint8_t x = 0; x < matrix.width(); x++) {
				setDispPixel(x, y, 0, getDispPixel(x, y, 1));
			} 
		}
		displayFrame();
		DateTime now = RTC.now();
		if (now.second() >= 58)
			break;

	} //end GOL loop
} //end forwardClock

void staticClock(DateTime now) { //Cull the current time out of static...
	displayTime(now);

	randomSeed(analogRead(5));
	for (uint8_t y = 0; y < matrix.height(); y++) { 
		for (uint8_t x = 0; x < matrix.width(); x++) {
			randPixel(x, y, 0, 90);
		}
	}
	displayFrame();
	while (1) {  
		for (uint8_t y = 0; y < matrix.height(); y++) { 
			for (uint8_t x = 0; x < matrix.width(); x++) {
				if ((getDispPixel(x, y, 0) == getDispPixel(x, y, 2)) && !(getDispPixel(x, y, 0))) {
					if (random(100) < 80) //Change 80 for different densities of correct pixels
						setDispPixel(x, y, 1, getDispPixel(x, y, 0));
					else
						randPixel(x, y, 1, 50);
				} 
				else if ((getDispPixel(x, y, 0) == getDispPixel(x, y, 2)) && (getDispPixel(x, y, 0))) {
					if (random(100) < 98) //Change 98 for different densities of correct pixels
						setDispPixel(x, y, 1, getDispPixel(x, y, 0));
					else
						randPixel(x, y, 1, 50);
				}
				else {
					randPixel(x, y, 1, 50);
				}
			}
		}
		//copy new frame into current  
		for (uint8_t y = 0; y < matrix.height(); y++) { 
			for (uint8_t x = 0; x < matrix.width(); x++) {
				setDispPixel(x, y, 0, getDispPixel(x, y, 1));
			} 
		}
		displayFrame();
		DateTime now = RTC.now();
		if (now.second() >= 58)
			break;
	}
} // end staticClock


void meltClock(DateTime now) { //melt the time...
	displayTime(now);
	for (uint8_t y = 0; y < matrix.height(); y++) { //show the current time
		for (uint8_t x = 0; x < matrix.width(); x++) {
			setDispPixel(x, y, 0, getDispPixel(x, y, 2));
		} 
	}
	displayFrame();
	delay(4000);
	randomSeed(analogRead(5));

	while (1) {  
		for (uint8_t y = 0; y < matrix.height(); y++) { 
			for (uint8_t x = 0; x < matrix.width(); x++) {
				meltPixel(x, y, 1, 2);
			}
		}
		//copy new frame into current  
		for (uint8_t y = 0; y < matrix.height(); y++) { 
			for (uint8_t x = 0; x < matrix.width(); x++) {
				setDispPixel(x, y, 0, getDispPixel(x, y, 1));
			} 
		}
		displayFrame();
		DateTime now = RTC.now();
		if (now.second() >= 58)
			break;
	}
} // end meltClock

void brightClock(DateTime now) {
	uint8_t hrs = now.hour();
	uint8_t partial = ((now.minute() * 60) + now.second())/225;
	for (uint8_t y = 0; y < matrix.height(); y++) { 
		for (uint8_t x = 0; x < matrix.width(); x++) {
			if (x < hrs) { //complete rows
				setDispPixel(x, y, 0, 1);
			}
			else if (x == hrs) { //part of row... seconds / 225 for a height of 16.
				if (y < partial) {
					setDispPixel(x, y, 0, 1);
				}
				else {
					setDispPixel(x, y, 0, 0);
				}
			}
			else { // row off
				setDispPixel(x, y, 0, 0);
			}
		} 
	}

	displayFrame();
}

void chartClock(DateTime now) {
	//Get julian date
	int partyear, partmonth, partday, parthour, partminute = 0;
	uint8_t m = now.month();
	uint8_t d = now.day();
	uint8_t h = now.hour();
	uint8_t mm = now.minute();
	uint8_t s = now.second();
	int jd = now.day();
	if (m > 1)  jd += 31;
	if (m > 2)  jd += 28; // doesn't count leapyears... good enough.
	if (m > 3)  jd += 31;
	if (m > 4)  jd += 30;
	if (m > 5)  jd += 31;
	if (m > 6)  jd += 30;
	if (m > 7)  jd += 31;
	if (m > 8)  jd += 31;
	if (m > 9)  jd += 30;
	if (m > 10) jd += 31;
	if (m > 11) jd += 30;

	partyear = jd / 15.2;

	partmonth = (d * 24 + h) / (pgm_read_byte(&daysInMonth[m - 1]));

	partday = (h * 60 + mm)/60;

	parthour = ((now.minute() * 60) + now.second())/150;

	partminute = s / 2.5;

	for (uint8_t y = 0; y < 16; y++) {
		for (uint8_t x = 0; x < 24; x++) {
			if ((y == 1) || (y == 2)) {
				if (partyear > x) {
					setDispPixel(x, y, 0, 1);
				}
				else setDispPixel(x, y, 0, 0);
			}
			if ((y == 4) || (y == 5)) {
				if (partmonth > x) {
					setDispPixel(x, y, 0, 1);
				}
				else setDispPixel(x, y, 0, 0);
			}
			if ((y == 7) || (y == 8)) {
				if (partday > x) {
					setDispPixel(x, y, 0, 1);
				}
				else setDispPixel(x, y, 0, 0);
			}
			if ((y == 10) || (y == 11)) {
				if (parthour > x) {
					setDispPixel(x, y, 0, 1);
				}
				else setDispPixel(x, y, 0, 0);
			}
			if ((y == 13) || (y == 14)) {
				if (partminute > x) {
					setDispPixel(x, y, 0, 1);
				}
				else setDispPixel(x, y, 0, 0);
			}
		}
	}
	displayFrame();
} //end chartClock

void binaryClock(DateTime now){

	uint8_t month = now.month();
	uint8_t day = now.day();
	uint8_t	hour = now.hour();
	uint8_t minute = now.minute();
	uint8_t second = now.second();

	//setup grid
	drawRect(4, 0, 3, 3, 1);
	drawRect(8, 0, 3, 3, 1);
	drawRect(12, 0, 3, 3, 1);
	drawRect(16, 0, 3, 3, 1);
	drawRect(20, 0, 3, 3, 1);

	drawRect(0, 6, 3, 3, 1);
	drawRect(4, 6, 3, 3, 1);
	drawRect(8, 6, 3, 3, 1);
	drawRect(12, 6, 3, 3, 1);
	drawRect(16, 6, 3, 3, 1);
	drawRect(20, 6, 3, 3, 1);

	drawRect(0, 12, 3, 3, 1);
	drawRect(4, 12, 3, 3, 1);
	drawRect(8, 12, 3, 3, 1);
	drawRect(12, 12, 3, 3, 1);
	
	drawRect(16, 12, 3, 3, 1);
	drawRect(20, 12, 3, 3, 1);

	//HOUR
	if (hour >= 16){
		drawPixel(5, 1, 1);
		hour -= 16;
	}
	else drawPixel(5, 1, 0);
	if (hour >= 8){
		drawPixel(9, 1, 1);
		hour -= 8;
	}
	else drawPixel(9, 1, 0);

	if (hour >= 4){
		drawPixel(13, 1, 1);
		hour -= 4;
	}
	else drawPixel(13, 1, 0);

	if (hour >= 2) {
		drawPixel(17, 1, 1);
		hour -= 2;
	}
	else drawPixel(17, 1, 0);

	if (hour >= 1) {
		drawPixel(21, 1, 1);
	}
	else drawPixel(21, 1, 0);

	//MINUTE
	if (minute >= 32){
		drawPixel(1, 7, 1);
		minute -= 32;
	}
	else drawPixel(1, 7, 0);
	if (minute >= 16){
		drawPixel(5, 7, 1);
		minute -= 16;
	}
	else drawPixel(5, 7, 0);
	if (minute >= 8){
		drawPixel(9, 7, 1);
		minute -= 8;
	}
	else drawPixel(9, 7, 0);

	if (minute >= 4){
		drawPixel(13, 7, 1);
		minute -= 4;
	}
	else drawPixel(13, 7, 0);

	if (minute >= 2) {
		drawPixel(17, 7, 1);
		minute -= 2;
	}
	else drawPixel(17, 7, 0);

	if (minute >= 1) {
		drawPixel(21, 7, 1);
	}
	else drawPixel(21, 7, 0);

	//SECOND
	if (second >= 32){
		drawPixel(1, 13, 1);
		second -= 32;
	}
	else drawPixel(1, 13, 0);
	if (second >= 16){
		drawPixel(5, 13, 1);
		second -= 16;
	}
	else drawPixel(5, 13, 0);
	if (second >= 8){
		drawPixel(9, 13, 1);
		second -= 8;
	}
	else drawPixel(9, 13, 0);

	if (second >= 4){
		drawPixel(13, 13, 1);
		second -= 4;
	}
	else drawPixel(13, 13, 0);

	if (second >= 2) {
		drawPixel(17, 13, 1);
		second -= 2;
	}
	else drawPixel(17, 13, 0);

	if (second >= 1) {
		drawPixel(21, 13, 1);
	}
	else drawPixel(21, 13, 0);

	displayFrame();
	delay(100);

} //end binaryClock

void epochClock(DateTime now) {
	static uint32_t unix;
	unix = now.unixtime();

	matrix.fillRect(0,0,24,16,0);
	setCursor(0, 0);
	setTextColor(1);

	static char topS[6]; //6 bytes for null terminator!
	getUnixStringSeg(unix / 100000, topS);
	static char botS[6]; //static so we don't keep allocating this
	getUnixStringSeg(unix % 100000, botS);

	printSm(topS);
	setCursor(0, 8);
	printSm(botS);
	matrix.writeScreen();

	delay(100);

} //end epochClock

uint8_t meltPixel (uint8_t x, uint8_t y, uint8_t z, uint8_t seed) {
	if (getDispPixel(x, y - 1, 0) && (random(100) < seed)) {
		if (randPixel(x, y, 1, 50) == 1)
			setDispPixel(x, y - 1, 1, 0);
		else
			setDispPixel(x, y, 1, getDispPixel(x, y, 0));
	}
	else
		setDispPixel(x, y, 1, getDispPixel(x, y, 0));
}

bool randPixel (uint8_t x, uint8_t y, uint8_t z, uint8_t seed) {
	if (random(100) < seed)
		setDispPixel(x, y, z, 1);
	else
		setDispPixel(x, y, z, 0);
	return getDispPixel(x, y, z);
}

void cycle(byte bigsteps) {  //this plays the game in reverse
	byte backsteps = bigsteps;
	while (backsteps >= 1) {

		for (uint8_t y = 0; y < matrix.height(); y++) { 
			for (uint8_t x = 0; x < matrix.width(); x++) {
				// Default is for cell to stay the same
				setDispPixel(x, y, 1, getDispPixel(x, y, 0));
				uint8_t count = neighbours(x, y); 

				if (count == 3 && !(getDispPixel(x, y, 0))) {
					// A new cell is born
					setDispPixel(x, y, 1, 1);
				} 

				if ((count < 2 || count > 3) && (getDispPixel(x, y, 0))) {
					// Cell dies
					setDispPixel(x, y, 1, 0);
				}
			}
		}
		backsteps--;
		// Copy next generation into place
		for (uint8_t y = 0; y < matrix.height(); y++) { 
			for (uint8_t x = 0; x < matrix.width(); x++) {
				setDispPixel(x, y, 0, getDispPixel(x, y, 1));
			} 
		}
	}
	displayFrame();
} //end cycle

void displayFrame() { //Spit current frame out to the display
	_displayUpdated = true;
	delay(DELAY);
} //end displayFrame

void clearFrame() { //clears all buffers and starts fresh
	for (uint8_t y = 0; y<matrix.height(); y++) {
		for (uint8_t x = 0; x< matrix.width(); x++) {
			for (uint8_t z = 0; z < 3; z++) {
				setDispPixel(x, y, z, 0);
			}
		}
	}


}

int neighbours(int x, int y) { //Calculate how many living neighbors each pixel has.
	return  getDispPixel((x + 1) % matrix.width(), y, 0) +
		getDispPixel(x, ((y + 1) % matrix.height()), 0) +
		getDispPixel((x + matrix.width() - 1) % matrix.width(), y, 0) +
		getDispPixel(x, (y + matrix.height() - 1) % matrix.height(), 0) +
		getDispPixel((x + 1) % matrix.width(), (y + 1) % matrix.height(), 0) +
		getDispPixel((x + matrix.width() - 1) % matrix.width(), (y + 1) % matrix.height(), 0) +
		getDispPixel((x + matrix.width() - 1) % matrix.width(), (y + matrix.height() - 1) % matrix.height(), 0) +
		getDispPixel((x + 1) % matrix.width(), (y + matrix.height() - 1) % matrix.height(), 0);
} //end neighbors


void displayTime(DateTime now) {
	//clear the disply and clock buffer
	for (uint8_t y = 0; y < matrix.height(); y++) { 
		for (uint8_t x = 0; x < matrix.width(); x++) {
			setDispPixel(x, y, 0, 0);
			setDispPixel(x, y, 2, 0);
		}
	}

	uint8_t top = 0;
	uint8_t hour = now.hour();
	uint8_t minute = now.minute();

	uint8_t bottom = 8;
	uint8_t day = now.day();
	uint8_t month = now.month();

	char hours[3];
	memset(hours, 0, sizeof(char)* 3);
	itoa(hour, hours, DEC);
	char minutes[3];
	memset(minutes, 0, sizeof(char)* 3);
	itoa(minute, minutes, DEC);
	char months[3];
	memset(months, 0, sizeof(char)* 3);
	itoa(month, months, DEC);
	char days[3];
	memset(days, 0, sizeof(char)* 3);
	itoa(day, days, DEC);

	setCursor(0, 0);
	setTextColor(1);

	if (hour < 10)
		printSm(" ");
	printSm(hours);
	printSm(":");
	if (minute < 10)
		printSm("0");
	printSm(minutes);
	setCursor(0, 8);
	if (month < 10)
		printSm(" ");
	printSm(months);
	printSm("/");
	if (day < 10)
		printSm("0");
	printSm(days);

	//save it into world 2
	for (uint8_t y = 0; y < matrix.height(); y++) { 
		for (uint8_t x = 0; x < matrix.width(); x++) {
			setDispPixel(x, y, 2, getDispPixel(x, y, 0));
			setDispPixel(x, y, 0, 0);//clear out world 0 again
		}
	}

} //end displaytime
