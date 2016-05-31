
typedef struct timer {
	unsigned char min2, min1, sec2, sec1;
} timerCntrl;

static timerCntrl mainTimer;

void outputTime(){ //Task tick function
	_delay_ms(100);
	if (mainTimer.sec1++ == 9) {
		mainTimer.sec2++;
		mainTimer.sec1 = 0;
		lcd_goto_xy(3, 0);
		lcd_chr('0' + mainTimer.sec2);
		lcd_goto_xy(4, 0);
		lcd_chr('0' + mainTimer.sec1);
	}
	else {
		lcd_goto_xy(4, 0);
		lcd_chr('0' + mainTimer.sec1);
	}
	if (mainTimer.sec2 == 6){
		mainTimer.min1++;
		if (mainTimer.min1 == 1){
			phase = 0x02;
			interrupt = 1;
		}
		else if (mainTimer.min1 == 3){
			phase = 0x03;
			interrupt = 1;
		}
		mainTimer.sec2 = 0;
		lcd_goto_xy(3, 0);
		lcd_chr('0' + mainTimer.sec2);
		lcd_goto_xy(1, 0);
		lcd_chr('0' + mainTimer.min1);

	}
	if (mainTimer.min1 == 10){
		mainTimer.min2++;
		mainTimer.min1 = 0;
		lcd_goto_xy(1, 0);
		lcd_chr('0' + mainTimer.min1);
		lcd_goto_xy(0,0);
		lcd_chr('0' + mainTimer.min2);
	}
}
void resetTimer(){
	mainTimer.min2 = 0;
	mainTimer.min1 = 0;
	mainTimer.sec2 = 0;
	mainTimer.sec1 = 0;
}
enum TimerControl {TimerStart, TimerWaitGame, GameTimerOn};
int TimerControlTick(int state){
	switch(state){
		case TimerStart:
		state = TimerWaitGame;
		end = 0;
		game_on = 0;
		phase = 0x00;
		health = 0;
		resetTimer();
		break;

		case TimerWaitGame:
		if (start) {
			phase = 0x01;
			game_on = 1;
			health = 0x03FF;
			state = GameTimerOn;
			resetTimer();
			_delay_ms(100);
			lcd_goto_xy(0,0);
			lcd_chr('0' + mainTimer.min2);
			lcd_goto_xy(1,0);
			lcd_chr('0' + mainTimer.min1);
			lcd_goto_xy(2,0);
			lcd_chr(':');
			lcd_goto_xy(3, 0);
			lcd_chr('0' + mainTimer.sec2);
			lcd_goto_xy(4, 0);
			lcd_chr('0' + mainTimer.sec1);
			interrupt = 1;
		}
		break;

		case GameTimerOn:
		if (!(health == 0)){
			outputTime();
			game_on = 1;
		}
		else if (health == 0){
			phase = 0x00;
			game_on = 0;
			end = 1;
			state = TimerStart;
		}
		break;
	}
	return state;
};