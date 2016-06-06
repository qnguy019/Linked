typedef struct playerStats {
	unsigned char totalSIN;
	unsigned char SINwin;
	unsigned char totalSS;
	unsigned char SSwin;
	unsigned char totalSM;
	unsigned char SMwin;
	unsigned char totalR;
	unsigned char Rwin;
	unsigned char totalM;
	unsigned char Mwin;
} player;

static player Player;

void initPlayer(){
	Player.totalSIN = 0;
	Player.SINwin = 0;
	Player.totalSS = 0;
	Player.SSwin = 0;
	Player.totalSM = 0;
	Player.SMwin = 0;
	Player.totalR = 0;
	Player.Rwin = 0;
	Player.totalM = 0;
	Player.Mwin = 0;
}
void outputEndGame(){
	lcd_clear_space(1,5);
	lcd_goto_xy(0,1);
	lcd_string_format("6 in 9: %d/%d", Player.SINwin, Player.totalSIN);

	lcd_goto_xy(0,2);
	lcd_string_format("S Says: %d/%d", Player.SSwin, Player.totalSS);

	lcd_goto_xy(0,3);
	lcd_string_format("Shape:  %d/%d", Player.SMwin, Player.totalSM);

	lcd_goto_xy(0,4);
	lcd_string_format("Reflex: %d/%d", Player.Rwin, Player.totalR);

	lcd_goto_xy(0,5);
	lcd_string_format("Maze:   %d/%d", Player.Mwin, Player.totalM);

	lcd_goto_xy(7,0);
	lcd_string_format("C)Menu");
}
/*Stores mini game states*/
/********************Six in Nines********************/
/*
Objective: To find the number of six's in nines
Difficulty: Players will have less time to answer as the game goes on
(originally 10 seconds to guess, then 8, then 6)
*/
//Game to find the amount of 6's in 9's
enum SIN {SINStart, SINWait, SINGame, SINDone};
int SINTick(int state){
	if(interrupt){
		state = SINStart;
		return state;
	}
	static unsigned char num_six;
	static unsigned char time_count;
	static unsigned char max_time;
	switch(state){
		unsigned char keyPress;
		case SINStart:
		state = SINWait;
		num_six = 0;
		time_count = 0;
		max_time = 0;
		break;

		case SINWait:
		if (game_index == 0 && game_on){
			state = SINGame;
			lcd_clear_space(1,5);
			lcd_goto_xy(0,1);
			lcd_string_format("# of 6 in 9?");
			if (phase == 0x01) max_time = 100;
			else if (phase == 0x02) max_time = 80;
			else if (phase == 0x03) max_time = 70;
			srand(seed);
			for (int i=0;i<14;i++)
			for (int j=2;j<6;j++) {
				lcd_goto_xy(i,j);
				if (rand() % 7 == 0 && num_six != 9){
					lcd_chr('0' + 6);
					num_six++;
				}
				else lcd_chr('0' + 9);
			}
			Player.totalSIN++;

		}
		else if (!game_on) state = SINWait;
		else state = SINWait;
		break;

		case SINGame:
		keyPress = 12;
		PORTC = 0x7F; // Enable col 4 with 0, disable others with 1?s
		asm("nop"); // add a delay to allow PORTC to stabilize before checking
		if (GetBit(PINC,3)==0) { keyPress = 1; }
		if (GetBit(PINC,2)==0) { keyPress = 4; }
		if (GetBit(PINC,1)==0) { keyPress = 7; }
		
		PORTC = 0xBF; // Enable col 4 with 0, disable others with 1?s
		asm("nop"); // add a delay to allow PORTC to stabilize before checking
		if (GetBit(PINC,3)==0) { keyPress = 2; }
		if (GetBit(PINC,2)==0) { keyPress = 5; }
		if (GetBit(PINC,1)==0) { keyPress = 8; }
		if (GetBit(PINC,0)==0) { keyPress = 0; }
		
		
		PORTC = 0xDF; // Enable col 4 with 0, disable others with 1?s
		asm("nop"); // add a delay to allow PORTC to stabilize before checking
		if (GetBit(PINC,3)==0) { keyPress = 3; }
		if (GetBit(PINC,2)==0) { keyPress = 6; }
		if (GetBit(PINC,1)==0) { keyPress = 9; }
		if (keyPress == num_six){
			win = 1;
			state = SINDone;
			lcd_clear_space(1, 5);
			printPictureAtPos(completed, 0, 1, 84, 30);
			time_count = 0;
			Player.SINwin++;
			break;
		}
		else if (keyPress != 12 || time_count == max_time){
			win = 0;
			state = SINDone;
			lcd_clear_space(1, 5);
			printPictureAtPos(wrong, 0, 1, 84, 30);
			time_count = 0;
			break;
		}
		time_count++;
		break;

		case SINDone:
		time_count++;
		if (time_count == 1) win = 0;
		if (time_count == 10){
			win = 0;
			state = SINStart;
			gamesArray[game_index]->done = 1;
			time_count = 0;
			lcd_clear_space(1, 5);
			break;
		}
		break;
	}
	return state;
};


