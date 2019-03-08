#ifndef DATA_H
#define DATA_H

#include<vector>
#include "climb.h"

using Team=int;//official season; all the teams will have real numbers.
using Match=int;//initially, assume that these are all qual match numbers.

enum class Alliance{RED,BLUE};

std::ostream& operator<<(std::ostream&,Alliance);

class Climb_assist{
	//0-3
	int i;

	public:
	explicit Climb_assist(int);

	int get()const;
};

bool operator!=(Climb_assist,Climb_assist);
std::ostream& operator<<(std::ostream&,Climb_assist);
Climb_assist rand(const Climb_assist*);
Climb_assist parse(const Climb_assist*,std::string const&);

#define ROBOT_MATCH_DATA_ITEMS(X)\
	X(Team,team,0)\
	X(Match,match,0)\
	X(Alliance,alliance,{})\
	X(bool,shelf,0)\
	X(unsigned,balls,0)\
	X(unsigned,hatches,0)\
	X(Climb_result,climb,{})\
	X(Climb_assist,climb_assist_a,Climb_assist(0))\
	X(Climb_assist,climb_assist_b,Climb_assist(0))\
	X(bool,climb_was_assisted,0)

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
Scouting_data read_data(std::string const&);
void check_consistency(Scouting_data const&);

#endif
