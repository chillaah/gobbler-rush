/* ======= GOBBLER RUSH ======= */

/*
   CHIRAN GAYANGA WALISUNDARA
   N10454012
   CAB202 MICRO-CONTROLLER ASSIGNMENT
*/

// c libraries required
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

//define all global variables (By Prof. Luis Mejias Alvarez)
#define SET_BIT(reg, pin)			      (reg) |= (1 << (pin))
#define CLEAR_BIT(reg, pin)			    (reg) &= ~(1 << (pin))
#define WRITE_BIT(reg, pin, value)	(reg) = (((reg) & ~(1 << (pin))) | ((value) << (pin)))
#define BIT_VALUE(reg, pin)			    (((reg) >> (pin)) & 1)
#define BIT_IS_SET(reg, pin)	    	(BIT_VALUE((reg),(pin))==1)
#define FREQ     (16000000.0)
#define PRESCALE (1024.0)

//definitions extracted from topic 08 - "Serial Communications" lecture notes
#define F_CPU    (16000000UL)
#define BAUD     (9600)
#define MYUBRR   (F_CPU/16/BAUD-1)

//definitions extracted from topic 11 - "The LCD Screen" lecture notes
#define LCD_USING_4PIN_MODE (1)

#define LCD_DATA4_DDR (DDRD)
#define LCD_DATA5_DDR (DDRD)
#define LCD_DATA6_DDR (DDRD)
#define LCD_DATA7_DDR (DDRD)

#define LCD_DATA4_PORT (PORTD)
#define LCD_DATA5_PORT (PORTD)
#define LCD_DATA6_PORT (PORTD)
#define LCD_DATA7_PORT (PORTD)

#define LCD_DATA4_PIN (4)
#define LCD_DATA5_PIN (5)
#define LCD_DATA6_PIN (6)
#define LCD_DATA7_PIN (7)

#define LCD_RS_DDR     (DDRB)
#define LCD_ENABLE_DDR (DDRB)

#define LCD_RS_PORT     (PORTB)
#define LCD_ENABLE_PORT (PORTB)

#define LCD_RS_PIN     (1)
#define LCD_ENABLE_PIN (0)

//DATASHEET: https://s3-us-west-1.amazonaws.com/123d-circuits-datasheets/uploads%2F1431564901240-mni4g6oo875bfbt9-6492779e35179defaf4482c7ac4f9915%2FLCD-WH1602B-TMI.pdf

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE  0x00
#define LCD_MOVERIGHT   0x04
#define LCD_MOVELEFT    0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE    0x08
#define LCD_1LINE    0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS  0x00

void lcd_init(void);
void lcd_write_string(uint8_t x, uint8_t y, char string[]);
void lcd_write_char(uint8_t x, uint8_t y, char val);

void lcd_clear(void);
void lcd_home(void);

void lcd_createChar(uint8_t, uint8_t[]);
void lcd_setCursor(uint8_t, uint8_t);

void lcd_noDisplay(void);
void lcd_display(void);
void lcd_noBlink(void);
void lcd_blink(void);
void lcd_noCursor(void);
void lcd_cursor(void);
void lcd_leftToRight(void);
void lcd_rightToLeft(void);
void lcd_autoscroll(void);
void lcd_noAutoscroll(void);
void scrollDisplayLeft(void);
void scrollDisplayRight(void);

size_t lcd_write(uint8_t);
void lcd_command(uint8_t);

void lcd_send(uint8_t, uint8_t);
void lcd_write4bits(uint8_t);
void lcd_write8bits(uint8_t);
void lcd_pulseEnable(void);

//function definitions
void adjustDifficulty(void);
void pretime(void);
void timer0(void);
void timer1(void);
void setupADC(void);
void eat(void);
void move(void);
void gameover(void);
void uart_setup(void);
void uart_process(void);
void uart_init(unsigned int ubrr);
unsigned char uart_getchar(void);
void uart_putchar(unsigned char data);
void uart_putstring(unsigned char* s);
void ftoa(float n, char * res, int afterpoint);
int intToStr(int x, char str[], int d);
void reverse(char * str, int len);

