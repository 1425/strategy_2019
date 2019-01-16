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

auto match(Robot_match_data a){
	return a.match;
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

void csv_test(){
	auto r=example_input();
	write_file("example.csv",to_csv(r));
	assert(read_data()==r);
}

