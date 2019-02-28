#include "data.h"
#include "util.h"

//#include<stdlib.h>
//#include<iostream>
//#include<cassert>

using namespace std;

using Team=int;//official season; all the teams will have real numbers.

Team rand(const Team*){
	return 1+rand()%8000;
}

using Match=int;//initially, assume that these are all qual match numbers.

template<typename T>
std::optional<T> rand(const std::optional<T>*){
	if(rand()%2){
		return rand((T*)0);
	}
	std::optional<T> r;
	return r; //not sure why g++ was warning about returning in one step here.
}

std::ostream& operator<<(std::ostream& o,Alliance a){
	if(a==Alliance::RED) return o<<"RED";
	if(a==Alliance::BLUE) return o<<"BLUE";
	assert(0);
}

Alliance rand(const Alliance*){
	return (rand()%2)?Alliance::RED:Alliance::BLUE;
}

Alliance parse(const Alliance*,string const& s){
	if(s=="RED") return Alliance::RED;
	if(s=="BLUE") return Alliance::BLUE;
	PRINT(s);
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Robot_match_data const& a){
	o<<"Robot_match_data( ";
	#define X(A,B,C) o<<""#B<<":"<<a.B<<" ";
	ROBOT_MATCH_DATA_ITEMS(X)
	#undef X
	return o<<")";
}

bool operator==(Robot_match_data const& a,Robot_match_data const& b){
	#define X(A,B,C) if(a.B!=b.B) return 0;
	ROBOT_MATCH_DATA_ITEMS(X)
	#undef X
	return 1;
}

unsigned rand(const unsigned*){
	return rand()%9; //was %22.
}

bool rand(const bool*){
	return rand()%2;
}

bool parse(const bool*,string s){
	if(s=="0") return 0;
	if(s=="1") return 1;
	PRINT(s);
	nyi
}

Robot_match_data rand(const Robot_match_data*){
	return Robot_match_data{
		#define X(A,B,C) rand((A*)nullptr),
		ROBOT_MATCH_DATA_ITEMS(X)
		#undef X
	};
}

auto alliance(Robot_match_data const& a){
	return a.alliance;
}

auto team(Robot_match_data const& a){
	return a.team;
}

auto match(Robot_match_data a){
	return a.match;
}

auto balls(Robot_match_data a){
	return a.balls;
}

auto hatches(Robot_match_data a){
	return a.hatches;
}

pair<Match,Alliance> alliance_id(Robot_match_data a){
	return make_pair(a.match,a.alliance);
}

Climb_result climb_result(Robot_match_data a){
	return a.climb;
}

using Scouting_data=vector<Robot_match_data>;

string to_csv(Scouting_data const& data){
	stringstream ss;
	#define X(A,B,C) ss<<""#B<<",";
	ROBOT_MATCH_DATA_ITEMS(X)
	#undef X
	ss<<"\n";
	//ss<<"team,match,balls,hatches,climb\n";
	for(auto const& d:data){
		#define X(A,B,C) ss<<d.B<<",";
		ROBOT_MATCH_DATA_ITEMS(X)
		#undef X
		ss<<"\n";
		//ss<<d.team<<","<<d.match<<","<<d.balls<<","<<d.hatches<<","<<d.climb<<"\n";
	}
	return ss.str();
}

int parse(const int*,std::string s){
	return atoi(s.c_str());
}

unsigned int parse(const unsigned int*,std::string s){
	return atoi(s.c_str());
}

Scouting_data read_data(){
	ifstream f("example.csv");
	assert(f.good());
	string s;
	getline(f,s);
	{
		stringstream ss;
		#define X(A,B,C) ss<<""#B<<",";
		ROBOT_MATCH_DATA_ITEMS(X)
		#undef X
		assert(s==ss.str());
		//assert(s=="team,match,alliance,balls,hatches,climb,");
	}
	Scouting_data r;
	while(f.good()){
		getline(f,s);
		auto sp=split(s,',');
		if(sp.empty()){
			continue;
		}
		//assert(sp.size()==5);
		r|=Robot_match_data{
			#define X(A,B,C) parse((A*)nullptr,pop(sp)),
			ROBOT_MATCH_DATA_ITEMS(X)
			#undef X
		};
		assert(sp.empty());
	}
	return r;
}

Scouting_data example_input(){
	set<Team> teams;
	static const int TOURNAMENT_SIZE=75; //old cmp division size
	for(auto _:range(TOURNAMENT_SIZE)){
		(void)_;
		Team team;
		do{
			team=rand((Team*)nullptr);
		}while(teams.count(team));
		teams|=team;
	}
	//PRINT(teams);
	unsigned matches=teams.size()*12/6;
	Scouting_data r;
	for(auto match:range(1u,matches+1)){
		//PRINT(match);
		auto this_match=choose(6,teams);
		for(auto team:this_match){
			auto d=rand((Robot_match_data*)0);
			d.team=team;
			d.match=match;
			r|=d;
		}
	}
	return r;
}

template<typename T>
T min(std::set<T> const& a){
	assert(a.size());
	T r=*begin(a);
	for(auto elem:a){
		r=min(r,elem);
	}
	return r;
}

template<typename T>
T max(std::set<T> const& a){
	assert(a.size());
	T r=*begin(a);
	for(auto elem:a){
		r=max(r,elem);
	}
	return r;
}