uint8_t _lcd_displayfunction;
uint8_t _lcd_displaycontrol;
uint8_t _lcd_displaymode;

#define BTNDDR       DDRC
#define BTNPIN       PINC
#define FUNCTION_BTN 1
#define UP_BTN       2
#define DOWN_BTN     3

#define LEDDDR      DDRB
#define LEDPORT     PORTB
#define LED_GREEN   2
#define LED_RED     3
#define LED_YELLOW  4
#define LED_BLUE    5

#define SCREEN_HEIGHT 2
#define SCREEN_WIDTH 16

#define GOBBLER_OM  0
#define GOBBLER_CM  1
#define BERRY       0b11001101


// 5x8 bitmap characters (pixel art)

// Character 0
// Gobbler Open-Mouth
uint8_t Gobbler_OM[8] = {0b00011100,
												 0b00010101,
												 0b00011110,
												 0b00011100,
												 0b00010110,
												 0b00010101,
											   0b00010100,
												 0b00010100};

// Character 1
// Gobbler Closed-Mouth
uint8_t Gobbler_CM[8] = {0b00011100,
												 0b00010100,
												 0b00011111,
												 0b00011111,
												 0b00010100,
												 0b00010100,
												 0b00010100,
												 0b00010100};

// defining variables
double survival_time = 0;
int character = 0;
int pwm_time = 0;
int score = 0;
int lives;


// movement patterns for each character

// gobbler only moves in y direction
// 0 and 1 in the x direction
static uint8_t gobblerx = 0;
static uint8_t gobblery = 0;

// berry moves in x direction and y direction
// 0 and 1 in the y direction
// 0 to 12 in the x direction
static uint8_t berryx = 2;
static uint8_t berryy = 1;

void setup_lcd0(void);
void setup_lcd1(void);
void setup_IO(void);
void process(void);
void move_character(uint8_t x, uint8_t y, uint8_t character);


// main function
// sequence of execution of the whole game
int main(void) {
	setupADC();
	timer0();
	timer1();
	timer2();
  setup_lcd0();
	uart_setup();
	adjustDifficulty();
	setup_lcd1();
  setup_IO();
  char score_text[14];
	TCNT2 = 0;
	SET_BIT(TIMSK2,TOIE2); // Enable timer2 interrupts
	SET_BIT(TIMSK0,TOIE0); // Enable timer0 interrupts
	// actual game loop
  while(1) {
		berryy = (rand() % 2);
		process();
		// game-over check
    if (lives < 1) {
			TCCR2B = 0;
			CLEAR_BIT(TIMSK0,TOIE0);
			gameover();
    	break;
    }
  }
  lcd_clear();
  snprintf(score_text, sizeof(score_text), "YOUR SCORE %d", score);
  lcd_write_string(0,0,score_text);
	char uart_buf[64];
	ftoa(survival_time, uart_buf, 2);
	uart_putstring((unsigned char *)"SURVIVAL TIME : ");
	uart_putstring((unsigned char *)uart_buf);
	uart_putstring((unsigned char *)" SECONDS");
	uart_putchar('\n');
	_delay_ms(30000);
	lcd_clear();
}

// lcd screen setup 1
void setup_lcd0(void) {
  lcd_init();
	_delay_ms(3000);
  lcd_write_string(0,0, "  WELCOME TO GOBBLER RUSH (^o^)");
  for (int i = 0; i < 20; ++i) {
    scrollDisplayLeft();
    _delay_ms(100);
		SET_BIT(LEDPORT,LED_RED);
		_delay_ms(200);
		CLEAR_BIT(LEDPORT,LED_RED);
		SET_BIT(LEDPORT,LED_YELLOW);
		_delay_ms(200);
		CLEAR_BIT(LEDPORT,LED_YELLOW);
		SET_BIT(LEDPORT,LED_BLUE);
		_delay_ms(200);
		CLEAR_BIT(LEDPORT,LED_BLUE);
  }
  lcd_clear();
  _delay_ms(4000);
}

