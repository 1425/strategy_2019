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

std::ostream& operator<<(std::ostream& o,Climb_action a){
	#define X(A) if(a==Climb_action::A) return o<<""#A;
	CLIMB_ACTION_ITEMS(X)
	#undef X
	assert(0);
}

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

bool operator<(Climb_result_robot const& a,Climb_result_robot const& b){
	#define X(A,B) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	CLIMB_RESULT_ROBOT_ITEMS(X)
	#undef X
	return 0;
}

std::ostream& operator<<(std::ostream& o,Climb_result_robot const& a){
	o<<"Climb_result_robot( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	CLIMB_RESULT_ROBOT_ITEMS(X)
	#undef X
	return o<<")";
}

using Partner_count=int;//0-2

#define CLIMB2_SITUATION_ITEMS(X)\
	X(int,l2_available)\
	X(bool,l3_available)\
	X(Partner_count,partners_below_l3)\
	X(Partner_count,partners_below_l2)
	
struct Climb2_situation{
	CLIMB2_SITUATION_ITEMS(INST)
};

bool operator<(Climb2_situation const& a,Climb2_situation const& b){
	#define X(A,B) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	CLIMB2_SITUATION_ITEMS(X)
	#undef X
	return 0;
}

ostream& operator<<(std::ostream& o,Climb2_situation const& a){
	o<<"Climb2_situation( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	CLIMB2_SITUATION_ITEMS(X)
	#undef X
	return o<<")";
}

bool operator<(Climb_capabilities const& a,Climb_capabilities const& b){
	#define X(A,B) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	CLIMB_CAPABILITIES_ITEMS(X)
	#undef X
	return 0;
}

template<typename K,typename V>
map<K,V>& operator+=(map<K,V>& a,map<K,V> const& b){
	for(auto [k,v]:b){
		auto f=a.find(k);
		if(f==a.end()){
			a[k]=v;
		}else{
			f->second+=v;
		}
	}
	return a;
}

Climb_capabilities& operator+=(Climb_capabilities& a,Climb_capabilities const& b){
	#define X(A,B) a.B+=b.B;
	CLIMB_CAPABILITIES_ITEMS(X)
	#undef X
	return a;
}

template<typename K>
map<K,float>& operator/=(map<K,float>& a,float f){
	for(auto &p:a){
		p.second/=f;
	}
	return a;
}

Climb_capabilities& operator/=(Climb_capabilities& a,float f){
	#define X(A,B) a.B/=f;
	CLIMB_CAPABILITIES_ITEMS(X)
	#undef X
	return a;
}

Climb_capabilities rand(const Climb_capabilities*){
	nyi
}