#define GROUP(F,A) group([&](auto x){ return F(x); },A)

template<typename A,typename B>
A first(pair<A,B>)nyi

template<typename A,typename B>
B second(pair<A,B>)nyi

void check_consistency(Scouting_data d){
	int invalid_cnt=0;
	int incomplete_cnt=0;

	auto invalid=[&](string s){
		(void)s; //cout<<"Invalid: "<<s<<"\n";
		invalid_cnt++;
	};
	auto incomplete=[&](string s){
		(void)s; //cout<<"Incomplete: "<<s<<"\n";
		incomplete_cnt++;
	};

	/*
	Things to check:
	buddy climbs are consistent among partners
	total # of game pieces scored for the alliance is possible
	*/

	auto match_numbers=to_set(mapf(match,d));
	if(min(match_numbers)<1){
		invalid("Match \""+as_string(min(match_numbers))+"\" seen.\n");
	}

 	//should not exceed what would be needed for 12 matches per robot in a championship division
	if(max(match_numbers)>=12*75/6){
		invalid("Match \""+as_string(max(match_numbers))+"\" seen.\n");
	}

	auto missing_matches=filter(
		[=](auto x){
			return match_numbers.count(x)==0;
		},
		range(1,max(match_numbers)+1)
	);
	if(missing_matches.size()){
		incomplete("Matches missing: "+as_string(missing_matches));
	}

	auto robots_seen=to_set(mapf(team,d));
	cout<<"Total robots seen:"<<robots_seen.size()<<"\n";

	for(auto team:robots_seen){
		if(team<1 || team>=10000){
			invalid("Team number:"+as_string(team));
		}
	}

	auto g0=mapf(team,d);
	auto g=GROUP(id,g0);
	auto g2=MAP_VALUES(size,g);
	map<size_t,vector<Team>> r;
	for(auto [team,count]:g2){
		r[count]|=team;
	}
	cout<<"By number of appearances:\n";
	print_lines(r);

	for(auto point:d){
		auto invalid1=[&](string s){
			invalid([&](){
				stringstream ss;
				ss<<"Team "<<point.team<<" match "<<point.match<<":"<<s;
				return ss.str();
			}());
		};
		auto balls=point.balls;
		if(balls>20){
			invalid1("balls too high:"+as_string(balls));
		}
		auto hatches=point.hatches;
        	if(hatches>20){
			invalid1("hatches too high:"+as_string(hatches));
		}
	}

	for(auto [match_num,match_data]:group(match,d)){
		for(auto [alliance_color,alliance_data]:group(alliance,match_data)){
			auto invalid1=[&](string s){
				stringstream ss;
				ss<<"Alliance "<<match_num<<" "<<alliance_color<<": "<<s;
				invalid(ss.str());
			};
			auto members=mapf(team,alliance_data);
			if(alliance_data.size()>3){
				invalid1([=](){
					stringstream ss;
					ss<<members.size()<<" members ("<<members<<").";
					return ss.str();
				}());
			}
			if(alliance_data.size()<3){
				incomplete([=](){
					stringstream ss;
					ss<<"Alliance "<<match_num<<" "<<alliance_color<<" has "<<members.size()<<" members ("<<members<<").";
					return ss.str();
				}());
			}

			auto alliance_balls=sum(MAP(balls,alliance_data));
			if(alliance_balls>20){
				invalid1("Too many balls:"+as_string(alliance_balls));
			}
			auto alliance_hatches=sum(MAP(hatches,alliance_data));
			if(alliance_hatches>20){
				invalid1("Too many hatches:"+as_string(alliance_hatches));
			}

			map<Team,pair<Climb_result,optional<Team>>> climb_info;
			for(auto point:alliance_data){
				auto f=climb_info.find(point.team);
				if(f==climb_info.end()){
					climb_info[point.team]=make_pair(point.climb,optional<Team>());
				}else{
					auto found=f->second;
					if(found.first!=point.climb){
						invalid1("Inconsistent climb data");
					}
				}

				auto other_teams=filter([=](auto x){ return x!=point.team; },MAP(team,alliance_data));
				if(other_teams.size()==2){
					auto add_buddy=[&](Team team,Climb_result result){
						auto f=climb_info.find(team);
						if(f==climb_info.end()){
							climb_info[team]=make_pair(result,point.team);
						}else{
							auto &d=f->second;
							if(d.first!=result){
								invalid1("Inconsistent climb data");
							}
							if(d.second){
								invalid1("Helped by both partners?");
							}else{
								d.second=point.team;
							}
						}
					};
					if(point.climb_buddy_lower){
						auto buddy_lower=other_teams[0];
						add_buddy(buddy_lower,point.climb_buddy_lower);
					}
					if(point.climb_buddy_higher){
						auto buddy_higher=other_teams[1];
						add_buddy(buddy_higher,point.climb_buddy_higher);
					}
				}
			}
		}

		auto teams=mapf(team,match_data);
		if(to_set(teams).size()!=teams.size()){
			invalid([=]{
				stringstream ss;
				ss<<"Team appeared multiple times in a match:"<<sorted(teams);
				return ss.str();
			}());
		}
	}

}

void csv_test(){
	auto r=example_input();
	check_consistency(r);
	write_file("example.csv",to_csv(r));
	assert(read_data()==r);
}

