#include "Gameflow.h"

unsigned int GameState;

void SetGameState(unsigned int state)
{
	GameState = state;
}
unsigned int GetGameState()
{
	return GameState;
}
bool ForGameState(unsigned int state)
{
	if (GameState == state)
		return true;
	else
		return false;
}