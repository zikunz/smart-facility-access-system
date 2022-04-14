# Lab 1: Development Tools Tutorial and Lab Exercise

## Application Overview

1. When the program starts, it displays a message on the console window with a header and usage instructions.
2. The program polls SW3 and SW2 on the CC3200 LaunchPad. When SW3 is pressed, a binary counting sequence on the LEDs starts. It will count from 000 to 111 continuously (after 111, it will restart from 000 unless SW2 is pressed). A message to the console "SW3 pressed" will also be shown. This message will not be printed again regardless of how many times the user keeps on pressing SW3 unless after SW2 is pressed.
* 000 means green LED is OFF, orange LED is OFF, red LED is OFF
* 001 means green LED is OFF, orange LED is OFF, red LED is ON
* 010 means green LED is OFF, orange LED is ON, red LED is OFF
* 011 means green LED is OFF, orange LED is ON, red LED is ON
* 100 means green LED is ON, orange LED is OFF, red LED is OFF
* 101 means green LED is ON, orange LED is OFF, red LED is ON
* 110 means green LED is ON, orange LED is ON, red LED is OFF
* 111 means green LED is ON, orange LED is ON, red LED is ON
3. When SW2 is pressed, the program will blink the LEDs ON and OFF in unison. Similarly, the program will print "SW2 pressed" to the console. This message will not be printed again regardless of how many times the user keeps on pressing SW2 unless after SW3 is pressed.
4. The output signal P18 is set high whenever SW2 is pressed and low whenever SW3 is pressed. This can be verified using an oscilloscope.

## Screenshot of the Console Window

<p align="center">
    <img src="https://user-images.githubusercontent.com/75139323/154020078-99edfd26-6051-4c25-a19c-dd20e1f71ba4.png">
 </p>

## Implementations

In `main()`, I include `InitTerm()` and `ClearTerm()` which are already implemented in `uart_if.c` that initializes and clears the terminal respectively. After that, I modify the user-defined `DisplayBanner()` in `uart_demo` to print our application name on the console. Lastly, I call `Message()` to print the usage instructions for the user.

After that, I declare two local variables `isSwitchTwoPressed` and `isSwitchThreePressed` respectively and initialize both of them to 0 (1 means a particular switch is pressed). Then, it enters a forever loop consisting of an `if` and `else if` statements:

- In the `if` statement:
  - I check if SW3 is pressed for the first time or after SW2 is pressed, only if this happens, the LEDs will need to perform the binary sequence from 000 to 111. This is checked using `if (GPIOPinRead(GPIOA1_BASE, 0x20) && isSwitchThreePressed == 0)` where `GPIORead(unsigned long ulPort, unsigned char ucPins)` takes in two parameters (where `ulPort` is the base address of the GPIO port and `ucPins` is the bit-packed representation of the pin(s)) and reads the values present in the specified pins. 
  - If the `if` statement evaluates to true, `Report()` needs to be called to print "SW3 pressed" on the console. `GPIOPinWrite()` also needs to be called to write a LOW to P18.
  - Now, I can set `isSwitchThreePressed` to be 1 because I only want the message of "SW3 pressed" to be printed only once after pressing SW3 multiple times. I also need to set `isSwitchTwoPressed` to be 0 here becuase the `else-if` has a similar logic later on. Here, I delcare a local variable called `flag` and set it to 0.
  - Finally, I have another forever loop and I iterate from 0 to `MAX_BINARY_SEQUENCE` (not inclusive) which is 7 + 1 = 8. I call a user-defined function `LEDBinaryCounting(int)` which takes in `i` and lights up the correct LED(s) with appropriate amount of delay time using `GPIO_IF_LEDOn()`, `GPIO_IF_LEDOff()` and `MAP_UtilsDelay()`. After any one iteration, I want to check if SW2 is pressed, this is done using `if (GPIOPinRead(GPIOA2_BASE, 0x40))`. If this happens, I will set `flag` to be 1 because I want to break the forever loop. There are two `break` statements becuase I have a `for` loop nesting in a `while` loop here.
- In the `else-if` statement:
  - Similarly, I check if SW2 is pressed for the first time or after SW3 is pressed, only if this happens, the LEDs will need to blink ON and OFF in unison. This is checked using `else if (GPIOPinRead(GPIOA2_BASE, 0x40) && isSwitchTwoPressed == 0)`.
  - Likewise, if the `else-if` statement evaluates to true, `Report()` needs to be called to print "SW2 pressed" on the console. `GPIOPinWrite()` also needs to be called to write a HIGH to P18.  
  - Now, I can set `isSwitchTwoPressed` to be 1 because I only want the message of "SW2 pressed" to be printed only once after pressing SW2 multiple times. I also need to set `isSwitchThreePressed` to be 0 here becuase the `if` has a similar logic previously.
  - Finally, I have another forever loop and I call a user-defined function `LEDLinkUnison()` lights up all the LEDs, delay for appropriate amount of time, turns off all the LEDs and again delay for appropriate amount of time. After any complete blink, I want to check if SW3 is pressed, this is done using `GPIOPinRead(GPIOA1_BASE, 0x20)`. If this happens, I want to break the forever loop.

## Known Limitations

As per the requirements, the polling approach is used to switch the LED mode from one to the other by pressing the other corresponding switch. You might notice that the switch is not entirely responsive as it is the limitation of polling. The user has to press SW3 and SW2 at the right time as they will only be checked after a part of the complete binary sequence (there are 8 parts from 000 to 111) and the LEDs blink once respectively. 

To overcome this issue, we could probably rely on some built-in mechanisms. However, upon checking the datasheet, there does not seem to have an internal register which stores a value of 1 if a particular switch is pressed, and after checking if it is pressed, its stored value resets to 0. 

Other possible solutions include using an Interrupt Service Routine (ISR) or Real-Time Operating System (RTOS) to allow multiple threads.
