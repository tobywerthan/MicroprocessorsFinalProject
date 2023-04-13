#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "inc/hw_gpio.h"

// pin definitions
#define RED GPIO_PIN_1
#define BLUE GPIO_PIN_2
#define GREEN GPIO_PIN_3
#define SW1 GPIO_PIN_4
#define SW2 GPIO_PIN_0

// variable definitions
volatile char isr_flag_1 = 0;
volatile char isr_flag_2 = 0;
int i, k , j;
unsigned char patternOne[20] = {0};
unsigned char userPattern[20] = {0};
bool patternActive = true;
bool selectingMode = true;
int mode = 0;
int finalMode = 0;
int patternSize = 1;
int random = 0;
bool userInput = true;
bool winner = false;
int correctCount = 0;
int delaySize = 20000000;

// IRQ Handler
void GPIO_IRQHandler(void){
    if(GPIOIntStatus(GPIO_PORTF_BASE, 1) == 16){
        // Clear Flag
        GPIOIntClear(GPIO_PORTF_BASE, SW1);
        for(i = 0; i < 5000; i++){
            // Blank for delay
        }
        // Handler Code
        if(GPIOPinRead(GPIO_PORTF_BASE, SW1) != 0){
            isr_flag_1 = 1;
        }
    }
    if(GPIOIntStatus(GPIO_PORTF_BASE, 1) == 1){
        // Clear Flag
        GPIOIntClear(GPIO_PORTF_BASE, SW2);
        for(i = 0; i < 5000; i++){
            // Blank for delay
        }
        // Handler Code
        if(GPIOPinRead(GPIO_PORTF_BASE, SW2) != 0){
            isr_flag_2 = 1;
        }
    }
}

// Configures LED and switch GPIO and Clock
void configure_LEDs(void){
    // Enable Port F as GPIO
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    // Set PF(1, 2, 3) to outputs.
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED | GREEN | BLUE);

    GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00); // turn off all leds

    // Critical Function GPIO Protection
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0x4C4F434B;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

    //Initialize Button 1 & 2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, SW1);

    // enable F4's pullup
    GPIOPadConfigSet(GPIO_PORTF_BASE, SW1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, SW2);
    GPIOPadConfigSet(GPIO_PORTF_BASE, SW1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

// Configures interrupts
void configure_PF_Int(){
    // Button 1
    GPIOIntTypeSet(GPIO_PORTF_BASE, SW1, GPIO_RISING_EDGE);
    GPIOIntRegister(GPIO_PORTF_BASE, GPIO_IRQHandler);
    GPIOIntEnable(GPIO_PORTF_BASE, SW1);

    // Button 2
    GPIOIntTypeSet(GPIO_PORTF_BASE, SW2, GPIO_RISING_EDGE);
    GPIOIntRegister(GPIO_PORTF_BASE, GPIO_IRQHandler);
    GPIOIntEnable(GPIO_PORTF_BASE, SW2);

    // enables interrupts
    IntEnable(INT_GPIOF);
    IntMasterEnable();
}

void toggleMode(){ // toggles the mode based on the switch 1 interrupt
    if(isr_flag_1){ // will execute if the interrupt for the first switch occurs
        isr_flag_1 = 0; // sets the flag back to zero
        if(mode < 2){ // will execute if the mode is less than two
            mode++; // increment the mode
        } else { // if mode is greater than 2 reset the mode
            mode = 0; // set the mode back to zero
        }
    }
}

void selectMode(){ // sets the finalMode value based on switch 2 interrupt
    if(mode == 0){ // will execute if mode is zero
        GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00); // turn off all leds
        GPIOPinWrite(GPIO_PORTF_BASE, GREEN, 0b00001000); // turn on green led
        if(isr_flag_2){ // will execute if the interrupt for the second switch occurs
            isr_flag_2 = 0; // sets the flag back to zero
            finalMode = 0; // sets the final mode to zero
            GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00); // turn off all leds
            SysCtlDelay(20000000); // delay
            GPIOPinWrite(GPIO_PORTF_BASE, GREEN, 0b00001000); // turn on green led
            selectingMode = false; // exit selecting mode
        }
    } else if (mode == 1){ // will execute if the mode is one
        GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00); // turn off all leds
        GPIOPinWrite(GPIO_PORTF_BASE, BLUE, 0b00000100); // turn on blue led
        if(isr_flag_2){ // will execute if the interrupt for the second switch occurs
            isr_flag_2 = 0; // set the flag back to zero
            finalMode = 1; // set the finalMode to one
            GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00); // turn off all leds
            SysCtlDelay(20000000); // delay
            GPIOPinWrite(GPIO_PORTF_BASE, RED, 0b00000100); // turn on blue led
            selectingMode = false; // exit selecting mode
        }
    } else { // will execute if the mode is 2
        GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00); // turn off all leds
        GPIOPinWrite(GPIO_PORTF_BASE, RED, 0b00000010); // turn on red led
        if(isr_flag_2){ // will execute if the interrupt for the second switch occurs
            isr_flag_2 = 0; // set the flag back to zero
            finalMode = 2; // set the final mode to two
            GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00); // turn off all leds
            SysCtlDelay(20000000); // delay
            GPIOPinWrite(GPIO_PORTF_BASE, RED, 0b00000010); // turn on red led
            selectingMode = false; // exit selecting mode
        }
    }
}

