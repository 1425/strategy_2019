#include<functional>
#include<sys/stat.h>
#include "util.h"
#include "table.h"

using namespace std;

/*
TODO: probability of winning based on different likelyhood of different types of robot
*/

enum class Climb_type{P3,P6,P12};

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

vector<Climb_type> climb_types(){
	return {
		Climb_type::P3,
		Climb_type::P6,
		Climb_type::P12
	};
}

struct Climb_odds{
	float l1,l2,l3; //0-1 (if no data, then this should come out as 0 instead of NaN)
};

bool operator<(Climb_odds a,Climb_odds b){
	#define X(A) if(a.A<b.A) return 1; if(b.A<a.A) return 0;
	X(l1) X(l2) X(l3)
	#undef X
	return 0;
}

std::ostream& operator<<(std::ostream& o,Climb_odds const& a){
	o<<"Climb_odds(";
	o<<"l1:"<<a.l1<<" ";
	o<<"l2:"<<a.l2<<" ";
	o<<"l3:"<<a.l3;
	return o<<")";
}

Climb_odds rand(const Climb_odds*)nyi

#define ROBOT_CAPABILITIES_ITEMS(X)\
	X(float,shelf_odds,0)\
	X(Climb_odds,climb,Climb_odds{})\
	X(float,ball_time,teleop_length()+1)\
	X(float,hatch_time,teleop_length()+1)

struct Robot_capabilities{
	//by default, can do almost nothing.
	//all the times are assumed to be >0.
	#define X(A,B,C) A B=C;
	ROBOT_CAPABILITIES_ITEMS(X)
	#undef X
};

bool operator<(Robot_capabilities const& a,Robot_capabilities const& b){
	#define X(A,B,C) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	ROBOT_CAPABILITIES_ITEMS(X)
	#undef X
	return 0;
}

std::ostream& operator<<(std::ostream& o,Robot_capabilities const& a){
	o<<"Robot_capabilities(";
	o<<"climb:"<<a.climb<<" ";
	o<<"ball_time:"<<a.ball_time<<" ";
	o<<"hatch_time:"<<a.hatch_time;
	return o<<")";
}

Robot_capabilities rand(const Robot_capabilities*){
	Robot_capabilities r;
	r.climb=rand((Climb_odds*)nullptr);
	r.ball_time=1+rand()%100;
	r.hatch_time=1+rand()%100;
	return r;
}

using Alliance_capabilities=std::array<Robot_capabilities,3>;

using Balls=int;
using Hatches=int;

using Climb_result=std::optional<Climb_type>;

vector<Climb_result> climb_results(){
	vector<Climb_result> r;
	r|=climb_types();
	r|=Climb_result{};
	return r;
}

//TODO: Add shelf
#define ROBOT_STRATEGY_ITEMS(X)\
	X(Balls,balls)\
	X(Hatches,hatches)\
	X(Climb_result,climb)

#define INST(A,B) A B;

struct Robot_strategy{
	ROBOT_STRATEGY_ITEMS(INST)
};

std::ostream& operator<<(std::ostream& o,Robot_strategy const& a){
	o<<"Robot_strategy(";
	o<<"b:"<<a.balls<<" h:"<<a.hatches<<" "<<a.climb;
	return o<<")";
}

