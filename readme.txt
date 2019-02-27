Clock Faces and DIP switch settings:

000 - forwardClock - Shows the current time and date for 4 seconds, then plays "Conway's Game of Life" with the lit LEDs for the remainder of the minute.

001 - staticClock - Starts with completely random static, and then resolves to mostly clear numbers showing the date and time.  The "correctly" lit pixels still have a chance to turn off and vice versa, so it is never perfectly displayed. Requires some motion sensing ability to read the time.

010 - meltClock - Displays the current time and date, and then randomly makes lit pixels start falling to the bottom of the display. After about 15-20 seconds, becomes unreadable.

011 - brightClock - Each fully lit column is one hour in the day, starting from the left edge. Partially lit lines represent the portion of the current hour, with 3.75 minute resolution.

100 - backwardClock - Plays the Game of Life... but in reverse!  It iterates through the game about 30 steps, displays the output, then goes back to the start and iterates 29 times, shows the output, etc... until it's back to 0 steps, and then it just shows the correct time for a few seconds, until it starts the process over.  As the minute progresses, the display speeds up since it takes a non-trivial amount of time to iterate 30 times through the Game (roughly 3.1 seconds for 30 iterations, and .1 seconds for a single iteration.) 

101 - chartClock - Displays a bar graph that shows the approximate fraction of 5 different time durations, from top to bottom year, month, day, hour, and minute.  The minute hand has a 2.5 second resolution... Everything above that is way worse.

110 - binaryClock - Displays the current hours, minutes, and seconds, in binary format. Each row of boxes is one segment of the clock, and the filled boxes represent "on" bits of the time.

111 - epochClock - Displays the decimal notation of the current epoch time. *NOTE* it does not account for timezone differences. Not for use for mission critical timekeeping, because unless you live in GMT-0, it will be off.
ToDo: add in some sort of time zone functionality while setting the time. Of course, that would only be useful for this clock, so very low priority. 

Setting the clock: Hold both buttons until "Year 13" appears. Use the left hand button (when looking at the display) to advance the number, and the right hand button to change the set mode.  Hold down the right button to cancel out, and both buttons to save the new time and return to the clock function.  Note: In some clock faces, you may hold both buttons for more than a second, but the display won't change to the set mode until the start of the next minute.

Setting brightness: DIP switch #4 controls the brightness. Off is dim, on is bright.  The clock checks this setting at the top of each minute for most clocks, so it could take up to a minute for a change to take effect. (cycling power to the clock will force an update, of course)

 