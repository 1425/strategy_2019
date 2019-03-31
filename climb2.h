#ifndef CLIMB2_H
#define CLIMB2_H

#include<bitset>
#include "data.h"
#include "util.h"

#define SINGLE_ARG(A,B) A,B

#define CLIMB_ACTION_ITEMS(X)\
	X(NONE) X(L1_SELF) X(L2_SELF) X(L3_SELF) X(L2_HELPEE) X(L3_HELPEE)

enum class Climb_action{
	#define X(A) A,
	CLIMB_ACTION_ITEMS(X)
	#undef X
};

std::ostream& operator<<(std::ostream&,Climb_action);

std::vector<Climb_action> climb_actions();

#define HELP_GIVEN_ITEMS(X) \
	X(NONE) X(L2) X(L3) X(L22) X(L23) X(L33)
enum class Help_given{
	#define X(A) A,
	HELP_GIVEN_ITEMS(X)
	#undef X
};

std::ostream& operator<<(std::ostream&,Help_given);
std::vector<Help_given> help_givens();

//using Self_map=std::map<Climb_action,float>;
using Self_map=Flat_map<Climb_action,6,float>;
//using Help_map=std::map<Help_given,float>;
using Help_map=Flat_map<Help_given,6,float>;

#define CLIMB_CAPABILITIES_ITEMS(X)\
	X(Self_map,self)\
	X(Help_map,help_given)
struct Climb_capabilities{
	CLIMB_CAPABILITIES_ITEMS(INST)
};

std::ostream& operator<<(std::ostream&,Climb_capabilities const&);
bool operator<(Climb_capabilities const&,Climb_capabilities const&);
Climb_capabilities& operator/=(Climb_capabilities&,float);
Climb_capabilities& operator+=(Climb_capabilities&,Climb_capabilities const&);
Climb_capabilities rand(const Climb_capabilities*);

std::map<Team,Climb_capabilities> analyze_climb(Scouting_data const&);

#endif