/********************Simon Says********************/
/*
Objective: To input the correct sequence of arrows
Difficulty: Players will have to memorize longer sequences as time goes on
(originally 4, 5, then 7)
*/
enum SimonSays {SSStart, SSWait, SSMemorize, SSAnswer, SSConfirm, SSDone};
void printArrow(int select, int x_pos, int y_pos){
	if (select == 0) printPictureAtPos(rightArrow, x_pos, y_pos, 10, 10);
	else if (select == 1) printPictureAtPos(leftArrow, x_pos, y_pos, 10, 10);
	else if (select == 2) printPictureAtPos(upArrow, x_pos, y_pos, 10, 10);
	else if (select == 3) printPictureAtPos(downArrow, x_pos, y_pos, 10, 10);
};

//Function to read the ADC value from the joystick
uint16_t ReadADC(uint16_t ch){
	//Select ADC Channel. ch must be 0-7
	ADMUX = (ADMUX & 0xF8) | (ch & 0x1F);
	ADCSRB = (ADCSRB & 0xDF) | (ch & 0x20);
	//Start single conversion
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC));
	return (ADC);
}

//Function to read the input from the joystick
unsigned char checkInput(){
 	unsigned short adc_value_UD = ReadADC(4);
 	unsigned short adc_value_LR = ReadADC(5);
    if (adc_value_UD == 0x006E ||
    adc_value_UD == 0x006C || adc_value_UD == 0x0065 || adc_value_UD == 0x0067 ||
    adc_value_UD == 0x0066 || adc_value_UD == 0x0068 || adc_value_UD == 0x006F||
    adc_value_UD == 0x008D || adc_value_UD == 0x0088 || adc_value_UD == 0x0089 || adc_value_UD == 0x007F || adc_value_UD == 0x0047
    || adc_value_UD == 0x0041 || adc_value_UD == 0x0042 || adc_value_UD == 0x0047 || adc_value_UD == 0x0045 || adc_value_UD == 0x0046 ||
    adc_value_UD == 0x0044 || adc_value_UD == 0x0077|| adc_value_UD == 0x0054 || adc_value_UD == 0x005F ||  adc_value_UD ==  0X005B ||
    adc_value_UD == 0x006D || adc_value_UD == 0x006F|| adc_value_UD == 0x0058|| adc_value_UD == 0X0059 || adc_value_UD == 0X0069||
    adc_value_UD == 0X0061 || adc_value_UD ==0X0062|| adc_value_UD == 0X0063|| adc_value_UD == 0X009F|| adc_value_UD == 0X007D ||
    adc_value_UD ==0X008B|| adc_value_UD == 0X007C ||adc_value_UD == 0x0055 || adc_value_UD == 0x0053 || adc_value_UD == 0x0052
    || adc_value_UD == 0x003F || adc_value_UD == 0x007F || adc_value_UD == 0x047 || adc_value_UD == 0x004F || adc_value_UD == 0x004B
   	||adc_value_UD == 0x006B || adc_value_UD == 0x006) return 1; //left 0x006D
  	else if (adc_value_UD == 0x03FF) return 0; //right
  	else if (adc_value_LR == 0x003F || adc_value_LR == 0x0096 || adc_value_LR == 0x0040 || adc_value_LR == 0x0041 || adc_value_LR == 0x0047 ||
	adc_value_LR == 0x05D ||adc_value_LR == 0x005F ||adc_value_LR == 0x0077 || adc_value_LR == 0x003D ||
	adc_value_LR == 0x007E || adc_value_LR == 0x005B || adc_value_LR == 0x007F || adc_value_LR == 0x0047 ) return 2; //up
  	else if (adc_value_LR == 0x03FF) return 3; //down
	return 4;
}
//Tick function
int SimonSaysTick(int state){
	if(interrupt){
		state = SSStart;
		return state;
	}
	static unsigned char sequence[10];
	static unsigned char max_sequence;
	static unsigned char time_count;
	static unsigned char seq_index;
	unsigned char joystick_input;
	static unsigned char print_posx;
	static unsigned char print_posy;
	static unsigned char answer;
	static unsigned char buttonPress;
	switch (state){
		case SSStart:
		state = SSWait;
		max_sequence = 0;
		time_count = 0;
		seq_index = 0;
		print_posx = 0;
		print_posy = 0;
		break;

		case SSWait:
		if (game_index == 1 && game_on){
			state = SSMemorize;
			lcd_clear_space(1, 5);
			lcd_goto_xy(0, 1);
			lcd_string_format("Memorize the \nsequence...");

			if (phase == 0x01) max_sequence = 4;
			if (phase == 0x02) max_sequence = 6;
			if (phase == 0x03) max_sequence = 7;
			int first_row = 0;
			int i = 0;
			srand(seed);
			for (i = 0; i < max_sequence; i++){
				//print out different arrows
				unsigned char arrow = rand() % 3;
				printArrow(arrow, first_row, 4);
				first_row += 2;
				sequence[i] = arrow;
			}
			sequence[i] = 5;
			time_count = 0;
			Player.totalSS++;
		}
		else if (!game_on) state = SSStart; 
		else state = SSStart;
		break;

		case SSMemorize:
		if (time_count >= 70){
			state = SSAnswer;
			time_count = 0;
			seq_index = 0;
			print_posx = 0;
			print_posy = 5;
			buttonPress = 0;
			lcd_clear_space(1, 5);
			lcd_goto_xy(0, 1);
			lcd_string_format("Use joystick &press A to\nconfirm each\ndirection.");
		}
		else time_count++;
		break;

		case SSAnswer:
		if (time_count >= 200){
    		state = SSDone;
    		time_count = 0;
    		lcd_clear_space(1, 5);
    		printPictureAtPos(wrong, 0, 1, 84, 30);
    		win = 0;
    		break;
		}
			joystick_input = checkInput();
			if (joystick_input == 0 || joystick_input == 1 || joystick_input == 2 || joystick_input == 3){
				printArrow(joystick_input, print_posx, print_posy);
				answer = joystick_input;
			}
			if (GetBit(PINC,3)==0 && buttonPress == 0 && joystick_input != '\0') {
				state = SSConfirm;
				buttonPress = 1;
			}
			else if (GetBit(PINC,3)==1) buttonPress = 0;
 			if (time_count == 70){
 				state = SSDone;
 				time_count = 0;
 				lcd_clear_space(1, 5);
 				printPictureAtPos(wrong, 0, 1, 84, 30);
 				win = 0;
 				break;
 			}
  			if (sequence[seq_index] == 5){
 	 			state = SSDone;
 	 			time_count = 0;
 	 			lcd_clear_space(1, 5);
 	 			printPictureAtPos(completed, 0, 1, 84, 30);
 	 			win = 1;
				Player.SSwin++;
 	 			break;
  			}
 			time_count++;
			break;

		case SSConfirm:
  	 	if (answer == sequence[seq_index]){ //answer correctly
			seq_index++;
  			state = SSAnswer;
			printPictureAtPos(Circle, print_posx, print_posy - 1, 10, 10);
			print_posx += 2;
			break;
  	 	}
    	 	else if (answer != sequence[seq_index]){
    		 	state = SSDone;
    		 	time_count = 0;
    		 	lcd_clear_space(1, 5);
    		 	printPictureAtPos(wrong, 0, 1, 84, 30);
    		 	win = 0;
    		 	break;
    	 	}
		break;

		case SSDone:
		time_count++;
		if (time_count == 1) win = 0;
		if (time_count == 15){
			win = 0;
			state = SSStart;
			gamesArray[game_index]->done = 1;
			time_count = 0;
			break;
		}

		break;
	}
	return state;
};
/********************Shape Matching********************/
/*
Objective: Match the number with the given shape
Difficulty: Players will have less time to memorize the shapes
(originally 35, 20, 10)
*/
enum ShapeMatch{SMStart, SMWait, SMInstructions, SMAnswer, SMDone};
void printSquare(int select, int posx, int posy){
	if (select == 0) printPictureAtPos(oneSquare, posx, posy, 84, 30);
	else if (select == 1) printPictureAtPos(twoSquare, posx, posy, 84, 30);
	else if (select == 2) printPictureAtPos(threeSquare, posx, posy, 84, 30);
	else if (select == 3) printPictureAtPos(fourSquare, posx, posy, 84, 30);
	else if (select == 4) printPictureAtPos(fiveSquare, posx, posy, 84, 30);
	else if (select == 5) printPictureAtPos(sixSquare, posx, posy, 84, 30);
	else if (select == 6) printPictureAtPos(sevenSquare, posx, posy, 84, 30);
	else if (select == 7) printPictureAtPos(eightSquare, posx, posy, 84, 30);
	else if (select == 8) printPictureAtPos(nineSquare, posx, posy, 84, 30);
};
void printCircle(int select, int posx, int posy){
	if (select == 0) printPictureAtPos(oneCircle, posx, posy, 84, 30);
	else if (select == 1) printPictureAtPos(twoCircle, posx, posy, 84, 30);
	else if (select == 2) printPictureAtPos(threeCircle, posx, posy, 84, 30);
	else if (select == 3) printPictureAtPos(fourCircle, posx, posy, 84, 30);
	else if (select == 4) printPictureAtPos(fiveCircle, posx, posy, 84, 30);
	else if (select == 5) printPictureAtPos(sixCircle, posx, posy, 84, 30);
	else if (select == 6) printPictureAtPos(sevenCircle, posx, posy, 84, 30);
	else if (select == 7) printPictureAtPos(eightCircle, posx, posy, 84, 30);
	else if (select == 8) printPictureAtPos(nineCircle, posx, posy, 84, 30);
};
void printTri(int select, int posx, int posy){
	if (select == 0) printPictureAtPos(oneTri, posx, posy, 84, 30);
	else if (select == 1) printPictureAtPos(twoTri, posx, posy, 84, 30);
	else if (select == 2) printPictureAtPos(threeTri, posx, posy, 84, 30);
	else if (select == 3) printPictureAtPos(fourTri, posx, posy, 84, 30);
	else if (select == 4) printPictureAtPos(fiveTri, posx, posy, 84, 30);
	else if (select == 5) printPictureAtPos(sixTri, posx, posy, 84, 30);
	else if (select == 6) printPictureAtPos(sevenTri, posx, posy, 84, 30);
	else if (select == 7) printPictureAtPos(eightTri, posx, posy, 84, 30);
	else if (select == 8) printPictureAtPos(nineTri, posx, posy, 84, 30);
};
int ShapeMatchTick(int state){
	if(interrupt){
		state = SMStart;
		return state;
	}
	static unsigned int shape_selected; //0x01 = circle, 0x02 = square, 0x03 = triangle
	static unsigned int answer_shape; //the number corresponding the the shape
	static unsigned char high; //Determines how long they will see the image
	static unsigned char image;
	static unsigned char time_count;
	static unsigned char keyPress;
	unsigned char random_num;
	switch(state){
		case SMStart:
		shape_selected = 0;
		answer_shape = 0;
		high = 0;
		image = 0;
		state = SMWait;
		break;

		case SMWait:
		if (game_index == 2 && game_on){
			//srand(seed);
			state = SMInstructions;
			lcd_clear_space(1,5);
			lcd_goto_xy(0, 1);
			lcd_string_format("Memorize what you see");
			Player.totalSM++;
			
			answer_shape = rand() % 9;
			shape_selected = rand () % 3;
			if (phase == 0x01) high = 30;
			else if (phase == 0x02) high = 20;
			else if (phase == 0x03) high = 10;
		}
		else if (!game_on) state = SMWait;
		else state = SMWait;
		break;

		case SMInstructions:
		//srand(seed);
		random_num = rand() % 9;
		if (time_count == 20 && image == 0){ //square 
			time_count = 0;
			image++;
			lcd_clear_space(1, 5);
			if (shape_selected == 0) {
				printSquare(answer_shape, 0, 1);
			}
			else printSquare(random_num, 0, 1);
			
		}
		else if (time_count == high && image == 1){ //circle 
			time_count = 0;
			image++;
			lcd_clear_space(1, 5);
			if (shape_selected == 1) {
				printCircle(answer_shape, 0, 1);
			}
			else printCircle(random_num, 0, 1);
		}
		else if (time_count == high && image == 2){ //triangle
			time_count = 0;
			image++;
			lcd_clear_space(1, 5);
			if (shape_selected == 2) {
				printTri(answer_shape, 0, 1);
			}
			else printTri(random_num, 0, 1);
		}
		else if (time_count == high && image == 3){
			lcd_clear_space(1,5);
			lcd_goto_xy(0, 1);
			if (shape_selected == 0) lcd_string_format("What number\nwasthe\nSQUARE?");
			else if (shape_selected == 1) lcd_string_format("What number\nwas\nthe\nCIRCLE?");
			else if (shape_selected == 2) lcd_string_format("What number\nwas\nthe\nTRIANGLE?");
			state = SMAnswer;
			image = 0;
			time_count = 0;
		}
		else time_count++;
		break;

		case SMAnswer:
		keyPress = 12;
		PORTC = 0x7F; // Enable col 4 with 0, disable others with 1?s
		asm("nop"); // add a delay to allow PORTC to stabilize before checking
		if (GetBit(PINC,3)==0) { keyPress = 1; }
		if (GetBit(PINC,2)==0) { keyPress = 4; }
		if (GetBit(PINC,1)==0) { keyPress = 7; }
		
		PORTC = 0xBF; // Enable col 4 with 0, disable others with 1?s
		asm("nop"); // add a delay to allow PORTC to stabilize before checking
		if (GetBit(PINC,3)==0) { keyPress = 2; }
		if (GetBit(PINC,2)==0) { keyPress = 5; }
		if (GetBit(PINC,1)==0) { keyPress = 8; }
		
		
		PORTC = 0xDF; // Enable col 4 with 0, disable others with 1?s
		asm("nop"); // add a delay to allow PORTC to stabilize before checking
		if (GetBit(PINC,3)==0) { keyPress = 3; }
		if (GetBit(PINC,2)==0) { keyPress = 6; }
		if (GetBit(PINC,1)==0) { keyPress = 9; }
		if (keyPress == (answer_shape + 1)){
			win = 1;
			state = SMDone;
			lcd_clear_space(1, 5);
			printPictureAtPos(completed, 0, 1, 84, 30);
			time_count = 0;
			Player.SMwin++;
			break;
		}
		else if (keyPress != 12 || time_count == 50){
			win = 0;
			state = SMDone;
			lcd_clear_space(1, 5);
			printPictureAtPos(wrong, 0, 1, 84, 30);
			time_count = 0;
			break;
		}
		time_count++;
		break;
		case SMDone:
		time_count++;
		if (time_count == 1) win = 0;
		if (time_count == 10){
			win = 0;
			state = SMStart;
			gamesArray[game_index]->done = 1;
			time_count = 0;
			break;
		}
		break;
	}	

	return state;

}
/********************It's High Noon********************/
/*
Objective: Press * when the ! pops up
Difficulty: As the phase increases, players will have less time to react
*/

