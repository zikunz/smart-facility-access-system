# Lab 4: IR Remote Control Texting Over an Asynchronous Serial (UART) Link

Make sure we have all of the following, to be delete later:

- [ ] Introduction on what the lab / final project is about

- [ ] Hardware and software tools used  

- [ ] High-level descriptions of goals and how we realize the goals (implementations)  

- [ ] Description of challenges and how we overcame them

- [ ] What we have learned from the lab / final project

## Introduction and Objective

In this lab, students will utilize the Saleae logic analyzer and an IR receiver module to characterize the transmissions when pressing some buttons on an AT&T IR remote control for a specific TV code (i.e., 1416). After decoding the respective waveforms generated after pressing 12 different buttons (i.e., `0`, `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`, `LAST` and `MUTE`), we are able to know which button on the remote control is pressed, before writing a program which uses interrupts to monitor the signal from the IR receiver module and use the remote control to compose messages using the [Multi-tap - Wikipedia](https://en.wikipedia.org/wiki/Multi-tap) to send text messages back and forth between two CC3200 LaunchPad boards over an asynchrous serial (UART) communcation channel. 

We have also went extra miles to enable the user to include numbers (i.e., 0 - 9) when composing text messages. In addition, the OLED screen of the sender will also show `Message sent successfully :)` after pressing `LAST` to send a message. More details can be found in `Part 6. Application Program and Innovation` below.

## Major Equipment Required

1. Two CC3200 LaunchPad boards
2. Two Adafruit OLED Breakout Board
3. AT&T S10-S3 Remote Control
4. Vishay TSOP31336 or 1236 or 31236
5. 100Ω resistor
6. 100uF capacitor

## Goals and Methods

### Part 1. IR Receiver Module Interface

We connect up the IR Receiver Module according to the Figure 1 below. We include $R_{1}$ and $C_{1}$ to help filter noise which could arise on the power source and to protect against Electrical Over-Stress (EOS) from power supply variations. Note that the IR receiver draws very little current and hence we power it up directly from a processor board. 

<p align="center">
    <img src="https://user-images.githubusercontent.com/75139323/156872986-5b975e7e-dcd4-4625-b6a7-14a5cf86a2e1.png">
 </p>

<p align="center">Figure 1. Application Circuit Using the Vishay TSOP31336 IR Receiver Module (Taken from Datasheet)</p>



## Part 2.

 



remote control. After this, we configure the remote control so that
it is set back to its factory defaults as well as the TV Device code
whose IR transmission encoding will vary. This will ensure
minimal interference from other remote interrupts in the lab area.



### Part 2. Implement OLED interface using SPI

Our GPIO configurations are as follows:

```c
void PinMuxConfig(void)
{
    //
    // Set unused pins to PIN_MODE_0 with the exception of JTAG pins 16, 17,19,20
    //
    PinModeSet(PIN_01, PIN_MODE_0);
    PinModeSet(PIN_02, PIN_MODE_0);
    PinModeSet(PIN_03, PIN_MODE_0);
    PinModeSet(PIN_04, PIN_MODE_0);
    PinModeSet(PIN_06, PIN_MODE_0);
    PinModeSet(PIN_08, PIN_MODE_0);
    PinModeSet(PIN_15, PIN_MODE_0);
    PinModeSet(PIN_21, PIN_MODE_0);
    PinModeSet(PIN_45, PIN_MODE_0);
    PinModeSet(PIN_52, PIN_MODE_0);
    PinModeSet(PIN_53, PIN_MODE_0);
    PinModeSet(PIN_55, PIN_MODE_0);
    PinModeSet(PIN_57, PIN_MODE_0);
    PinModeSet(PIN_58, PIN_MODE_0);
    PinModeSet(PIN_59, PIN_MODE_0);
    PinModeSet(PIN_60, PIN_MODE_0);
    PinModeSet(PIN_63, PIN_MODE_0);
    PinModeSet(PIN_64, PIN_MODE_0);

    //
    // Enable Peripheral Clocks 
    //
    PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

    //
    // Configure PIN_61 for GPIO Output, CS
    //
    PinTypeGPIO(PIN_61, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA0_BASE, 0x40, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_62 for GPIO Output, DC
    //
    PinTypeGPIO(PIN_62, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA0_BASE, 0x80, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_18 for GPIO Output, RESET (R)
    //
    PinTypeGPIO(PIN_18, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA3_BASE, 0x10, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_50 for SPI0 GSPI_CS
    //
    PinTypeSPI(PIN_50, PIN_MODE_9);

    //
    // Configure PIN_07 for SPI0 GSPI_MOSI
    //
    PinTypeSPI(PIN_07, PIN_MODE_7);
}
```

Our implementation of `writeCommand()` and `writeData()` is as follows:

```c
void writeCommand(unsigned char c)
{
    unsigned long ulDummy;

    // Set CS to low to start sending data -> Pin 61
    MAP_SPICSEnable(GSPI_BASE);
    GPIOPinWrite(GPIOA0_BASE, 0x40, 0x00);

    // Set DC to low to write command -> Pin 62
    GPIOPinWrite(GPIOA0_BASE, 0x80, 0x00);

    // Send the command
    MAP_SPIDataPut(GSPI_BASE, c);

    // Clean up the received register
    MAP_SPIDataGet(GSPI_BASE, &ulDummy);

    // Set CS to high to end sending data -> Pin 61
    MAP_SPICSDisable(GSPI_BASE);
    GPIOPinWrite(GPIOA0_BASE, 0x40, 0x40);
}

void writeData(unsigned char c)
{
    unsigned long ulDummy;

    // Set CS to low to start sending data -> Pin 61
    MAP_SPICSEnable(GSPI_BASE);
    GPIOPinWrite(GPIOA0_BASE, 0x40, 0x00);

    // Set DC to high to write data -> Pin 62
    GPIOPinWrite(GPIOA0_BASE, 0x80, 0x80);

    // Send the data
    MAP_SPIDataPut(GSPI_BASE, c);

    // Clean up received register
    MAP_SPIDataGet(GSPI_BASE, &ulDummy);

    // Set CS to high to end sending data -> Pin 61
    MAP_SPICSDisable(GSPI_BASE);
    GPIOPinWrite(GPIOA0_BASE, 0x40, 0x40);
}
```

Note that in both operations, we need to clean up the register although we expect no messages to be received while sending data.

Our implementation of different functions to test the OLED display is as follows: 
`testFullChar()`: Print the full character set found in the font table in `glcdfont.h`.

```c
void testFullChar()
{
    int x = 0;
    int y = 0;
    int i;

    for (i = 0; i < 1275; i++)
    {
        if (x == 0 && y == 0)
        {
            fillScreen(BLACK);
        }
        drawChar(x, y, font[i], RED, RED, 1);
        x = x + 8;
        if (x >= 128)
        {
            x = 0;
            y = y + 10;
        }
        if (y >= 128)
        {
            y = 0;
        }
    }
}
```

`testHelloworld()`: Print the string "Hello world!" to the display.

```c
void testHelloworld()
{
    drawChar(0, 64, 'H', BLUE, BLUE, 2);
    drawChar(10, 64, 'e', BLUE, BLUE, 2);
    drawChar(20, 64, 'l', BLUE, BLUE, 2);
    drawChar(30, 64, 'l', BLUE, BLUE, 2);
    drawChar(40, 64, 'o', BLUE, BLUE, 2);
    drawChar(60, 64, 'w', BLUE, BLUE, 2);
    drawChar(70, 64, 'o', BLUE, BLUE, 2);
    drawChar(80, 64, 'r', BLUE, BLUE, 2);
    drawChar(90, 64, 'l', BLUE, BLUE, 2);
    drawChar(100, 64, 'd', BLUE, BLUE, 2);
    drawChar(110, 64, '!', BLUE, BLUE, 2);
}
```

`testHorizontal()`: Display a pattern of 8 bands of different colors horizontally across the full OLED display. 

```c
void testHorizontal()
{
    fillRect(0, 0, 16, 128, BLUE);
    fillRect(16, 0, 16, 128, BLUE + 50);
    fillRect(32, 0, 16, 128, RED);
    fillRect(48, 0, 16, 128, CYAN);
    fillRect(64, 0, 16, 128, GREEN);
    fillRect(80, 0, 16, 128, YELLOW);
    fillRect(96, 0, 16, 128, WHITE);
    fillRect(112, 0, 16, 128, MAGENTA);
}
```

`testVertical()`: Display a pattern of 8 bands of different colors vertically across the full OLED display.

```c
void testVertical()
{
    fillRect(0, 0, 128, 16, BLUE);
    fillRect(0, 16, 128, 16, BLUE + 50);
    fillRect(0, 32, 128, 16, RED);
    fillRect(0, 48, 128, 16, CYAN);
    fillRect(0, 64, 128, 16, GREEN);
    fillRect(0, 80, 128, 16, YELLOW);
    fillRect(0, 96, 128, 16, WHITE);
    fillRect(0, 112, 128, 16, MAGENTA);
}
```

### Part 3. Verify SPI waveforms using a Saleae logic

We verified SPI waveforms using a Saleae logic which can detect the output signals of the OLED screen while receiving data from the CC3200 LaunchPad. We observed that the output signals correspond to the code written for `writeData()`and`writeCommand()`.

### Part 4. Use I²C to communicate with the on-board BMA222 accelerometer

We imported `i2c_demo` project and read data from the registers (0x03) acc_x, (0x05) acc_y in I²C device address 0x18. We made use of `readreg` and confirmed that the acceleration data changes given different tilt angles.

### Part 5. Verify I²C waveforms using a Saleae logic

Again, we verified I²C waveforms using a Saleae logic. We observed that `SCLK` alternates between `HIGH` and `LOW`, the Bosch accelerometer consistently sends x-axis and y-axis readings to the CC3200 LaunchPad.

### Part 6. Application Program and Innovation

In our application, the user is able to move a small ball (with a radius of 4 pixels) around on the OLED display by tilting the board in x and y axes. The steeper the angle, the faster the ball will move across the OLED display. In the event that the ball reaches the edge of the OLED display, it will be "bounded" and stay at the edge of the OLED display. Note that when the board is flat, the ball does not move at all. Also, the ball is able to move in any direction in the 2D dimension given.

We have also designed a game out of it - there will be a stationary red apple (in the form of a circular icon) and a moving yellow apple (also in the form of a circular icon) performing a [random walk](https://en.wikipedia.org/wiki/Random_walk) on the OLED display. 

1. If the ball touches the red apple, the ball will increase in size and be able to move faster (note that given the bigger size, the increase in speed might not be obvious to the user). The red apple appears again at a pseudo-random position and the user can control the ball to hit it again. The user will score a point every time the ball touches the red apple and this is represented by "game stage" which will be shown at the end of the game.

2. In the event that the ball collides with the yellow apple, the game stops and "GAME OVER" in blue is displayed on the OLED screen. The console window will also print the game stage (i.e., score) reached by the user.

3. If the user manages to hit the red apple using the ball for ten consecutive times while avoiding any collisions with the yellow apple, the user wins and the game also stops and "GAME OVER" in red is displayed on the OLED screen. The console window will show congratulatory messages of achievement and  print the game stage of 10 reached by the user as well.

Note that the yellow apple is not bounded and it is free to move outside the OLED screen.

We made use of the `pin_mux_config.c` from our spi program and added in these two lines below:

```c
// Configure PIN_01 for I2C0 I2C_SCL
PinTypeI2C(PIN_01, PIN_MODE_1);

// Configure PIN_02 for I2C0 I2C_SDA
PinTypeI2C(PIN_02, PIN_MODE_1);
```

The main application logic is as follows:

```c
int ball_x_position = OLED_MAXIMUM_LENGTH / 2;
int ball_y_position = OLED_MAXIMUM_LENGTH / 2;
int ball_x_offset_position;
int ball_y_offset_position;
int red_apple_x_position = rand() % OLED_MAXIMUM_LENGTH; // If we want a number to be between 0 to 124 (inclusive), we need to % a random number by 125
int red_apple_y_position = rand() % OLED_MAXIMUM_LENGTH; 
int yellow_apple_x_position = rand() % OLED_MAXIMUM_LENGTH - 1; // If we want a number to be between 0 to 123 (inclusive), we need to % a random number by 124
int yellow_apple_y_position = rand() % OLED_MAXIMUM_LENGTH - 1; 
int game_stage = 0; // When game_stage increases, the speed of the ball increases proportionally

int ball_current_size = BALL_STARTING_SIZE; // Starting ball size is 4 pixels

while (1) {
    char x_buffer[256] = "readreg 0x18 0x5 1 \n\r";
    char y_buffer[256] = "readreg 0x18 0x3 1 \n\r";

    if (abs(ball_x_position - red_apple_x_position) <= ball_current_size &&
        abs(ball_y_position - red_apple_y_position) <=
        ball_current_size) // If the ball touches the red apple
    {
        fillCircle(red_apple_x_position, red_apple_y_position, 2,
            GAME_BACKGROUND_COLOR); // Make the red apple disappear by filling it black
        red_apple_x_position = rand() % OLED_MAXIMUM_LENGTH; // Compute a random x coordinate
        red_apple_y_position = rand() % OLED_MAXIMUM_LENGTH; // Compute a random y coordinate
        ball_current_size += BALL_INCREMENTAL_SIZE; // The size of the ball increases by 2 pixels
        game_stage++;
    }

    if (abs(ball_x_position - yellow_apple_x_position) <= ball_current_size &&
        abs(ball_y_position - yellow_apple_y_position) <=
        ball_current_size) // If the ball touches the yellow apple
    {
        fillScreen(BLACK);
        drawChar(0, 64, 'G', BLUE, BLUE, 2);
        drawChar(15, 64, 'A', BLUE, BLUE, 2);
        drawChar(30, 64, 'M', BLUE, BLUE, 2);
        drawChar(45, 64, 'E', BLUE, BLUE, 2);
        drawChar(70, 64, 'O', BLUE, BLUE, 2);
        drawChar(85, 64, 'V', BLUE, BLUE, 2);
        drawChar(100, 64, 'E', BLUE, BLUE, 2);
        drawChar(115, 64, 'R', BLUE, BLUE, 2);
        printf("GAME OVER! Game stage reached is %d :)", game_stage);
        break;
    }

    fillCircle(red_apple_x_position, red_apple_y_position, RED_BALL_SIZE,
        RED_APPLE_COLOR); // Let a red apple appear at a pseudo-random 2D spot if it is executed for the first time, otherwise, make a new red apple appear after the ball touches it
    fillCircle(yellow_apple_x_position, yellow_apple_y_position,
        YELLOW_BALL_SIZE,
        YELLOW_APPLE_COLOR);

    // Yellow ball is performing a random walk
    int random_boolean = rand() & 1;
    if (random_boolean) {
        yellow_apple_x_position += 1;
        fillCircle(yellow_apple_x_position - 1, yellow_apple_y_position,
            YELLOW_BALL_SIZE,
            GAME_BACKGROUND_COLOR);
    } else {
        yellow_apple_x_position -= 1;
        fillCircle(yellow_apple_x_position + 1, yellow_apple_y_position,
            YELLOW_BALL_SIZE,
            GAME_BACKGROUND_COLOR);
    }

    random_boolean = rand() & 1;
    if (random_boolean) {
        yellow_apple_y_position += 1;
        fillCircle(yellow_apple_x_position, yellow_apple_y_position - 1,
            YELLOW_BALL_SIZE,
            GAME_BACKGROUND_COLOR);
    } else {
        yellow_apple_y_position -= 1;
        fillCircle(yellow_apple_x_position, yellow_apple_y_position + 1,
            YELLOW_BALL_SIZE,
            GAME_BACKGROUND_COLOR);
    }

    // Find the offset position of x and y needed and decrease it by 10 times and increase it proportional according to its current size
    ball_x_offset_position = ParseNProcessCmd(x_buffer) /
        BALL_SPEED_SCALING_FACTOR *
        pow(SPEED_INCREMENTAL_COEFFICIENT, game_stage);
    ball_y_offset_position = ParseNProcessCmd(y_buffer) /
        BALL_SPEED_SCALING_FACTOR *
        pow(SPEED_INCREMENTAL_COEFFICIENT, game_stage);

    // Increase the magnitude of the "velocity" by subtracting offset_position required needed in both dimensions according to the current tilt angle, we use "-" here because of the positioning of our OLED on the breadboard
    ball_x_position -= ball_x_offset_position;
    ball_y_position -= ball_y_offset_position;

    // Bound the ball position, note that the yellow ball is not bounded
    if (ball_x_position >= (OLED_MAXIMUM_LENGTH - ball_current_size)) {
        ball_x_position = OLED_MAXIMUM_LENGTH - ball_current_size;
    } else if (ball_x_position <= ball_current_size) {
        ball_x_position = ball_current_size;
    }

    if (ball_y_position >= (OLED_MAXIMUM_LENGTH - ball_current_size)) {
        ball_y_position = OLED_MAXIMUM_LENGTH - ball_current_size;
    } else if (ball_y_position <= ball_current_size) {
        ball_y_position = ball_current_size;
    }

    // Whenever the ball moves, its current location should be filled with the ball color
    fillCircle(ball_x_position, ball_y_position, ball_current_size,
        BALL_COLOR);

    fillCircle(yellow_apple_x_position, yellow_apple_y_position,
        YELLOW_BALL_SIZE,
        YELLOW_APPLE_COLOR);

    // Whenever the ball moves, its previous location should be filled with the background color
    fillCircle(ball_x_position, ball_y_position, ball_current_size,
        GAME_BACKGROUND_COLOR);

    if (game_stage == 10) {
        fillScreen(BLACK);
        drawChar(0, 64, 'G', RED, RED, 2);
        drawChar(15, 64, 'A', RED, RED, 2);
        drawChar(30, 64, 'M', RED, RED, 2);
        drawChar(45, 64, 'E', RED, RED, 2);
        drawChar(70, 64, 'O', RED, RED, 2);
        drawChar(85, 64, 'V', RED, RED, 2);
        drawChar(100, 64, 'E', RED, RED, 2);
        drawChar(115, 64, 'R', RED, RED, 2);
        printf("YOU WON! Game stage reached is %d :D", game_stage);
        break;
    }
}
```

Note that the sizes of the red apple and yellow apple are 2 pixels and 3 pixels respectively. Hence, the pseudo-random x and y positions of the red apple and yellow apple can only take the ranges of `0 ~ OLED_MAXIMUM - 2 / 2` and `0 ~ OLED_MAXIMUM - 3 / 2`. This is to avoid the situation of having a red / yellow ball appearing both at the top and bottom / left and right on the OLED screen.

### Video Demo

Please refer to `Lab 3 SPI Demo` [here](https://youtu.be/vKJY5e-6vfY) and `Lab 3 I2C Demo` [here](https://youtu.be/vveH6eGk6_8).

## Challenges

During our development procedure, we have met some difficulties in function implementation and testing.

1. The SPI request and response functions should be precisely ordered in the main program to avoid possible malfunctions due to the conflict.
2. Saleae analyzer has a picky environment requirement. In our case, the CC3200 board and the analyzer cannot be connected and powered by the same computer. If this happens, the Logic software will show errors of connection.
3. The refreshing rate of the LCD is too low that the previous frame will remain on the screen. To solve this, one possible solution is to increase the SPI and I²C communication rate.
4. The ball movement in the last section is not natural. In future improvement the acceleration factor could be involved to simulate the real physical environment with inertia.

## Conclusion

In this lab, we successfully finished all tasks listed in the lab manual and verified our design with our TA. We have learnt the basics of SPI and I²C and tried to write the functions in C to implement the communication between two MCUs and then with the OLED screen. Besides that, we also learned how to use modern analyzing tools to detect and test the communicating signals. In the last part of the lab, we designed our own game with the given prompt. With our modification and innovation the game is playable and well-practiced with our skill in SPI/I²C programming with MCU.