// timer for software based pwm (setup)
void timer0() {
	TCCR0A = 0;
	// Lower the prescale, accuracy increases
	SET_BIT(TCCR0B,CS00); // Prescaler of 1024 (division factor)
	SET_BIT(TCCR0B,CS02);
	sei(); // Enable global interrupts and timer0 overflow
}
// calling from interrupt serivce routine for timer0
ISR(TIMER0_OVF_vect) {
	++pwm_time;
	if (BIT_VALUE(LEDPORT,LED_GREEN) == 1 && pwm_time > 50) {
		CLEAR_BIT(LEDPORT,LED_GREEN);
		pwm_time = 0;
	}
	if (BIT_VALUE(LEDPORT,LED_GREEN) == 0 && pwm_time > 50) {
		SET_BIT(LEDPORT,LED_GREEN);
		pwm_time = 0;
	}
}

// timer for debouncing (setup)
void timer1() {
	TCCR1A = 0;
	SET_BIT(TCCR1B,CS10); // Prescaler of 1
	SET_BIT(TIMSK1,TOIE1); // Enable timer interrupts
	sei(); // Enable global interrupts and timer1 overflow
}
// timer and interrupt based debouncing (from topic 9 ex 3)
volatile unsigned int functionbutton_statecount = 0;
volatile unsigned int pressed_state = 0;
// calling from intterupt service routine for timer1
ISR(TIMER1_OVF_vect) {
	functionbutton_statecount = (functionbutton_statecount<<1);
	uint8_t mask = 0b00000111;
  functionbutton_statecount &= mask;
  WRITE_BIT(functionbutton_statecount, 0, BIT_VALUE(BTNPIN,FUNCTION_BTN));
  if (functionbutton_statecount == mask) {
		pressed_state = 0;
	}
  if (functionbutton_statecount == 0) {
		pressed_state = 1;
	}
}

// timer for survival time (setup)
void timer2() {
	TCCR2A = 0;
	CLEAR_BIT(TCCR2B,WGM22);
	SET_BIT(TCCR2B,CS20); // Prescaler of 1024
	SET_BIT(TCCR2B,CS21);
	SET_BIT(TCCR2B,CS22);
	SET_BIT(LEDDDR,LED_GREEN);
	sei(); // Enable global interrupts and timer2 overflow
}
// calling from interrupt service routine for timer2
ISR(TIMER2_OVF_vect) {
	// timer2 is an 8 bit timer; 256
	// prescale value; 1024
	// frequency of the microcontroller; 16000000
	// overflow period = 256 * 1024 / 16000000
	// overflow period = 0.016384
	survival_time += 0.016384; // stopwatch implementation
}

// analog to digital conversion (setup)
void setupADC() {
	SET_BIT(ADCSRA,ADEN);
	SET_BIT(ADCSRA,ADPS2);
	SET_BIT(ADCSRA,ADPS1);
	SET_BIT(ADCSRA,ADPS1);
	SET_BIT(ADMUX,REFS0);
}

