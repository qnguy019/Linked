
enum updateHealth{UHStart, UHWait, UHOn};
int updateHealthTick(int state){
	unsigned char health1;
	unsigned char health2;
	static unsigned char decrease_count;
	static unsigned char decrement;
	unsigned char receiveHealth = 0;
	static unsigned char player_dead;
	switch (state){
		case UHStart:
		state = UHWait;
		health1 = 0;
		health2 = 0;
		decrease_count = 0;
		decrement = 0;
		player_dead = 0;
		break;
		
		case UHWait:
		if (game_on){
			state = UHOn;
			decrease_count = 0;
			decrement = 50;
			player_dead = 0;
		}
		break;

		case UHOn:
		if (!game_on)
		{
			state = UHStart;
		}
		else if (game_on){
			if (player_dead) decrement = 10;
			else if (phase == 0x01) decrement = 50;
			else if (phase == 0x02) decrement = 45;
			else if (phase == 0x03) decrement = 40;
			if (decrease_count >= decrement){
				health = health >> 1;
				decrease_count = 0;
			}
			else decrease_count++;
			//Send health if won

			if (win && USART_IsSendReady(0)){
				USART_Send(0x33, 0);
			}
			//Get health if won from other player
			if (USART_HasReceived(0)){
				receiveHealth = USART_Receive(0);
				USART_Flush(0);
			}
			if (receiveHealth == 0x33){
				health = health << 4;
				health |= 0x0F;
				receiveHealth = 0;
				if (health > 0x03FF) health = 0x03FF;
			}
			else if  (receiveHealth == 0x55){
				player_dead = 1;
			}
			//output the health
			health1 = (char)health;
			health2 = (char)(health >> 8);
			transmit_data(health1);
			if (GetBit(health2, 0) == 1) PORTA |= (0x01 << 6);
			else SetBit(PORTA, 6, 0);
			if (GetBit(health2, 1) == 1) PORTA |= (0x01 << 7);
			else SetBit(PORTA, 7, 0);
			if (health == 0){
				game_on = 0;
				outputEndGame();
				if (USART_IsSendReady(0)) USART_Send(0x55, 0);
			}
		break;
		}
	}
	return state;
};