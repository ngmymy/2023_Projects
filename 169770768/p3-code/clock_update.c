#include <stdio.h>
#include "clock.h"

// Reads the time of day from the TIME_OF_DAY_PORT global variable. If
// the port's value is invalid (negative or larger than 16 times the
// number of seconds in a day) does nothing to tod and returns 1 to
// indicate an error. Otherwise, this function uses the port value to
// calculate the number of seconds from start of day (port value is
// 16*number of seconds from midnight). Rounds seconds up if there at
// least 8/16 have passed. Uses shifts and masks for this calculation
// to be efficient. Then uses division on the seconds ith 1 for AM and
// 2 for PM. By the end, all fields of the `tod` struct are filled in
// and 0 is returned for success.
 // 
// CONSTRAINT: Uses only integer operations. No floating point
// operations are used as the target machine does not have a FPU.
// 
// CONSTRAINT: Limit the complexity of code as much as possible. Do
// not use deeply nested conditional structures. Seek to make the code
// as short, and simple as possible. Code longer than 40 lines may be
// penalized for complexity.
int set_tod_from_ports(tod_t *tod){
    if(TIME_OF_DAY_PORT < 0 || TIME_OF_DAY_PORT > (16 * 86400)){ // if <0 or ># of secs in day then error
        return 1;
    }else{
        tod->day_secs = TIME_OF_DAY_PORT >> 4;
        if((TIME_OF_DAY_PORT & 0b1111) >= 8){
            tod->day_secs += 1;
        }
        int x = tod->day_secs;
        tod->time_secs = x % 60; // sets time_secs
        x /= 60;
        tod->time_mins = x % 60; // sets time_mins
        x /= 60;
        tod->time_hours = x; // sets time_hours

        if(tod->time_hours < 12) // condition to set ampm
            tod->ampm = 1;
        else
            tod->ampm = 2;

        tod->time_hours %= 12; // set time_hours

        if(tod->time_hours == 0){ // special case for if hours == 0
            tod->time_hours = 12;
        }
    }
    return 0;
}

// Accepts a tod and alters the bits in the int pointed at by display
// to reflect how the LCD clock should appear. If any time_** fields
// of tod are negative or too large (e.g. bigger than 12 for hours,
// bigger than 59 for min/sec) or if the AM/PM is not 1 or 2, no
// change is made to display and 1 is returned to indicate an
// error. The display pattern is constructed via shifting bit patterns
// representing digits and using logical operations to combine them.
// May make use of an array of bit masks corresponding to the pattern
// for each digit of the clock to make the task easier.  Returns 0 to
// indicate success. This function DOES NOT modify any global
// variables
// 
// CONSTRAINT: Limit the complexity of code as much as possible. Do
// not use deeply nested conditional structures. Seek to make the code
// as short, and simple as possible. Code longer than 85 lines may be
// penalized for complexity.

int set_display_from_tod(tod_t tod, int *display){
    if(tod.time_secs < 0 || tod.time_secs > 59 || tod.time_mins < 0 || tod.time_mins > 59 || tod.time_hours < 0 || tod.time_hours > 12 || (tod.ampm != 1 && tod.ampm != 2)){
        return 1;
    }
    int display_bits[10] = {0b1110111, 0b0100100, 0b1011101, 0b1101101, 0b0101110, 0b1101011, 0b1111011, 0b0100101, 0b1111111, 0b1101111};
    int x = 0; // start of bit string
    int min_ones = tod.time_mins % 10; // % = keep rightmost digit
    int min_tens = tod.time_mins / 10; // / = keep leftmost digit
    int hour_ones = tod.time_hours % 10;

    x = x | (display_bits[min_ones]);           // 00:0x
    x = x | (display_bits[min_tens] << 7);      // 00:x0

    x = x | (display_bits[hour_ones] << 14);    // 0x:00
    if(tod.time_hours > 9){
        x = x | (display_bits[1] << 21);        // x0:00
    }

    if(tod.ampm == 1){
        x = x | (0b1 << 28); // am
    }else{
        x = x | (0b1 << 29); // pm
    }
    *display = x;
    return 0;
}

// Examines the TIME_OF_DAY_PORT global variable to determine hour,
// minute, and am/pm.  Sets the global variable CLOCK_DISPLAY_PORT bits
// to show the proper time.  If TIME_OF_DAY_PORT appears to be in error
// (to large/small) makes no change to CLOCK_DISPLAY_PORT and returns 1
// to indicate an error. Otherwise returns 0 to indicate success.
//
// Makes use of the previous two functions: set_tod_from_ports() and
// set_display_from_tod().
// 
// CONSTRAINT: Does not allocate any heap memory as malloc() is NOT
// available on the target microcontroller.  Uses stack and global
// memory only.
int clock_update(){
    tod_t tod;
    int set_from_ports = set_tod_from_ports(&tod); // runs set_tod_from ports, returns 1 if fails, 0 passes
    if(set_from_ports){ // if fails (1) == true
        return 1;
    }else{
        set_display_from_tod(tod, &CLOCK_DISPLAY_PORT);
    }
    return 0;
}