// (from topic 10 example 1.c)
// analog input from potentiometer to adjust the difficulty of the game
// easy - 5 live(s)
// normal - 3 lives(s)
// hard - 1 life
void adjustDifficulty() {
	lcd_write_string(0,0,"PLEASE ADJUST");
	lcd_write_string(0,1,"DIFFICULTY");
	_delay_ms(5000);
	lcd_clear();
	uint16_t value = 0;
  while(1) {
		char temp_buf[64];
    SET_BIT(ADCSRA,ADSC);
		while (ADCSRA & (1 << ADSC) ) {}
		if (ADC < 300 && ADC > 2) {
			lives = 5;
		}
		else if (ADC > 300 && ADC < 700) {
			lives = 3;
		}
		else if (ADC > 700) {
			lives = 1;
		}
		if (ADC < 2) {
			lcd_write_string(0,0,"USE THE KNOB    ");
			lcd_write_string(0,1,"BELOW GREEN LED ");
		}
		else {
 			snprintf(temp_buf, sizeof(temp_buf), "NO. OF LIVES: %d", lives);
			if (lives != value) {
				SET_BIT(LEDPORT,LED_GREEN);
				lcd_write_string(0,0,temp_buf);
				value = lives;
			}
			else {
				CLEAR_BIT(LEDPORT,LED_GREEN);
			}
			lcd_write_string(0,1,"RED -> CONTINUE");
			SET_BIT(LEDPORT,LED_RED);
			if (pressed_state != 0) {
				CLEAR_BIT(LEDPORT,LED_RED);
				break;
			}
	  }
  }
}

// lcd screen setup 2
void setup_lcd1(void){
	lcd_clear();
	_delay_ms(6000);

	lcd_write_string(0,0, "  YELLOW -> UP");
	SET_BIT(LEDPORT,LED_YELLOW);
	_delay_ms(5000);
	lcd_clear();
	CLEAR_BIT(LEDPORT,LED_YELLOW);
	_delay_ms(5000);

	lcd_write_string(0,0, "  BLUE -> DOWN");
	SET_BIT(LEDPORT,LED_BLUE);
	_delay_ms(5000);
	lcd_clear();
	CLEAR_BIT(LEDPORT,LED_BLUE);
	_delay_ms(5000);

  lcd_write_string(5,0, "READY!");
  _delay_ms(3000);
  lcd_clear();
  _delay_ms(1000);

  lcd_write_string(6,0, "SET!!");
  _delay_ms(3000);
  lcd_clear();
  _delay_ms(1000);

  lcd_write_string(4,0, "GOBBLE!!!");
  _delay_ms(2000);
  lcd_clear();
  _delay_ms(500);

  // creating the bitmap characters
  lcd_createChar(GOBBLER_OM, Gobbler_OM);
	lcd_createChar(GOBBLER_CM, Gobbler_CM);
}

//setting up buttons and LEDs
void setup_IO(void) {
  // enabling button input
  CLEAR_BIT(BTNDDR,FUNCTION_BTN);
  CLEAR_BIT(BTNDDR,UP_BTN);
  CLEAR_BIT(BTNDDR,DOWN_BTN);

	// setting LED's to output
	SET_BIT(LEDDDR,LED_RED);
	SET_BIT(LEDDDR,LED_YELLOW);
	SET_BIT(LEDDDR,LED_BLUE);
}

// gobbler's eating function
void eat (void) {
	if (character == 0) {
		move_character(gobblerx,gobblery, GOBBLER_CM);
		_delay_ms(100);
		character = 1;
	}
	else {
		move_character(gobblerx,gobblery, GOBBLER_OM);
		_delay_ms(100);
		character = 0;
	}
}

// gobbler's button movement function
void move(void) {
	if (!BIT_VALUE(BTNPIN,UP_BTN) == 1) {
		if (gobblery != 0) {
			gobblery = 0;
		}
	}
	else if (!BIT_VALUE(BTNPIN,DOWN_BTN) == 1) {
		if (gobblery != 1) {
			gobblery = 1;
		}
	}
}

// gobbler's and berry's movement interaction function
void process(void) {
  TCCR1B = 0;
  int berry_loop = 4;
  while(berry_loop < 17) {
  	berryx = 16 - berry_loop;
  	move_character(berryx,berryy, BERRY);
  	eat();
  	move();
    _delay_ms(125);
    lcd_clear();
    if (gobblerx == berryx && !gobblery == berryy) {
    	--lives;
    	lcd_clear();
    	lcd_write_string(1,0,"YOU DIDN'T EAT ");
			_delay_ms(4000);
			lcd_clear();
    }
    if (gobblerx == berryx && gobblery == berryy) {
  		++score;
		}
 	++berry_loop;
	}
}

