enum Game_Handler {Start, INTRO, OFF, ON, PACT, AWAIT_PACT, INSTRUCTIONS, ENDGAME};
int Game_Handler_Tick(int state){
	static unsigned char instruction_index;
	unsigned char keyPress = 0;
	unsigned char startGame = 0;
	unsigned char response = 0;
	if (state == OFF || state == INSTRUCTIONS)
	{
		if (USART_HasReceived(0)){
			startGame = USART_Receive(0);
			USART_Flush(0);
		}
		if (startGame == 0xFF){
			_delay_ms(100);
			lcd_clear();
			lcd_string_format("Accept the \npact?\nA)Yes\nB)No");
			state = PACT;
		}
	}
	//Check inputs from the keypad
	PORTC = 0xEF; // Enable col 4 with 0, disable others with 1?s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINC,3)==0) { keyPress = 'A'; }
	if (GetBit(PINC,2)==0) { keyPress = 'B'; }
	if (GetBit(PINC,1)==0) { keyPress = 'C'; }
	switch(state){

		case Start:
		state = INTRO;
		_delay_ms(100);
		lcd_clear();
		printPictureOnLCD(Link);
		break;

		case INTRO:
		switch(keyPress){
			case 'A':
			state = OFF;
			start = 0;
			instruction_index = 0;
			end = 0;
			_delay_ms(100);
			lcd_clear();
			lcd_string_format("Linked Menu\nA)Game Start\nB)Instructions");
			break;
		}
		break;

		case PACT:
		switch(keyPress){
			case 'A':
			state = ON;
			start = 1;
			if (USART_IsSendReady(0)){
				USART_Send(0x01, 0);
			}
			lcd_clear();
			break;
			case 'B':
			state = OFF;
			if (USART_IsSendReady(0)){
				USART_Send(0x02, 0);
			}
			_delay_ms(100);
			lcd_clear();
			lcd_string_format("Linked Menu\nA)Game Start\nB)Instructions");
			break;
		}
		break;

		case AWAIT_PACT:
		if (USART_HasReceived(0)){
			response = USART_Receive(0);
			USART_Flush(0);
		}
		if (response == 0x01){
			state = ON;
			start = 1;
			_delay_ms(100);
			lcd_clear();
		}
		else if (response == 0x02) {
			state = OFF;
			_delay_ms(100);
			lcd_clear();
			lcd_string_format("Linked Menu\nA)Game Start\nB)Instructions");
		}
		break;

		case OFF:
		switch (keyPress)
		{
			case 'A':
			state = AWAIT_PACT;
			if (USART_IsSendReady(0)){
				USART_Send(0xFF, 0);
			}
			_delay_ms(100);
			lcd_clear();
			lcd_string_format("Waiting for\nother player'sresponse");
			break;

			case 'B':
			state = INSTRUCTIONS;
			instruction_index = 0;
			//push into queue of output instructions but for now
			_delay_ms(100);
			lcd_clear();
			lcd_string_format(instructions[instruction_index]);
			
			break;
		}
		break;

		case ON:
		if (end){
			state = ENDGAME;
			end = 0;
			start = 0; //possibly problem with placements
		}
		else start = 1;
		break;

		case INSTRUCTIONS:
		switch (keyPress)
		{
			case 'A':
			if (instruction_index + 1 < 4){
				_delay_ms(100);
				lcd_clear();
				instruction_index++;
				lcd_string_format(instructions[instruction_index]);
			}
			break;

			case 'B':
			if (instruction_index - 1 >= 0 || instruction_index != 0){
				_delay_ms(100);
				lcd_clear();
				instruction_index--;
				lcd_string_format(instructions[instruction_index]);
			}
			break;

			case 'C':
			state = OFF;
			_delay_ms(100);
			lcd_clear();
			lcd_string_format("Linked Menu\nA)Game Start\nB)Instructions");
			break;
		}
		break;

		case ENDGAME:
		if (keyPress == 'C'){
			state = OFF;
			start = 0;
			instruction_index = 0;
			end = 0;
			_delay_ms(100);
			lcd_clear();
			lcd_string_format("Linked Menu\nA)Game Start\nB)Instructions");
		}
		break;
	}
	return state;
};