enum Reflex{RStart, RWait, RInstructions, RStartGame, RReaction, RDone};
int ReflexTick(int state){
	static unsigned char wait_time;
	static unsigned char reaction_time;
	static unsigned char time_count;
	static unsigned char wonGame = 0;
	switch(state){
		case RStart:
		state = RWait;
		wait_time = 0;
		reaction_time = 0;
		time_count = 0;
		break;

		case RWait:
		if (game_index == 3 && game_on){
			state = RInstructions;
			wait_time = (rand() % 30) + 40;
			if (phase == 0x01) reaction_time = 5;
			else if (phase == 0x02) reaction_time = 3;
			else if (phase == 0x03) reaction_time = 1;
			time_count = 0;
			lcd_clear_space(1,5);
			lcd_goto_xy(0, 1);
			lcd_string_format("It's High NoonPress *\nwhen the\nexclamation\npoint pops up!");
			Player.totalR++;
		}
		else if (!game_on) state = RStart;
		else state = RStart;
		break;

		case RInstructions:
		if (time_count == 40){
			state = RStartGame;
			lcd_clear_space(1,5);
			printPictureAtPos(cowboyStart, 0, 1, 84, 48);
		}
		else time_count++;
		break;

		case RStartGame:
		if (time_count == wait_time){
			state = RReaction;
			printPictureAtPos(cowboyGo2, 0, 1, 84, 48);
			time_count = 0;
			break;
		}
		else time_count++;
		PORTC = 0x7F; // Enable col 4 with 0, disable others with 1?s
		asm("nop"); // add a delay to allow PORTC to stabilize before checking
		if (GetBit(PINC,0)==0) {
			win = 0;
			state = RDone;
			lcd_clear_space(1, 5);
			printPictureAtPos(wrong, 0, 1, 84, 30);
			time_count = 0;
			wonGame = 0x00;
			break;
		}

		break;

		case RReaction:
		PORTC = 0x7F; // Enable col 4 with 0, disable others with 1?s
		asm("nop"); // add a delay to allow PORTC to stabilize before checking
		if (GetBit(PINC,0)==0 && time_count <= reaction_time) {
			win = 1;
			wonGame = 0x01;
			state = RDone;
			lcd_clear_space(1, 5);
			printPictureAtPos(cowboyPlayer, 0, 1, 84, 48);
			time_count = 0;
			Player.Rwin++;
			break;
		}
		else if (time_count > reaction_time){
			win = 0;
			wonGame = 0x02;
			state = RDone;
			lcd_clear_space(1, 5);
			printPictureAtPos(cowboyEnemy, 0, 1, 84, 48);
			time_count = 0;
			break;
		}
		else time_count++;
		break;

		case RDone:
		time_count++;
		if (time_count == 1) win = 0;
		if (time_count == 10 && wonGame == 0x01){
			lcd_clear_space(1, 5);
			printPictureAtPos(completed, 0, 1, 84, 30);
		} 
		else if (time_count == 10 && wonGame == 0x02){
			lcd_clear_space(1, 5);
			printPictureAtPos(wrong, 0, 1, 84, 30);
		}
		if (time_count == 20){
			win = 0;
			state = RStart;
			gamesArray[game_index]->done = 1;
			time_count = 0;
			break;
		}
		break;
	}
	return state;
};
/********************Escape the Maze********************/
/*
Objective: Get to the G
Difficulty: The players will (hopefully) have a more difficult time getting to the G with the second map
One player controls left and right movement and the other player controls up and down.
*/
//Generate more maps if time allows
char map1[5][14] = {
	//	  0   1   2   3   4   5   6   7   8   9  10   11  12  13
	{'x','x','x','x','x','x','x','x','x','x','x','x','x','x'}, //0
	{' ',' ',' ','x','x','x',' ',' ',' ','x',' ',' ',' ','x'}, //1
	{'x','x',' ','x',' ',' ',' ','x',' ',' ',' ',' ',' ','x'}, //2
	{'x','x',' ',' ',' ','x','x','x',' ',' ','x','x',' ','x'}, //3
	{'x','x','x','x','x','x','x','x','x','x','x','x','G','x'} //4
};

