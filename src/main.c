/*
Quynh Nguyen
SID: 861149235
Project Name: Linked
Description:


*/
#include <avr/io.h>
#include "io.c"
#include <stdio.h>
#include <stdlib.h>
#include "timeout.h"
#include "5110.h"
#include "5110.c"
#include "LCD_Pictures.h"
#include <math.h>
#include "usart.h"

static unsigned long seed = 1;

/**************************Player Stats**************************/
typedef struct playerStats {
	unsigned char totalSIN;
	unsigned char SINwin;
	unsigned char totalSS;
	unsigned char SSwin;
	unsigned char totalSM;
	unsigned char SMwin;
} player;

static player Player;

void initPlayer(){
	Player.totalSIN = 0;
	Player.SINwin = 0;
	Player.totalSS = 0;
	Player.SSwin = 0;
}

/**************************ADC Init**************************/
void ADC_init() {
	//ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	// in Free Running Mode, a new conversion will trigger whenever
	// the previous conversion completes.
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}

/**************************Set/Get Bit**************************/
unsigned short SetBit(unsigned short x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
unsigned char GetBit(unsigned short x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

/**************************Shift Register**************************/
void transmit_data(unsigned char data){
	int i;
	for (i = 0; i < 8; i++)
	{
		PORTA = 0x08;
		PORTA |= ((data >> i) & 0x01);
		PORTA |= 0x02;
	}
	PORTA |= 0x04;
	PORTA = 0x00;
}

/**************************Task Scheduler*************************/
// Struct for Tasks represent a running process in our simple real-time operating system.
typedef struct _task {
	/*Tasks should have members that include: state, period,
	a measurement of elapsed time, and a function pointer.*/
	signed char state; //Task's current state
	unsigned long int period; //Task period
	unsigned long int elapsedTime; //Time elapsed since last task tick
	int (*TickFct)(int); //Task tick function
} task;
static task task_Start, task_Timer, task_MiniGameController, task_UpdateHealth;
task *tasks[] = {&task_Start, &task_Timer, &task_MiniGameController, &task_UpdateHealth};
unsigned short numTasks;

/**************************Timer Code**************************/
//Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; //Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; //Current internal count of 1ms ticks


void TimerOn() {
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //bit3 = 0: CTC mode (clear timer on compare)
	//bit2bit1bit0 = 011: pre-scaler /64
	//00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	//Thus, TCNT1 register will count as 125,000 ticks/s

	//AVR output compasre register OCR1A
	OCR1A = 125; //Timer interrupt will be generated when TCNT1==OCR1A
	//We want a 1ms tick. 0.001 s * 125,000 ticks/s = 125
	//SO, when TCNT1 register equals 125
	//1 ms has passed. Thus, we compare to 125.
	//AVR timer interrupt mask register
	TIMSK1 = 0x02; //bit1: OCIE1A -- enables compare match interrupt

	//Initializes avr counter
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every_avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; //0x80 = 10000000
}

void TimerOff() {
	TCCR1B = 0x00; //bit3bit1bit0 = 000: timer off
}

void TimerISR(){
	for ( short i = 0; i < numTasks; i++ ) {
		// Task is ready to tick
		if ( tasks[i]->elapsedTime == tasks[i]->period ) {
			// Setting next state for task
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			// Reset the elapsed time for next tick.
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 1;
		seed = seed + 10;
	}
}

//In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect){
	//CPU automatically calls when TCNT1 == OCR1 (Every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; //Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { //results in more efficient compare
		TimerISR(); //Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

//Set TimerISR() to tick every M ms
void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
//--------Find GCD function --------------------------------------------------
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}
//--------Shared Variables----------------------------------------------------
static unsigned char start;
static unsigned char end; 
static unsigned char phase;
static unsigned char game_on;
static unsigned char win; 
static unsigned short health;
static unsigned char interrupt;
char* instructions[] = {"A)Next page\nB)Prev page\nC)Menu", "Welcome playerYou will play a series of \nmini games.\nFollow the\ndirection(1/3)",
"Your life willgo down as thegame goes on.\n\n\n         (2/3)","Win the mini  games to keep your partner  alive!\n\n         (3/3)" };

//Tasks inside the scheduler
#include "GameHandler.h"
#include "TimerHandler.h"
#include "MiniGameHandler.h"
#include "miniGames.h"
#include "HealthHandler.h"
//

int main()
{

	numTasks = sizeof(tasks)/sizeof(task*);

	/*******************Set tasks in scheduler*******************/
	// Task 1 - Game Handler 
	task_Start.state = Start;//Task initial state.
	task_Start.period = 1;//Task Period.
	task_Start.elapsedTime = 1;//Task current elapsed time.
	task_Start.TickFct = &Game_Handler_Tick;//Function pointer for the tick.

	// Task 2
	task_Timer.state = TimerStart;//Task initial state.
	task_Timer.period = 10;//Task Period.
	task_Timer.elapsedTime = 10;//Task current elapsed time.
	task_Timer.TickFct = &TimerControlTick;//Function pointer for the tick.

	// Task 3
	task_MiniGameController.state = MGCStart;//Task initial state.
	task_MiniGameController.period = 1;//Task Period.
	task_MiniGameController.elapsedTime = 1;//Task current elapsed time.
	task_MiniGameController.TickFct = &MGC_Tick;//Function pointer for the tick.

	// Task 4
	task_UpdateHealth.state = UHStart;//Task initial state.
	task_UpdateHealth.period = 1;//Task Period.
	task_UpdateHealth.elapsedTime = 1;//Task current elapsed time.
	task_UpdateHealth.TickFct = &updateHealthTick;//Function pointer for the tick.

	/*******************Set minigames in Mini Game Handler*******************/
// 
//  	SIN_Game.state = SINStart;
//  	SIN_Game.GameFct = &SINTick;
//  	SIN_Game.done = 0;
	
// 	SS_Game.state = SSStart;
// 	SS_Game.GameFct = &SimonSaysTick;
// 	SS_Game.done = 0;

// 	SM_Game.state = SMStart;
// 	SM_Game.GameFct = &ShapeMatchTick;
// 	SM_Game.done = 0;
	
	
	M_Game.state = MStart;
	M_Game.GameFct = &MazeTick;
	M_Game.done = 0;
	//Initialize Ports
	DDRB = 0xFF; PORTB = 0x00; // PORTB set to output, outputs init 0s
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0xF0; PORTC = 0x0F; // PC7..4 outputs init 0s, PC3..0 inputs init 1s
	DDRA = 0xCF; PORTA = 0x00; //input

	//Init
	lcd_init(&PORTB, PB0, &PORTB, PB1, &PORTB, PB2, &PORTB, PB3, &PORTB, PB4);
	ADC_init();
	initUSART(0);

	//Set Timer
	TimerSet(100);
	TimerOn();

	while(1) {}
	return 0;
}