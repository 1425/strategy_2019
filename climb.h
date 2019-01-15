#ifndef CLIMB_H
#define CLIMB_H

#include<iosfwd>
#include<vector>
#include<optional>

enum class Climb_type{P3,P6,P12};

std::ostream& operator<<(std::ostream&,Climb_type);

Climb_type rand(const Climb_type*);
Climb_type parse(const Climb_type*,std::string);
std::vector<Climb_type> climb_types();
int points(Climb_type);

using Climb_result=std::optional<Climb_type>;

std::vector<Climb_result> climb_results();
int points(Climb_result);

Climb_result parse(const Climb_result*,std::string s);

#define CLIMB_ODDS_ITEMS(X) \
	X(l1) X(l2) X(l3)

struct Climb_odds{
	float l1,l2,l3; //0-1 (if no data, then this should come out as 0 instead of NaN)
};

bool operator<(Climb_odds,Climb_odds);

Climb_odds& operator+=(Climb_odds&,Climb_odds);
Climb_odds& operator/=(Climb_odds&,float);
std::ostream& operator<<(std::ostream&,Climb_odds const&);
Climb_odds rand(const Climb_odds*);

float odds(Climb_odds,Climb_type);
Climb_type max_odds_type(Climb_odds);

struct Climb_situation{
	//l1 is assumed to always be available.
	bool l2_available,l3_available;
};

std::ostream& operator<<(std::ostream&,Climb_situation const&);
bool operator<(Climb_situation,Climb_situation);

std::vector<Climb_situation> climb_situations();
std::vector<Climb_situation> demangle_climb_result(std::vector<Climb_result>);

#endif