char map2[5][14] = {
	//0   1   2   3   4   5   6   7   8   9  10   11  12  13
	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x','x','x'}, //0
	{' ',' ','x','x',' ','x','x','x','x','x',' ','x','x',' '}, //1
	{'x','x',' ',' ',' ',' ',' ',' ',' ','x',' ',' ',' ',' '}, //2
	{' ',' ',' ','x','x',' ',' ',' ',' ','x','x','x','x',' '}, //3
	{' ',' ',' ',' ',' ',' ','x','x',' ',' ',' ',' ','G','x'} //4
};

char map3[5][14] = {
	//0   1   2   3   4   5   6   7   8   9  10   11  12  13
	{' ',' ',' ',' ',' ','x','x','x',' ',' ',' ',' ',' ',' '}, //0
	{' ','x',' ','x',' ','x',' ',' ',' ','x','x',' ',' ',' '}, //1
	{'x',' ',' ',' ',' ','x',' ',' ','x',' ',' ',' ','x',' '}, //2
	{' ',' ','x',' ','x','x',' ',' ',' ',' ','x',' ','x',' '}, //3
	{'x',' ',' ',' ',' ',' ',' ','x','x','x',' ','x','G',' '} //4
};
void generate_map(unsigned char phase){
	//space means good
	//x means bomb
	//_ and ! means walls
	if (phase == 0x01){
		for (int i = 0; i < 5; i++){
			for(int j = 0; j < 14; j++){
				lcd_goto_xy(j, i+1);
				lcd_chr(map1[i][j]);
			}
		}
		
	}
	else if (phase == 0x02){
		for (int i = 0; i < 5; i++){
			for(int j = 0; j < 14; j++){
				lcd_goto_xy(j, i+1);
				lcd_chr(map2[i][j]);
			}
		}
	}
	else if (phase == 0x03){
		for (int i = 0; i < 5; i++){
			for(int j = 0; j < 14; j++){
				lcd_goto_xy(j, i+1);
				lcd_chr(map3[i][j]);
			}
		}
	}
}

