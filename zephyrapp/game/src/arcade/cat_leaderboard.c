#include "cat_leaderboard.h"

#include <string.h>
#include "cat_core.h"
#include "cat_air.h"

int snake_highscore = 0;
int mines_highscore = 0;
int foursquares_highscore = 0;

CAT_stroop_data stroop_data = {0};
bool stroop_data_valid = false;
float stroop_correctness = 0;

uint8_t survey_field;

