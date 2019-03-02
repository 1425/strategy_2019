#include "climb2.h"
#include<array>
#include "climb.h"
#include "util.h"
#include "data.h"

using namespace std;

enum class Helpee_result{NOT_HELPED,L2,L3};

struct Climb_data_robot{
	Climb_result self;
	std::array<Helpee_result,2> helpees;
	bool was_helped;
};

using Climb_data_alliance=std::vector<Climb_data_robot>; //usually of length 3.

//enum class Climb_action{NONE,L1_SELF,L2_SELF,L3_SELF,L2_HELPEE,L3_HELPEE};
#define CLIMB_ACTION_ITEMS(X)\
	X(NONE) X(L1_SELF) X(L2_SELF) X(L3_SELF) X(L2_HELPEE) X(L3_HELPEE)

enum class Climb_action{
	#define X(A) A,
	CLIMB_ACTION_ITEMS(X)
	#undef X
};

std::ostream& operator<<(std::ostream& o,Climb_action a){
	#define X(A) if(a==Climb_action::A) return o<<""#A;
	CLIMB_ACTION_ITEMS(X)
	#undef X
	assert(0);
}

#define HELP_GIVEN_ITEMS(X) \
	X(NONE) X(L2) X(L3) X(L22) X(L23) X(L33)
enum class Help_given{
	#define X(A) A,
	HELP_GIVEN_ITEMS(X)
	#undef X
};

std::ostream& operator<<(std::ostream& o,Help_given a){
	#define X(A) if(a==Help_given::A) return o<<""#A;
	HELP_GIVEN_ITEMS(X)
	#undef X
	assert(0);
}

#define CLIMB_RESULT_ROBOT_ITEMS(X)\
	X(Climb_action,climb_action)\
	X(Help_given,help_given)

struct Climb_result_robot{
	CLIMB_RESULT_ROBOT_ITEMS(INST)
};

std::ostream& operator<<(std::ostream& o,Climb_result_robot const& a){
	o<<"Climb_result_robot( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	CLIMB_RESULT_ROBOT_ITEMS(X)
	#undef X
	return o<<")";
}

using Partner_count=int;//0-2

#define CLIMB2_SITUATION_ITEMS(X)\
	X(bool,l2_available)\
	X(bool,l3_available)\
	X(Partner_count,partners_below_l3)\
	X(Partner_count,partners_below_l2)
	
struct Climb2_situation{
	CLIMB2_SITUATION_ITEMS(INST)
};

ostream& operator<<(std::ostream& o,Climb2_situation const& a){
	o<<"Climb2_situation( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	CLIMB2_SITUATION_ITEMS(X)
	#undef X
	return o<<")";
}

struct Climb_capabilities{
	map<Climb_action,float> self;
	map<Help_given,float> help_given;
};

std::ostream& operator<<(std::ostream& ,Climb_capabilities const& ){
	nyi
}

template<typename T,size_t N>
vector<T> without_index(size_t i,std::array<T,N> const& a){
	vector<T> r;
	for(auto j:range(N)){
		if(j!=i){
			r|=a[i];
		}
	}
	return r;
}

template<typename T>
vector<T> without_index(size_t i,std::vector<T> const& a){
	vector<T> r;
	for(auto j:range(a.size())){
		if(j!=i){
			r|=a[i];
		}
	}
	return r;
}

std::array<pair<Climb_result_robot,Climb2_situation>,3> demangle(Climb_data_alliance const& a){
	std::array<pair<Climb_result_robot,Climb2_situation>,3> r;
	for(auto i:range(a.size())){
		auto this_robot=a[i];
		auto others=without_index(i,a);
		Climb2_situation cs;
		cs.l2_available=filter([](auto x){ return x.self==Climb_type::P6; },others).size()<2;
		cs.l3_available=filter([](auto x){ return x.self==Climb_type::P12; },others).size()<1;
		cs.partners_below_l2=filter(
			[](auto x){ return x.self==Climb_result{} || x.self==Climb_type::P3; },
			others
		).size();
		cs.partners_below_l3=filter(
			[](auto x){ return x.self==Climb_result{} || x.self==Climb_type::P3 || x.self==Climb_type::P6; },
			others
		).size();
		Climb_result_robot climb_result_robot{
			[&](){
				if(this_robot.self){
					if(this_robot.was_helped){
						switch(*this_robot.self){
							case Climb_type::P6:
								return Climb_action::L2_HELPEE;
							case Climb_type::P12:
								return Climb_action::L3_HELPEE;
							default: assert(0);
						}
					}else{
						switch(*this_robot.self){
							case Climb_type::P3:
								return Climb_action::L1_SELF;
							case Climb_type::P6:
								return Climb_action::L2_SELF;
							case Climb_type::P12:
								return Climb_action::L3_SELF;
							default: assert(0);
						}
					}
				}
				return Climb_action::NONE;
			}(),
			[&](){
			
				//enum class Helpee_result{NOT_HELPED,L2,L3};
				//enum class Help_given{NONE,L2,L3,L22,L23,L33};
				switch(this_robot.helpees[0]){
					case Helpee_result::NOT_HELPED:
						switch(this_robot.helpees[1]){
							case Helpee_result::NOT_HELPED:
								return Help_given::NONE;
							case Helpee_result::L2:	
								return Help_given::L2;
							case Helpee_result::L3:
								return Help_given::L3;
							default: assert(0);
						}
					case Helpee_result::L2:
						switch(this_robot.helpees[1]){
							case Helpee_result::NOT_HELPED:
								return Help_given::L2;
							case Helpee_result::L2:	
								return Help_given::L22;
							case Helpee_result::L3:
								return Help_given::L23;
							default: assert(0);
						}
					case Helpee_result::L3:
						switch(this_robot.helpees[1]){
							case Helpee_result::NOT_HELPED:
								return Help_given::L3;
							case Helpee_result::L2:	
								return Help_given::L23;
							case Helpee_result::L3:
								return Help_given::L33;
							default: assert(0);
						}
					default: assert(0);
				}
			}()
		};
		r[i]=make_pair(climb_result_robot,cs);
	}
	return r;
}

Climb_data_robot climb_data_robot(Robot_match_data a){
	
/*	truct Climb_data_robot{
        Climb_result self;
        std::array<Helpee_result,2> helpees;
        bool was_helped;
};*/
	return Climb_data_robot{
		a.climb,
		//TODO: These next two lines need to change.
		{Helpee_result::NOT_HELPED,Helpee_result::NOT_HELPED},
		0
	};
}

map<Team,Climb_capabilities> analyze_climb(Scouting_data d){
	map<Team,pair<Climb_result_robot,Climb2_situation>> by_team;
	//vector<Climb_data_alliance>)nyi
	for(auto data:values(group(alliance_id,d))){
		Climb_data_alliance cd=mapf(climb_data_robot,data);
		auto dm=demangle(cd);
		for(auto [data_point,situation]:zip(data,dm)){
			PRINT(data_point);
			PRINT(situation);
			//by_team[data_point.team]|=situation;
		}
	}

	print_r(by_team);

	map<Team,Climb_capabilities> r;
	for(auto [team,data]:by_team){
		PRINT(team);
		PRINT(data);
		nyi
	}
	return r;
}

int main(){
	Scouting_data d=example_input();
	PRINT(analyze_climb(d));
}
