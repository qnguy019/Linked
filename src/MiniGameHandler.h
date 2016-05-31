
typedef struct miniGames{
	signed char state;
	int(*GameFct)(int);
	unsigned char done;

} MG;
// static MG SIN_Game, SS_Game;
// MG *gamesArray[] = {&SIN_Game, &SS_Game};
// unsigned char numGames = 2;
static MG M_Game;
MG *gamesArray[] = {&M_Game};
unsigned char numGames = 1;
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
			game_index = 0;
		}
		break;

		case MGCOn:
		if (game_on){
			if(interrupt)
			{
				gamesArray[0]->state = gamesArray[0]->GameFct(gamesArray[0]->state);
			}
			else if (!interrupt)
			{
				gamesArray[game_index]->state = gamesArray[game_index]->GameFct(gamesArray[game_index]->state);
			}
			if (gamesArray[game_index]->done == 1){
				gamesArray[game_index]->done = 0;
				game_index = rand() % 2;
				interrupt = 1;
// 				if (game_index + 1 == numGames) game_index = 0;
// 				else game_index++;
			}
			
		}
		else if (!game_on) state = MGCStart;
		break;
	}
	return state;
};