void determineSize(){ // determines the pattern length based on the mode
    if( mode == 0 ){ // will execute if the mode is zero
        patternSize = 5; // sets the patternSize to 5
    } else if ( mode == 1 ){ // will execute if the mode is one
        patternSize = 10; // sets the patternSize to 10
    } else { // will execute if the mode is two
        patternSize = 20; // sets the patternSize to 20
    }
}

void generatePattern(unsigned char *patt, int size){ // generates a random pattern that consists of binary values 1 or 2
    for(j = 0; j < size; j++){ // iterates over a passed interval
        random = rand() % 101; // generates a random value from 0 to 100
        if(random > 50){ // will execute if the random value is greater than 50
           patt[j] = 1; // sets the pattern array at the current index to one
        } else { // will execute if the random value is less than 50
           patt[j] = 2; // sets the pattern array the the current index to 2
        }
    }
}

void displayPattern(){ // lights the leds corresponding the generated pattern
    if( mode == 0 ){ // will execute if the mode is zero
        delaySize = 20000000; // sets the delay to 20000000
    } else if ( mode == 1 ){ // will execute if the mode is 1
        delaySize = 15000000; // sets the delay to 15000000
    } else { // will execute if the mode is 2
        delaySize = 8000000; // sets the delay to 8000000
    }

    // flashes white twice indicating the pattern is about to be displayed
    GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00);  // turn off all leds
    GPIOPinWrite(GPIO_PORTF_BASE, RED | BLUE | GREEN, 0b00001110); // white
    SysCtlDelay(7000000); // delay
    GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00);  // turn off all leds
    SysCtlDelay(7000000); // delay
    GPIOPinWrite(GPIO_PORTF_BASE, RED | BLUE | GREEN, 0b00001110); // white
    SysCtlDelay(7000000); // delay
    GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00);  // turn off all leds
    SysCtlDelay(20000000); // delay

    while(patternActive){ // will execute if patternActive is true
        for(i = 0; i < patternSize; i++){ // iterates over the length of the pattern
            SysCtlDelay(delaySize); // delays based on the mode
            if(patternOne[i] == 1){ // will execute if the current pattern element is one
                GPIOPinWrite(GPIO_PORTF_BASE, RED | BLUE, 0b00000110); // purple
            } else if (patternOne[i] == 2){ // will execute if the current pattern element is two
                GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN, 0b00001010); // yellow
            } else { // will execute if the current pattern element is not one or two
                patternActive = false; // ends the loop
            }
            SysCtlDelay(delaySize); // delays based on the mode
            GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00);  // turn off all leds
        }
        patternActive = false; // ends the loop
    }
}

void generateUserPattern(unsigned char *userPatt, int size){ // generates a pattern based on the user's input
    int index = 0; // initializes an index variable
    while(userInput){ // will execute if the userInput is true
        if(index < size){ // will execute if the index is less than the passed size
            if(isr_flag_1){ // will execute if the interrupt for the first switch occurs
                isr_flag_1 = 0; // sets the flag back to zero
                userPatt[index] = 1; // sets the current pattern element to one
                index++; // increments the index variable
            }
            if(isr_flag_2){ // will execute if the index is less than the passed size
                isr_flag_2 = 0; // set the interrupt back to zero
                userPatt[index] = 2; // set the current pattern element to two
                index++; // increments the index variable
            }
        } else { // will execute if the index is less than the passed size
            userInput = false; // sets the userInput to false
        }
    }
}

void checkPattern(){ // determines if the user input matches the randomly generated pattern
    for(k = 0; k < patternSize; k++){ // loops through the size of the pattern determined by the mode
        if(patternOne[k] == userPattern[k]){ // will execute if the current index of the pattrn and the user input is the same value
            correctCount++; // increments the correctCount
        }
    }
    if(correctCount == patternSize){ // will execute if the correctCount is the same as the patternSize indicating that the user input the correct pattern
        winner = true; // sets winner to true;
    }
}

void findMode(){ // loop for selecting the mode
    while(selectingMode){ // will execute if the selectingMode is true
        toggleMode(); // calls toggleMode
        selectMode(); // calls selectMode
    }
    GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00);  // turn off all leds
}

void gameOver(){ // End game sequence
    GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00); // turn off all leds
    if(winner){ // will execute if winner is true
        while(1){ // infinite loop
            GPIOPinWrite(GPIO_PORTF_BASE, GREEN, 0b00001000); // turn on green led
            SysCtlDelay(8000000); // delay
            GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00);  // turn off all leds
            SysCtlDelay(8000000); // delay
        }
    } else { // will execute if winner is false
        while(1){ // infinite loop
            GPIOPinWrite(GPIO_PORTF_BASE, RED, 0b00000010); // turn on red led
            SysCtlDelay(8000000); // delay
            GPIOPinWrite(GPIO_PORTF_BASE, RED | GREEN | BLUE, 0x00); // turn off all leds
            SysCtlDelay(8000000); // delay
        }
    }
}

int main(void){ // main
    time_t t; // creates time variable for random value
    srand((unsigned) time(&t)); // srand initialization
    configure_LEDs(); // configures GPIO pin for leds
    configure_PF_Int(); // confiigure interrupts and handlers
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); // sets the clock
    findMode(); // calls findMode
    determineSize(); // calls determineSize
    generatePattern(patternOne, patternSize); // calls generateSize and passes patternOnr and patternSize
    displayPattern(); // calls displayPattern
    generateUserPattern(userPattern, patternSize); // calls generateUserPattern and passes userPattern and patternSize
    checkPattern(); // calls checkPattern
    gameOver(); // calls gameOver
}
