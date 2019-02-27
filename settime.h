//All the stuff involved in setting the time. Most of it is arcane digital spellcrafting that I barely comprehend at all. All glory to the Hypnotoad!
#include "TimerOne.h"

void displayRefresh()
{
	if(_displayUpdated)
	{
		_displayUpdated = false;
		for (uint8_t y=0; y<matrix.height(); y++) {
			for (uint8_t x=0; x< matrix.width(); x++) {
				matrix.drawPixel(x,y,getDispPixel(x, y, 0));
			}
		}
		matrix.writeScreen();
	}
}

inline void setupDisplayTimer()
{
	Timer1.initialize();
	Timer1.attachInterrupt(displayRefresh, 1000000 / 10); //10 Hz 
}

static void setTimeVals(int in, uint8_t idx) {
	setValues[idx    ] = in / 10;
	setValues[idx + 1] = in - (setValues[idx] * 10);
}
/*
Moves forward the value at curSet and constrains
to proper date/time conventions including months,
leap years, etc.
Much of this borrowed from: 
https://github.com/adafruit/TIMESQUARE-Watch/blob/master/examples/Watch/Set.pde
*/
inline void incrementSetValue()
{
	uint8_t upper, lower;
	upper = pgm_read_byte(&limit[curSet]);

	if((curSet == SET_MON) || (curSet == SET_DAY)) {
		lower = 1;
	}
	else {
		lower = 0;
	}

	if(curSet == SET_DAY) {
		// Cap most months at 30 or 31 days as appropriate
		uint8_t m = setValues[SET_MON];
		upper = pgm_read_byte(&daysInMonth[m - 1]);
		// Finally, the dreaded leap year...

		if(m == 2) {
			uint8_t y = setValues[SET_YR];
			if((y > 0) && !(y & 3)) upper = 29;
		}
	} 

	if(upper == lower) return;

	if(++setValues[curSet] > upper) setValues[curSet] = lower;

	uint8_t m, d;

	timeUpdated = setChanged = true;
}

//move to next set value and wrap if needed
inline void nextSetValue()
{
	curSet++;
	if(curSet > SET_MIN) curSet = SET_YR;
	setChanged = true;
}

//Populate setValues with the current time
inline void loadSetVals()
{

	setValues[SET_YR] = (dt_now.year() - 2000);
	setValues[SET_MON] = dt_now.month();
	setValues[SET_DAY] = dt_now.day();
	setValues[SET_HR] = dt_now.hour();
	setValues[SET_MIN] = dt_now.minute();
}

//push setValues data to the RTC
inline void adjustFromSetVal()
{
	DateTime set = DateTime(
		setValues[SET_YR] + 2000,
		setValues[SET_MON],
		setValues[SET_DAY],
		setValues[SET_HR],
		setValues[SET_MIN],
		0 //reset seconds to 0
		);

	RTC.adjust(set);
}

/*
Used for software disable of reset on Serial connection.
Setting PIND5 to an output through a 110 ohm resistor 
later will place 5V at low impedence on the reset pin, 
preventing it from reseting when RTS is pulsed on connection.
*/
inline void setResetDisable(bool state)
{
	if(state)
		DDRD |= (_BV(PIND5));
	else
		DDRD &= ~(_BV(PIND5));  
}  

//Setup all things interrupt related
inline void setInterrupts()
{
	//disable interrupt  timers
	cli();
	// Set up interrupt-on-change for buttons.
	EICRA = _BV(ISC10)  | _BV(ISC00);  // Trigger on any logic change
	EIMSK = _BV(INT1)   | _BV(INT0);   // Enable interrupts on pins
	bSave = BUTTON_STATE; // Get initial button state

	//Setup Timer2 interrupts for button handling
	//Runs at about 60Hz, which is as slow as we can go
	TCCR2A = 0;// set entire TCCR1A register to 0
	TCCR2B = 0;// same for TCCR1B
	TCNT2  = 0;//initialize counter value to 0

	// set compare match register for max
	OCR2A = 255;
	// turn on CTC mode
	TCCR2B |= _BV(WGM21);

	TCCR2B |= PRESCALE2_1024;  
	// enable timer compare interrupt
	TIMSK2 |= _BV(OCIE2A);

	//enable interrupt timers
	sei();
}
//Timer2 interrupt for handling button presses
ISR(TIMER2_COMPA_vect)
{
	// Check for button 'hold' conditions
	if(bSave != BUTTON_MASK) 
	{ // button(s) held
		if(bCount >= holdMax && !holdFlag) 
		{ //held passed 1 second
			holdFlag = true;
			bCount = 0;
			if(bSave & ~BUTTON_A)
			{        
				if(curState == STATE_MANUAL_SET)
				{
					//nothing here
				}
			}
			else if(bSave & ~BUTTON_B)
			{
				if(curState == STATE_MANUAL_SET)
				{
					setCancel = true;
				}
			}
			else //Both buttons held down!
			{
				if(curState == STATE_CLOCK)
				{
					timeReady = false;
					setCancel = false;
					curState = STATE_MANUAL_SET;
					curSet = SET_YR;
					loadSetVals();
					setChanged = true;
				}
				else if(curState == STATE_MANUAL_SET)
				{
					//set to new time
					timeReady = true;
				}
			}
		} 
		else bCount++; // else keep counting...
	} 
}

//Button external interrupts
ISR(INT0_vect) {
	uint8_t state = BUTTON_STATE;
	if(state == BUTTON_MASK) //both are high meaning they've been released
	{
		if(holdFlag)
		{
			holdFlag = false;
			bCount = 0;
		}
		else if(bCount > 3) //past debounce threshold
		{
			if(bSave & ~BUTTON_A)
			{
				//Serial.write("A");
				if(curState == STATE_MANUAL_SET)
				{
					nextSetValue();
				}
			}
			else if(bSave & ~BUTTON_B)
			{
				//Serial.write("B");
				if (curState == STATE_CLOCK)
				{
					/* //Change PWM level
					pwmLevel--;
					if (pwmLevel <= 0 || pwmLevel > pwmMax)
					pwmLevel = pwmMax;

					EEPROM.write(0, pwmLevel);
					*/
				}
				if(curState == STATE_MANUAL_SET)
				{
					incrementSetValue();
				}
			}
		}
		bCount = 0;
	}
	else if(state != bSave) {
		bCount = 0; 
	}

	bSave = state;
}

//Use the same handler for both INT0 and INT1 interrupts
ISR(INT1_vect, ISR_ALIASOF(INT0_vect));

