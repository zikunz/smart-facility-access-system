// Application Name: Lab 1
// Application Overview:
// 1. When the program starts, it should display a message on the console window with a header and usage instructions.
// 2. The program should poll the SW3 and SW2 switches on the CC3200 LaunchPad. When SW3 is
// pressed, a binary counting sequence on the LEDs starts (which counts from 000 �C 111
// continuously on the three LEDs). A message to the console "SW3 pressed" will also be shown. This message
// should not be printed again until after SW2 has been pressed. After 111, it will restart from 000 unless SW2 is pressed.
// When SW2 is pressed, the program should blink the LEDs ON and OFF in unison. Similarly, the program will print
" to the console. This message should not be printed again until after SW3 has been pressed.
// 3. The output signal P18 is set high whenever SW2 is pressed and low whenever SW3 is pressed. This can be verified
// using an oscilloscope.