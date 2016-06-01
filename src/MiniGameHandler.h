
typedef struct miniGames{
	signed char state;
	int(*GameFct)(int);
	unsigned char done;

} MG;

static MG SIN_Game, SS_Game, SM_Game, R_Game, M_Game;
MG *gamesArray[] = {&SIN_Game, &SS_Game, &SM_Game, &R_Game, &M_Game};
unsigned char numGames = 5;
static unsigned game_index;
enum MiniGameController {MGCStart, MGCWait, MGCOn};
int MGC_Tick(int state){
	switch (state){
		case MGCStart:
		state = MGCWait;
		game_index = 0;
		initPlayer();
		break;

		case MGCWait:
		if (!game_on) state = MGCWait;
		else if (game_on){
			state = MGCOn;
			game_index = rand() % 4;
			initPlayer();
		}
		break;

		case MGCOn:
		if (game_on){
			if(interrupt)
			{
				game_index = 4;
				gamesArray[game_index]->state = gamesArray[game_index]->GameFct(gamesArray[game_index]->state);
			}
			else if (!interrupt)
			{
				gamesArray[game_index]->state = gamesArray[game_index]->GameFct(gamesArray[game_index]->state);
			}
			if (gamesArray[game_index]->done == 1){
				gamesArray[game_index]->done = 0;
				unsigned char temp_index = rand() % 4;
				while (temp_index == game_index) temp_index = rand() % 4; //so the player doesn't play the same game twice in a row
				game_index = temp_index;
				interrupt = 0;
			}		
		}
		else if (!game_on) state = MGCStart;
		break;
	}
	return state;
};