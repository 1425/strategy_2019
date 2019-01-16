#ifndef DATA_H
#define DATA_H

#include<vector>
#include "climb.h"

using Team=int;//official season; all the teams will have real numbers.
using Match=int;//initially, assume that these are all qual match numbers.

enum class Alliance{RED,BLUE};

std::ostream& operator<<(std::ostream&,Alliance);

#define ROBOT_MATCH_DATA_ITEMS(X)\
	X(Team,team,0)\
	X(Match,match,0)\
	X(Alliance,alliance,{})\
	X(bool,shelf,0)\
	X(unsigned,balls,0)\
	X(unsigned,hatches,0)\
	X(Climb_result,climb,{})\
	X(Climb_result,climb_buddy_lower,{})\
	X(Climb_result,climb_buddy_higher,{})

struct Robot_match_data{
	#define X(A,B,C) A B=C;
	ROBOT_MATCH_DATA_ITEMS(X)
	#undef X
};

std::ostream& operator<<(std::ostream&,Robot_match_data const&);
bool operator==(Robot_match_data const&,Robot_match_data const&);

std::pair<Match,Alliance> alliance_id(Robot_match_data);
Climb_result climb_result(Robot_match_data);

using Scouting_data=std::vector<Robot_match_data>;

Scouting_data example_input();
void csv_test();

#endif