// end of the game if the live(s) are over
void gameover(void) {
	_delay_ms(1000);
	lcd_clear();
	_delay_ms(3000);
	lcd_write_string(2,0,"GAME OVER :(");
	_delay_ms(3000);
	lcd_clear();
	lcd_write_string(0,0,"THANKYOU FOR PLAYING. HOPE YOU LOVED IT!    ");
	for (int i = 0; i < 24; ++i) {
		_delay_ms(300);
		scrollDisplayLeft();
	}
	_delay_ms(1000);
	CLEAR_BIT(LEDPORT,LED_RED);
	CLEAR_BIT(LEDPORT,LED_YELLOW);
	CLEAR_BIT(LEDPORT,LED_BLUE);
	CLEAR_BIT(LEDPORT,LED_GREEN);
}

// uart setup
void uart_setup(void) {
	uart_init(MYUBRR);
	timer2();
}

//  ==== commands & functions from topic 08 - "Serial Communications" ====  //

// Reverses a string 'str' of length 'len'
void reverse(char * str, int len) {
  int i = 0, j = len - 1, temp;
  while (i < j) {
    temp = str[i];
    str[i] = str[j];
    str[j] = temp;
    ++i;
    --j;
  }
}

// Converts a given integer x to string str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d) {
  int i = 0;
  while (x) {
    str[i++] = (x % 10) + '0';
    x = x / 10;
  }

  // If number of digits required is more, then
  // add 0s at the beginning
  while (i < d)
    str[i++] = '0';

  reverse(str, i);
  str[i] = '\0';
  return i;
}

// Converts a floating-point/double number to a string.
void ftoa(float n, char * res, int afterpoint) {
  // Extract integer part
  int ipart = (int) n;

  // Extract floating part
  float fpart = n - (float) ipart;

  // convert integer part to string
  int i = intToStr(ipart, res, 0);

  // check for display option after point
  if (afterpoint != 0) {
    res[i] = '.'; // add dot

    // Get the value of fraction part upto given no.
    // of points after dot. The third parameter
    // is needed to handle cases like 233.007
    fpart = fpart * pow(10, afterpoint);

    intToStr((int) fpart, res + i + 1, afterpoint);
  }
}

// initialize the UART
void uart_init(unsigned int ubrr) {
	UBRR0H = (unsigned char)(ubrr>>8);
  UBRR0L = (unsigned char)(ubrr);
	SET_BIT(UCSR0B,RXEN0);
	SET_BIT(UCSR0B,TXEN0);
	SET_BIT(UCSR0C,UCSZ00);
	SET_BIT(UCSR0C,UCSZ01);
}

// transmit a data
void uart_putchar(unsigned char data) {
    while (!( UCSR0A & (1<<UDRE0))); // Wait for empty transmit buffer
  	UDR0 = data; // Put data into buffer, sends the data
}

// receive data
unsigned char uart_getchar(void) {
  while (!(UCSR0A & (1<<RXC0))); // Wait for data to be received
  return UDR0; // Get and return received data from buffer
}

// transmit a string
void uart_putstring(unsigned char* s) {
	while(*s > 0) uart_putchar(*s++); // transsmit character until NULL is reached
}

//  ==== commands & functions from topic 11 - "The LCD Screen" ====  //
void move_character(uint8_t x, uint8_t y, uint8_t character) {
    lcd_setCursor(x,y);
    lcd_write(character);
}