bool operator<(Robot_strategy const& a,Robot_strategy const& b){
	#define X(A,B) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	ROBOT_STRATEGY_ITEMS(X)
	#undef X
	return 0;
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

vector<Climb_result> climb_capabilities(Robot_capabilities const& capabilities){
	return filter(
		[=](auto climb_result)->bool{
			if(!climb_result) return 1;
			return odds(capabilities.climb,*climb_result)>0;
		},
		climb_results()
	);
}

vector<Robot_strategy> available_strategies(Robot_capabilities const& capabilities){
	vector<Robot_strategy> r;
	for(auto climb:climb_capabilities(capabilities)){
		static const int CLIMB_TIME=20;
		auto remaining_time=teleop_length()-(climb?CLIMB_TIME:0);
		unsigned max_hatches=remaining_time/capabilities.hatch_time;
		for(auto hatches:range(1+max_hatches)){
			auto time_for_balls=remaining_time-hatches*capabilities.hatch_time;
			int balls=time_for_balls/capabilities.ball_time;
			r|=Robot_strategy{balls,(int)hatches,climb};
		}
	}
	return r;
}

using Alliance_strategy=std::array<Robot_strategy,3>;

vector<Alliance_strategy> available_strategies(Alliance_capabilities const& a){
	vector<Alliance_strategy> r;
	for(auto r0:available_strategies(a[0])){
		for(auto r1:available_strategies(a[1])){
			for(auto r2:available_strategies(a[2])){
				r|=Alliance_strategy{r0,r1,r2};
			}
		}
	}

	/*
	Spit into 3 sections:
	initial
	main
	final

	then combine the results of the three
	*/

	/*using Shelf=std::array<bool,3>;
	vector<Shelf> shelf;
	for(auto a:bools()){
		for(auto b:bools()){
			for(auto c:bools()){
				shelf|={a,b,c};
			}
		}
	}*/
	//std::array<int,3> shelf_strats{{0,1,2}}; //which robot to leave down
	//one that's assigned to be up could always just not use the slot

	/*std::vector<array<Climb_result,3>> climb_strats;
	auto ccap=mapf(climb_capabilities,a);
	for(auto a:ccap[0]){
		auto b_opt=to_set(ccap[1]);
		b_opt-=((a==Climb_type::P12)?set<Climb_result>{Climb_type::P12}:set<Climb_result>{});
		for(auto b:b_opt){
			auto c_opt=to_set(ccap[2]);
			if(a==Climb_type::P12 || b==Climb_type::P12){
				c_opt-=set<Climb_result>{Climb_type::P12};
			}
			if(a==Climb_type::P6 && b==Climb_type::P6){
				c_opt-=Climb_type::P6;
			}
			for(auto c:c_opt){
				climb_strats|=std::array<Climb_result,3>{{a,b,c}};
			}
		}
	}
	PRINT(climb_strats);*/

	/*for(auto shelf:shelf_strats){ //could make this one be the inner-most since it is independent of the others
		for(auto climb:climb_strats){
			nyi
		}
	}*/

	return r;
}

Climb_type max_odds_type(Climb_odds a){
	if(a.l1>a.l2 && a.l3) return Climb_type::P3;
	if(a.l2>a.l3) return Climb_type::P6;
	return Climb_type::P12;
}

int points(Climb_type a){
	switch(a){
		case Climb_type::P3: return 3;
		case Climb_type::P6: return 6;
		case Climb_type::P12: return 12;
		default: assert(0);
	}
}

int points(Climb_result a){
	if(!a){ return 0; }
	return points(*a);
}

int points(Alliance_capabilities cap,Alliance_strategy strat){
	auto balls=sum(mapf([](auto a){ return a.balls; },strat));
	auto hatches=sum(mapf([](auto a){ return a.hatches; },strat));
	auto p=points(balls,hatches);

	//TODO: Make this more precise.
	auto c=sum(mapf(
		[](auto p)->float{
			if(!p.second.climb){
				return 0;
			}
			auto type_odds=odds(p.first.climb,*p.second.climb);
			auto type_points=points(p.second.climb);
			return type_odds*type_points;
		},
		zip(cap,strat)
	));
	return p+c;
}

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

enum class Alliance{RED,BLUE};

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

#define ROBOT_MATCH_DATA_ITEMS(X)\
	X(Team,team,0)\
	X(Match,match,0)\
	X(Alliance,alliance,{})\
	X(bool,shelf,0)\
	X(unsigned,balls,0)\
	X(unsigned,hatches,0)\
	X(Climb_result,climb,{})

struct Robot_match_data{
	#define X(A,B,C) A B=C;
	ROBOT_MATCH_DATA_ITEMS(X)
	#undef X
};

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
	return rand()%22;
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

using Scouting_data=vector<Robot_match_data>;

/*Scouting_data rand(const Scouting_data*){
	nyi
}*/

auto balls(Robot_strategy a){
	return a.balls;
}

auto balls(Alliance_strategy a){
	return MAP(balls,a);
}

auto hatches(Robot_strategy a){
	return a.hatches;
}

auto hatches(Alliance_strategy a){
	return MAP(hatches,a);
}

Alliance_strategy normalize(Alliance_strategy a){
	a[0].balls=sum(balls(a));
	a[0].hatches=sum(hatches(a));
	a[1].balls=0;
	a[1].hatches=0;
	a[2].balls=0;
	a[2].hatches=0;
	return a;
}

static map<Alliance_capabilities,int> cache;

int points(Alliance_capabilities const& cap){
	auto n=sorted(cap);
	auto f=cache.find(n);
	if(f!=cache.end()){
		return f->second;
	}

	//auto as=to_set(mapf(normalize,available_strategies(cap)));
	//auto as=to_set(available_strategies(cap));
	auto as=available_strategies(cap);
	assert(as.size());
	//put in a filter here to only look at the ones that have the possiblity to be the best?

	auto m=mapf([=](auto x){ return points(cap,x); },as);
	auto r=max(m);
	cache[n]=r;
	return r;
}

static const int TOURNAMENT_SIZE=30; //old cmp division size
//TODO: Try out with fewer than 24 teams.

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

Climb_type parse(const Climb_type*,std::string s){
	if(s=="P3") return Climb_type::P3;
	if(s=="P6") return Climb_type::P6;
	if(s=="P12") return Climb_type::P12;
	assert(0);
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

string pop(vector<string>& v){
	//warning! this is O(n) and modifies its argument
	assert(v.size());
	auto r=v[0];
	v.erase(v.begin());
	//PRINT(v);
	return r;
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

struct Climb_situation{
	//l1 is assumed to always be available.
	bool l2_available,l3_available;
};

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

//using Climb_result_ext=pair<Climb_situation,Climb_result>;

//std::array<Climb_situation,3> demangle_climb_result(std::array<Climb_result,3> in){
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

auto match(Robot_match_data a){
	return a.match;
}

auto alliance_id(Robot_match_data a){
	return make_pair(a.match,a.alliance);
}

auto climb_result(Robot_match_data a){
	return a.climb;
}

map<Team,Climb_odds> show_climb_summary(Scouting_data d){
	/*
	Note: The odds that this returns do not add up to 100% becuase they are odds of things happening under different circumstances.
	So for example, it would be possible to return (0,1,1), which would mean that a team climbs L3 anytime it's open and otherwise 
	climbs to L2.
	*/
	map<Team,map<Climb_situation,multiset<Climb_result>>> by_team;
	for(auto data:values(group(alliance_id,d))){
		//(what do we mark things as if the robot is a no-show? -> hopefully just a blank sheet)
		auto d=demangle_climb_result(mapf(climb_result,data));

		/*print_r(data);
		print_r(d);
		cout<<"\n";*/

		for(auto [data_point,situation]:zip(data,d)){
			by_team[data_point.team][situation]|=data_point.climb;
		}
	}
	//print_r(by_team);
	//l3 odds given open
	//l2 odds given open and did not go for l3
	return map_values(
		[](auto data){
			auto l1_total=filter(
				[](auto x){ return x!=Climb_type::P6 && x!=Climb_type::P12; },
				flatten(values(data))
			);
			auto l1_used=filter([](auto x){ return x==Climb_type::P3; },l1_total);

			auto l2_total=filter([](auto x){ return x!=Climb_type::P12; },data[Climb_situation{1,1}])+data[Climb_situation{1,0}];
			auto l2_used=filter([](auto x){ return x==Climb_type::P6; },l2_total);

			auto l3_total=data[Climb_situation{0,1}]+data[Climb_situation{1,1}];
			auto l3_used=filter([](auto x){ return x==Climb_type::P12; },l3_total);

			auto f=[](auto a,auto b)->float{
				auto as=a.size();
				auto bs=b.size();
				if(bs==0) return 0;
				return (0.0+as)/bs;
			};
			return Climb_odds{
				f(l1_used,l1_total),f(l2_used,l2_total),f(l3_used,l3_total)
			};
		},
		by_team
	);
}

size_t sum(multiset<bool> const& m){
	return FILTER(id,m).size();
}

float mean(multiset<bool> const& m){
	assert(m.size());
	return (0.0+sum(m))/m.size();
}

#define MAP_VALUES(f,v) map_values([&](auto x){ return f(x); },v)

map<Team,float> interpret_shelf(Scouting_data d){
	//returns the odds that go on a shelf to start the game given that there is the opportunity to do so

	//basically, this is just doing the same thing as for the climb at the end of the game
	map<Team,multiset<bool>> by_team;
	for(auto data:values(group(alliance_id,d))){
		auto open_space=filter([](auto x){ return x.shelf; },data).size()<2;
		for(auto data_point:data){
			auto available=open_space || data_point.shelf;
			if(available){
				by_team[data_point.team]|=data_point.shelf;
			}
		}
	}
	
	return MAP_VALUES(mean,by_team);
}

map<Team,Robot_capabilities> interpret_data(Scouting_data d){
	//this is where the sausage is made

	auto climbing=show_climb_summary(d);
	auto shelf=interpret_shelf(d);

	map<Team,Robot_capabilities> r;
	for(auto [team,data]:group([](auto x){ return x.team; },d)){
		//Nick says that he and Evan may have a nice thing to put here.
		//For now, just throw something in.

		auto climbs=non_null(mapf([](auto x){ return x.climb; },data));
		vector<float> ball_times;
		vector<float> hatch_times;
		static const int ENDGAME_USED=20;
		for(auto match:data){
			auto balls=match.balls;
			auto hatches=match.hatches;
			auto actions=balls+hatches;
			if(actions){
				auto action_time=(0.0+teleop_length()-ENDGAME_USED)/actions;
				for(auto _:range(balls)){
					(void)_;
					ball_times|=action_time;
				}
				for(auto _:range(hatches)){
					(void)_;
					hatch_times|=action_time;
				}
			}else{
				auto penalty=teleop_length()+1; //This is just chosen to be anything that is longer than the match.
				ball_times|=penalty;
				hatch_times|=penalty;
			}
		}
		float ball_time=mean(ball_times);
		float hatch_time=mean(hatch_times);
		r[team]=Robot_capabilities{
			shelf[team],
			//climbs.size()?mode(climbs):Climb_type::P3,
			//ENDGAME_USED,//climb_time
			climbing[team],
			ball_time,
			hatch_time
		};
	}
	return r;
}

string th1(string s){ return th(s); }

int by_alliance(){
	csv_test();
	auto robots=interpret_data(example_input());

	auto t2="Robot capabilities";
	write_file(
		"robot_capabilities.html",
		html(
			head(title(t2))+
			body(
				h1(t2)+
				tag("table border",
					tr(join(mapf(
						th1,
						vector<string>{
							"Team number","P(shelf)",
							"P(Climb L1)","P(Climb L2)","P(Climb L3)",
							"Ball time(s)","Hatch time (s)"
							/*#define X(A,B,C) ""#B,
							ROBOT_CAPABILITIES_ITEMS(X)
							#undef X*/
						}
					)))+
					join(mapf(
						[](auto p){
							auto [team,cap]=p;
							return tr(
								td(team)+td(cap.shelf_odds)+
								td(cap.climb.l1)+td(cap.climb.l2)+td(cap.climb.l3)+
								td(cap.ball_time)+td(cap.hatch_time)
								/*join(vector<string>{
									#define X(A,B,C) td(cap.
								})*/
							);
						},
						robots
					))
				)
			)
		)
	);

	/*map<Team,Robot_capabilities> robots;
	for(auto team:range(TOURNAMENT_SIZE)){
		robots[team]=rand((Robot_capabilities*)nullptr);
	}*/
	assert(robots.size());
	auto target_team=begin(robots)->first;//TODO: make it us & configurable

	auto own_cap=robots[target_team];
	auto other_teams=without_key(robots,target_team);

	auto first_picks=reversed(sorted(mapf(
		[=](auto p){
			auto [team,cap]=p;
			return make_pair(
				points(Alliance_capabilities{own_cap,cap,Robot_capabilities{}}),
				team
			);
		},
		other_teams
	)));

	//enumerate_from(1,first_picks);
	print_lines(enumerate_from(1,first_picks));

	//first pass, rank by # of points would get alone?
	//first pass, rank by # of points would get with given robot
	//assume that robot #3 is going to go and play defense
	//TODO: Make the 2d pick list with first pick in the column down, and second pick going across
	using Points=int;
	map<Team,vector<pair<Points,Team>>> second_pick;
	for(auto [p1_team,p1_cap]:other_teams){
		PRINT(p1_team);
		auto remaining_teams=without_key(other_teams,p1_team);
		second_pick[p1_team]=reversed(sorted(mapf(
			[=](auto p){
				auto [p2_team,p2_cap]=p;
				return make_pair(
					points(Alliance_capabilities{own_cap,p1_cap,p2_cap}),
					p2_team
				);
			},
			remaining_teams
		)));
	}

	auto t1="Team "+as_string(target_team)+" Picklist 2019";
	auto cell=[](pair<int,int> p){
		return td(
			as_string(p.second)+"<br>"+small(p.first)
		);
	};
	write_file(
		"picks.html",
		html(
			head(title(t1))+
			body(
				h1(t1)+
				tag("table border",
					tr(
						tag("th rowspan=2 colspan=2","First pick")+
						tag("th colspan=22","Second pick")
					)+
					tr(join(mapf([](auto i){ return th(i); },range(1,1+22))))+
					//TODO: Put the main body of the table here
					join(mapf(
						[&](int i)->string{
							auto first_pick=first_picks[i].second;
							return tr(
								[](){
									/*if(i==0){
										return tag("th rowspan=23","First pick");
									}*/
									return "";
								}()+
								th(i+1)+
								cell(first_picks[i])+
								join(mapf(
									[&](int j){
										//return td("2nd pick:"+as_string(i)+" "+as_string(j));
										return cell(second_pick[first_pick][j]);
									},
									range(22)
								))
							);
						},
						range(23)
					))
				)
			)
		)
	);

	//TODO: Show best strategy for given alliance
	//TODO: Make a whole tournament work for robots
	//TODO: Check division size for the world championship, and the size for PNW districts
	return 0;
}

int main(int argc,char **argv){
	vector<pair<const char *,std::function<int()>>> options{
		make_pair("--strategy",by_alliance)
	};
	auto help=[&](){
		cout<<"Available arguments:\n";
		for(auto [arg,func]:options){
			(void)func;
			cout<<"\t"<<arg<<"\n";
		}
		return 0;
	};
	options|=make_pair("--help",help);

	if(argc!=2){
		cout<<"Argument required.\n\n";
		help();
		return 1;
	}

	for(auto [arg,func]:options){
		if(argv[1]==string(arg)){
			return func();
		}
	}
	cerr<<"Unrecognized argument: \""<<argv[1]<<"\"\n";
	return 1;
}