unsigned char getCharAtPos(unsigned char phase, unsigned char x, unsigned char y){
	if (phase == 0x01) return map1[x][y];
	else if (phase == 0x02) return map2[x][y];
	else if (phase == 0x03) return map3[x][y];
	return 'q';
}

//1 is left, 0 is right, 2 is up, 3 is down
enum Maze{MStart, MWait, MInstructions, MPlay, MDone};
int MazeTick(int state){
	static unsigned char player_x;
	static unsigned char player_y;
	static unsigned short time_count;
	unsigned char joystick_input;
	static unsigned char answer;
	static unsigned char buttonPress;
	unsigned char otherPlayer; //0x04 left, 0x05 right, 0x06 up, 0x07 down from 0
	switch(state){
		case MStart:
		player_x = 0;
		player_y = 2;
		time_count = 0;
		joystick_input = 0;
		answer = 0;
		buttonPress = 0;
		state = MWait;
		break;

		case MWait:
		if (interrupt && game_on && game_index == 4)
		{
			player_x = 0;
			player_y = 2;
			time_count = 0;
			state = MInstructions;
			otherPlayer = 0;
			joystick_input = 0;
			buttonPress = 0;
			lcd_goto_xy(0,1);
			//lcd_string_format("Exit the maze You can only\nmove left &\nright. Go to  the G. Avoid X");/********Microcontroller 1********/
			lcd_string_format("Exit the maze You can only\nmove up & downGo to the G\nAvoid X");/********Microcontroller 2********/
			Player.totalM++;
		}
		break;

		case MInstructions:
		if (time_count == 60){
			state = MPlay;
			time_count = 0;
			lcd_clear_space(1, 5);
			generate_map(phase);
			lcd_goto_xy(player_x, player_y);
			lcd_chr('0');
		}
		else time_count++;
		break;

		case MPlay:
		//Get movement from other player if taken
		if (USART_HasReceived(0)){
			otherPlayer = USART_Receive(0);
			USART_Flush(0);
			lcd_goto_xy(player_x,player_y);
			lcd_chr(' ');
			//player_y = otherPlayer; /********Microcontroller 1********/
			player_x = otherPlayer; /********Microcontroller 2********/
			otherPlayer = 0;
			lcd_goto_xy(player_x,player_y);
			lcd_chr('0');
		}
		//If the other player stepped on an X
		if (USART_HasReceived(1)){
			otherPlayer = USART_Receive(1);
			USART_Flush(1);
			if (otherPlayer == 0x01){
				state = MDone;
				time_count = 0;
				lcd_clear_space(1, 5);
				printPictureAtPos(wrong, 0, 1, 84, 30);
				win = 0;
				break;
			}

		}
		if (time_count >= 300){ //30 seconds to get out of the maze
			state = MDone;
			time_count = 0;
			lcd_clear_space(1, 5);
			printPictureAtPos(wrong, 0, 1, 84, 30);
			win = 0;
			break;
		}
		else time_count++;

		//Get input from player
		joystick_input = checkInput();
		/********Microcontroller 1********/
		/*
		if (joystick_input == 0 || joystick_input == 1){
			printArrow(joystick_input, 0, 5);
			answer = joystick_input;
		}
		if (GetBit(PINC,3)==0 && buttonPress == 0 && joystick_input != '\0') {
			buttonPress = 1;
			lcd_goto_xy(0, 5);
			lcd_chr(' ');
			lcd_goto_xy(player_x,player_y);
			lcd_chr(' ');
			if (answer == 1 && player_x != 0) player_x--;
			else if (answer == 0 && player_x != 13) player_x++;
			lcd_goto_xy(player_x,player_y);
			lcd_chr('0');

			///Update the position of the other player
			if (USART_IsSendReady(0)) USART_Send(player_x, 0);
		}
		else if (GetBit(PINC,3)==1) buttonPress = 0;
		//*/

		/*********Microcontroller 2********/
		
		if (joystick_input == 2 || joystick_input == 3){
			printArrow(joystick_input, 0, 5);
			answer = joystick_input;
		}
		if (GetBit(PINC,3)==0 && buttonPress == 0 && joystick_input != '\0') {
			buttonPress = 1;
			lcd_goto_xy(0, 5);
			lcd_chr(' ');
			lcd_goto_xy(player_x,player_y);
			lcd_chr(' ');
			if (answer == 2 && player_y != 1) player_y--;
			else if (answer == 3 && player_y != 5) player_y++;
			lcd_goto_xy(player_x,player_y);
			lcd_chr('0');

			///Update the position of the other player
			if (USART_IsSendReady(0)) USART_Send(player_y, 0);
		}
		else if (GetBit(PINC,3)==1) buttonPress = 0;
	

		//*/
		if (getCharAtPos(phase, player_y - 1, player_x) == 'G'){
			state = MDone;
			time_count = 0;
			lcd_clear_space(1, 5);
			printPictureAtPos(completed, 0, 1, 84, 30);
			win = 1;
			Player.Mwin++;
			break;
		}
		if (getCharAtPos(phase, player_y - 1, player_x) == 'x'){
			state = MDone;
			time_count = 0;
			lcd_clear_space(1, 5);
			printPictureAtPos(wrong, 0, 1, 84, 30);
			win = 0;
			if (USART_IsSendReady(1)) USART_Send(0x01, 1);
			break;
		}
		
		break;

		case MDone:
		time_count++;
		if (time_count == 1) win = 0;
		if (time_count == 10){
			win = 0;
			state = MStart;
			gamesArray[game_index]->done = 1;
			time_count = 0;
			break;
		}
		break;
	}
	return state;
};