// initialising the lcd screen
void lcd_init(void) {
  //dotsize
  if (LCD_USING_4PIN_MODE){
    _lcd_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  } else {
    _lcd_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  }

  _lcd_displayfunction |= LCD_2LINE;

  // RS Pin
  LCD_RS_DDR |= (1 << LCD_RS_PIN);
  // Enable Pin
  LCD_ENABLE_DDR |= (1 << LCD_ENABLE_PIN);

  #if LCD_USING_4PIN_MODE
    //Set DDR for all the data pins
    LCD_DATA4_DDR |= (1 << 4);
    LCD_DATA5_DDR |= (1 << 5);
    LCD_DATA6_DDR |= (1 << 6);
    LCD_DATA7_DDR |= (1 << 7);

  #else
    //Set DDR for all the data pins
    LCD_DATA0_DDR |= (1 << LCD_DATA0_PIN);
    LCD_DATA1_DDR |= (1 << LCD_DATA1_PIN);
    LCD_DATA2_DDR |= (1 << LCD_DATA2_PIN);
    LCD_DATA3_DDR |= (1 << LCD_DATA3_PIN);
    LCD_DATA4_DDR |= (1 << LCD_DATA4_PIN);
    LCD_DATA5_DDR |= (1 << LCD_DATA5_PIN);
    LCD_DATA6_DDR |= (1 << LCD_DATA6_PIN);
    LCD_DATA7_DDR |= (1 << LCD_DATA7_PIN);
  #endif

  // SEE PAGE 45/46 OF Hitachi HD44780 DATASHEET FOR INITIALIZATION SPECIFICATION!

  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  _delay_us(50000);
  // Now we pull both RS and Enable low to begin commands (R/W is wired to ground)
  LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);

  //put the LCD into 4 bit or 8 bit mode
  if (LCD_USING_4PIN_MODE) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    lcd_write4bits(0b0111);
    _delay_us(4500); // wait min 4.1ms

    // second try
    lcd_write4bits(0b0111);
    _delay_us(4500); // wait min 4.1ms

    // third go!
    lcd_write4bits(0b0111);
    _delay_us(150);

    // finally, set to 4-bit interface
    lcd_write4bits(0b0010);
  } else {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
    _delay_us(4500);  // wait more than 4.1ms

    // second try
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
    _delay_us(150);

    // third go
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
  }

  // finally, set # lines, font size, etc.
  lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);

  // turn the display on with no cursor or blinking default
  _lcd_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  lcd_display();

  // clear it off
  lcd_clear();

  // Initialize to default text direction (for romance languages)
  _lcd_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

void lcd_write_string(uint8_t x, uint8_t y, char string[]){
  lcd_setCursor(x,y);
  for(int i=0; string[i]!='\0'; ++i){
    lcd_write(string[i]);
  }
}

void lcd_write_char(uint8_t x, uint8_t y, char val){
  lcd_setCursor(x,y);
  lcd_write(val);
}

void lcd_clear(void){
  lcd_command(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
  _delay_us(2000); // this command takes a long time!
}

void lcd_home(void){
  lcd_command(LCD_RETURNHOME); // set cursor position to zero
  _delay_us(2000); // this command takes a long time!
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void lcd_createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  lcd_command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; ++i) {
    lcd_write(charmap[i]);
  }
}

void lcd_setCursor(uint8_t col, uint8_t row) {
  if ( row >= 2 ) {
    row = 1;
  }

  lcd_command(LCD_SETDDRAMADDR | (col + row*0x40));
}

