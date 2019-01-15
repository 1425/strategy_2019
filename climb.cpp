#include "climb.h"
#include "util.h"

using namespace std;

std::ostream& operator<<(std::ostream& o,Climb_type a){
	switch(a){
		case Climb_type::P3:
			return o<<"P3";
		case Climb_type::P6:
			return o<<"P6";
		case Climb_type::P12:
			return o<<"P12";
		default:
			assert(0);
	}
}

Climb_type rand(const Climb_type*){
	switch(rand()%3){
		case 0: return Climb_type::P3;
		case 1: return Climb_type::P6;
		case 2: return Climb_type::P12;
		default: assert(0);
	}
}

Climb_type parse(const Climb_type*,std::string s){
	if(s=="P3") return Climb_type::P3;
	if(s=="P6") return Climb_type::P6;
	if(s=="P12") return Climb_type::P12;
	assert(0);
}

vector<Climb_type> climb_types(){
	return {
		Climb_type::P3,
		Climb_type::P6,
		Climb_type::P12
	};
}

int points(Climb_type a){
	switch(a){
		case Climb_type::P3: return 3;
		case Climb_type::P6: return 6;
		case Climb_type::P12: return 12;
		default: assert(0);
	}
}

bool operator<(Climb_odds a,Climb_odds b){
	#define X(A) if(a.A<b.A) return 1; if(b.A<a.A) return 0;
	CLIMB_ODDS_ITEMS(X)
	#undef X
	return 0;
}

Climb_odds& operator+=(Climb_odds& a,Climb_odds b){
	#define X(A) a.A+=b.A;
	CLIMB_ODDS_ITEMS(X)
	#undef X
	return a;
}

Climb_odds& operator/=(Climb_odds& a,float f){
	#define X(A) a.A/=f;
	CLIMB_ODDS_ITEMS(X)
	#undef X
	return a;
}

std::ostream& operator<<(std::ostream& o,Climb_odds const& a){
	o<<"Climb_odds(";
	o<<"l1:"<<a.l1<<" ";
	o<<"l2:"<<a.l2<<" ";
	o<<"l3:"<<a.l3;
	return o<<")";
}

Climb_odds rand(const Climb_odds*)nyi

using Climb_result=std::optional<Climb_type>;

vector<Climb_result> climb_results(){
	vector<Climb_result> r;
	r|=climb_types();
	r|=Climb_result{};
	return r;
}

int points(Climb_result a){
	if(!a){ return 0; }
	return points(*a);
}

Climb_result parse(const Climb_result*,std::string s){
	if(s=="NULL"){
		//Not sure why the first line was generating a warning at high optimization levels.
		//return Climb_result{};
		Climb_result r;
		return r;
	}
	return parse((Climb_type*)nullptr,s);
}

float odds(Climb_odds odds,Climb_type type){
	switch(type){
		case Climb_type::P3:
			return odds.l1;
		case Climb_type::P6:
			return odds.l2;
		case Climb_type::P12:
			return odds.l3;
		default:
			assert(0);
	}
}

std::ostream& operator<<(std::ostream& o,Climb_situation const& a){
	o<<"Climb_situation(l2:"<<a.l2_available<<",l3:"<<a.l3_available<<")";
	return o;
}

bool operator<(Climb_situation a,Climb_situation b){
	if(a.l2_available<b.l2_available) return 1;
	if(b.l2_available<a.l2_available) return 0;
	return a.l3_available<b.l3_available;
}

vector<Climb_situation> climb_situations(){
	return mapf(
		[](auto p){
			return Climb_situation{p.first,p.second};
		},
		cross(bools(),bools())
	);
}

vector<Climb_situation> demangle_climb_result(vector<Climb_result> in){
	//initially not worrying about buddy climbs
	auto l2_used=filter([](auto x){ return x==Climb_type::P6; },in).size();
	auto l3_used=filter([](auto x){ return x==Climb_type::P12; },in).size();
	return mapf(
		[=](Climb_result x){
			return Climb_situation{
				l2_used<2 || x==Climb_type::P6,
				l3_used<1 || x==Climb_type::P12
			};
		},
		in
	);
}

Climb_type max_odds_type(Climb_odds a){
	if(a.l1>a.l2 && a.l3) return Climb_type::P3;
	if(a.l2>a.l3) return Climb_type::P6;
	return Climb_type::P12;
}