std::ostream& operator<<(std::ostream& o,Climb_capabilities const& a){
	o<<"Climb_capabilities( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	CLIMB_CAPABILITIES_ITEMS(X)
	#undef X
	return o<<")";
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

vector<pair<Climb_result_robot,Climb2_situation>> demangle(Climb_data_alliance const& a){
	std::vector<pair<Climb_result_robot,Climb2_situation>> r;
	for(auto i:range(a.size())){
		auto this_robot=a.at(i);
		auto others=without_index(i,a);
		Climb2_situation cs;
		cs.l2_available=2-filter([](auto x){ return x.self==Climb_type::P6; },others).size();
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
							case Climb_type::P3:
								//being helped to level 1 will be ignored.
								return Climb_action::L1_SELF;
							case Climb_type::P6:
								return Climb_action::L2_HELPEE;
							case Climb_type::P12:
								return Climb_action::L3_HELPEE;
							default:
								PRINT(this_robot.self);
								assert(0);
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
		r|=make_pair(climb_result_robot,cs);
	}
	return r;
}

Helpee_result to_helpee_result(Climb_assist const& a){
	switch(a.get()){
		case 0: return Helpee_result::NOT_HELPED;
		case 1: return Helpee_result::NOT_HELPED; //if you are marked as helping someone to level 1 this counts for nothing.
		case 2: return Helpee_result::L2;
		case 3: return Helpee_result::L3;
		default: assert(0);
	}
}

Climb_data_robot climb_data_robot(Robot_match_data a){
	return Climb_data_robot{
		a.climb,
		{to_helpee_result(a.climb_assist_a),to_helpee_result(a.climb_assist_b)},
		a.climb_was_assisted
	};
}

template<typename T>
map<T,float> averages(map<T,pair<unsigned,unsigned>> const& in){
	return map_values(
		[](auto p)->float{
			if(p.first==0){
				//force to 0 when no examples rather than letting it go to inf or some undefined number.
				return 0;
			}
			return (0.0+p.second)/p.first;
		},
		in
	);
}

Climb_capabilities f(vector<pair<Climb_result_robot,Climb2_situation>> a){
	//first item in pair is oppurtunities, second is actually done
	map<Climb_action,pair<unsigned,unsigned>> self;
	map<Help_given,pair<unsigned,unsigned>> help_given;

	for(auto [result,situation]:a){
		self[result.climb_action].second++;

		self[Climb_action::NONE].first+=result.climb_action==Climb_action::NONE;
		self[Climb_action::L1_SELF].first+=(result.climb_action==Climb_action::NONE || result.climb_action==Climb_action::L1_SELF);
		self[Climb_action::L2_SELF].first+=(result.climb_action==Climb_action::L2_SELF) || (
			situation.l2_available && (
				result.climb_action==Climb_action::NONE || result.climb_action==Climb_action::L1_SELF
			)
		);
		self[Climb_action::L3_SELF].first+=(result.climb_action==Climb_action::L3_SELF) || (
			situation.l3_available && result.climb_action!=Climb_action::L3_HELPEE
		);
		self[Climb_action::L2_HELPEE].first+=result.climb_action!=Climb_action::L3_SELF && result.climb_action!=Climb_action::L3_HELPEE && situation.l2_available;
		self[Climb_action::L3_HELPEE].first+=result.climb_action!=Climb_action::L3_SELF && situation.l3_available;

		help_given[result.help_given].second++;
		help_given[Help_given::NONE].first+=result.help_given==Help_given::NONE;
		help_given[Help_given::L2].first+=situation.partners_below_l2>=1 && situation.l2_available && (
			result.help_given==Help_given::NONE || result.help_given==Help_given::L2
		);
		help_given[Help_given::L3].first+=situation.partners_below_l3>=1 && situation.l3_available && (
			result.help_given==Help_given::NONE || result.help_given==Help_given::L2 || result.help_given==Help_given::L3
		);
		help_given[Help_given::L22].first+=situation.partners_below_l2>=2 && situation.l2_available>=2 && (
			result.help_given!=Help_given::L3 && result.help_given!=Help_given::L23 && result.help_given!=Help_given::L33
		);
		help_given[Help_given::L23].first+=(
			situation.partners_below_l2>=1 && 
			situation.partners_below_l3>=1 && 
			situation.l2_available && 
			situation.l3_available && 
			result.help_given!=Help_given::L33
		);

		//Don't look for there to actually be 
		help_given[Help_given::L33].first+=result.help_given==Help_given::L33 || (situation.partners_below_l3>=2 && situation.l3_available /*>=2*/);
	}
	return Climb_capabilities{averages(self),averages(help_given)};
}

map<Team,Climb_capabilities> analyze_climb(Scouting_data const& d){
	map<Team,vector<pair<Climb_result_robot,Climb2_situation>>> by_team;
	//vector<Climb_data_alliance>)nyi
	for(auto data:values(group(alliance_id,d))){
		Climb_data_alliance cd=mapf(climb_data_robot,data);
		auto dm=demangle(cd);
		for(auto [data_point,situation]:zip(data,dm)){
			by_team[data_point.team]|=situation;
		}
	}

	map<Team,Climb_capabilities> r;
	print_lines(r);
	for(auto [team,data]:by_team){
		r[team]=f(data);
	}
	return r;
}

/*int main(){
	Scouting_data d=example_input();
	auto result=analyze_climb(d);
	print_r(result);
}*/