// Turn the display on/off (quickly)
void lcd_noDisplay(void) {
  _lcd_displaycontrol &= ~LCD_DISPLAYON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

void lcd_display(void) {
  _lcd_displaycontrol |= LCD_DISPLAYON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Turns the underline cursor on/off
void lcd_noCursor(void) {
  _lcd_displaycontrol &= ~LCD_CURSORON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

void lcd_cursor(void) {
  _lcd_displaycontrol |= LCD_CURSORON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Turn on and off the blinking cursor
void lcd_noBlink(void) {
  _lcd_displaycontrol &= ~LCD_BLINKON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

void lcd_blink(void) {
  _lcd_displaycontrol |= LCD_BLINKON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void) {
  lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void scrollDisplayRight(void) {
  lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void lcd_leftToRight(void) {
  _lcd_displaymode |= LCD_ENTRYLEFT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This is for text that flows Right to Left
void lcd_rightToLeft(void) {
  _lcd_displaymode &= ~LCD_ENTRYLEFT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This will 'right justify' text from the cursor
void lcd_autoscroll(void) {
  _lcd_displaymode |= LCD_ENTRYSHIFTINCREMENT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This will 'left justify' text from the cursor
void lcd_noAutoscroll(void) {
  _lcd_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

inline void lcd_command(uint8_t value) {
  //
  lcd_send(value, 0);
}

inline size_t lcd_write(uint8_t value) {
  lcd_send(value, 1);
  return 1; // assume sucess
}

// write either command or data, with automatic 4/8-bit selection
void lcd_send(uint8_t value, uint8_t mode) {
  //RS Pin
  LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
  LCD_RS_PORT |= (!!mode << LCD_RS_PIN);

  if (LCD_USING_4PIN_MODE) {
    lcd_write4bits(value>>4);
    lcd_write4bits(value);
  } else {
    lcd_write8bits(value);
  }
}

void lcd_pulseEnable(void) {
  //Enable Pin
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  _delay_us(1);
  LCD_ENABLE_PORT |= (1 << LCD_ENABLE_PIN);
  _delay_us(1);    // enable pulse must be >450ns
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  _delay_us(100);   // commands need > 37us to settle
}

void lcd_write4bits(uint8_t value) {
  //Set each wire one at a time
  LCD_DATA4_PORT &= ~(1 << LCD_DATA4_PIN);
  LCD_DATA4_PORT |= ((value & 1) << LCD_DATA4_PIN);
  value >>= 1;

  LCD_DATA5_PORT &= ~(1 << LCD_DATA5_PIN);
  LCD_DATA5_PORT |= ((value & 1) << LCD_DATA5_PIN);
  value >>= 1;

  LCD_DATA6_PORT &= ~(1 << LCD_DATA6_PIN);
  LCD_DATA6_PORT |= ((value & 1) << LCD_DATA6_PIN);
  value >>= 1;

  LCD_DATA7_PORT &= ~(1 << LCD_DATA7_PIN);
  LCD_DATA7_PORT |= ((value & 1) << LCD_DATA7_PIN);

  lcd_pulseEnable();
}

void lcd_write8bits(uint8_t value) {
  //Set each wire one at a time
  #if !LCD_USING_4PIN_MODE
    LCD_DATA0_PORT &= ~(1 << LCD_DATA0_PIN);
    LCD_DATA0_PORT |= ((value & 1) << LCD_DATA0_PIN);
    value >>= 1;

    LCD_DATA1_PORT &= ~(1 << LCD_DATA1_PIN);
    LCD_DATA1_PORT |= ((value & 1) << LCD_DATA1_PIN);
    value >>= 1;

    LCD_DATA2_PORT &= ~(1 << LCD_DATA2_PIN);
    LCD_DATA2_PORT |= ((value & 1) << LCD_DATA2_PIN);
    value >>= 1;

    LCD_DATA3_PORT &= ~(1 << LCD_DATA3_PIN);
    LCD_DATA3_PORT |= ((value & 1) << LCD_DATA3_PIN);
    value >>= 1;

    LCD_DATA4_PORT &= ~(1 << LCD_DATA4_PIN);
    LCD_DATA4_PORT |= ((value & 1) << LCD_DATA4_PIN);
    value >>= 1;

    LCD_DATA5_PORT &= ~(1 << LCD_DATA5_PIN);
    LCD_DATA5_PORT |= ((value & 1) << LCD_DATA5_PIN);
    value >>= 1;

    LCD_DATA6_PORT &= ~(1 << LCD_DATA6_PIN);
    LCD_DATA6_PORT |= ((value & 1) << LCD_DATA6_PIN);
    value >>= 1;

    LCD_DATA7_PORT &= ~(1 << LCD_DATA7_PIN);
    LCD_DATA7_PORT |= ((value & 1) << LCD_DATA7_PIN);

    lcd_pulseEnable();
  #endif
}
