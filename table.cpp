#include<functional>
#include<sys/stat.h>
#include "util.h"

//start program-specific functions

using namespace std;

static const int SPACES=8+6*2;
static const int NULL_HATCHES=6;
static const int TELEOP_LENGTH=135;

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

#define ROBOT_CAPABILITIES_ITEMS(X)\
	X(Climb_type,climb_type,Climb_type::P3)\
	X(float,climb_time,TELEOP_LENGTH+1)\
	X(float,ball_time,TELEOP_LENGTH+1)\
	X(float,hatch_time,TELEOP_LENGTH+1)

struct Robot_capabilities{
	//by default, can do almost nothing.
	Climb_type climb_type=Climb_type::P3;
	//all the times are assumed to be >0.
	float climb_time=TELEOP_LENGTH+1;
	float ball_time=TELEOP_LENGTH+1,hatch_time=TELEOP_LENGTH+1;
};

bool operator<(Robot_capabilities const& a,Robot_capabilities const& b){
	#define X(A,B,C) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	ROBOT_CAPABILITIES_ITEMS(X)
	#undef X
	return 0;
}

/*
asignments for robots based on their capabilities
how long for them to climb?
first pass, assume that going to be able to get 

probability of winning based on different likelyhood of different types of robot

it was field vs balls time being spent
*/

std::ostream& operator<<(std::ostream& o,Robot_capabilities const& a){
	o<<"Robot_capabilities(";
	o<<"climb:"<<a.climb_type<<" "<<a.climb_time<<" ";
	o<<"ball_time:"<<a.ball_time<<" ";
	o<<"hatch_time:"<<a.hatch_time;
	return o<<")";
}

Robot_capabilities rand(const Robot_capabilities*){
	Robot_capabilities r;
	r.climb_type=rand((Climb_type*)nullptr);
	r.climb_time=1+rand()%30;
	r.ball_time=1+rand()%100;
	r.hatch_time=1+rand()%100;
	return r;
}

using Alliance_capabilities=std::array<Robot_capabilities,3>;

using Balls=int;
using Hatches=int;

#define ROBOT_STRATEGY_ITEMS(X)\
	X(Balls,balls)\
	X(Hatches,hatches)\
	X(bool,climb)

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

vector<Robot_strategy> available_strategies(Robot_capabilities const& capabilities){
	vector<Robot_strategy> r;
	auto climb_available=capabilities.climb_time<TELEOP_LENGTH;
	auto climb_options=climb_available?vector<bool>{{0,1}}:vector<bool>{{0}};
	for(auto climb:climb_options){
		auto remaining_time=TELEOP_LENGTH-capabilities.climb_time;
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
	return r;
}

int points(unsigned balls,unsigned hatches){
	int hatch_points=2*min(hatches,SPACES);
	int ball_spaces_available=min(SPACES,hatches+NULL_HATCHES);
	int scored_balls=min(ball_spaces_available,balls);
	int ball_points=scored_balls*3;
	return hatch_points+ball_points;
}

int marginal_ball(int balls,int hatches){
	return points(balls+1,hatches)-points(balls,hatches);
}

int marginal_hatch(int balls,int hatches){
	return points(balls,hatches+1)-points(balls,hatches);
}

template<typename Func,typename Colorizer>
void run(string filename,string title,Func f,Colorizer color){
	stringstream ss;
	ss<<tag(
		"tr",
		tag("td colspan=2 rowspan=2","")+
		tag("th colspan="+as_string(SPACES+1),"Hatches")
	);
	ss<<tag(
		"tr",
		[](){
			stringstream ss;
			for(auto balls:range_st<SPACES+1>()){
				ss<<tag("th",balls);
			}
			return ss.str();
		}()
	);
	for(auto balls:range_st<SPACES+1>()){
		stringstream s2;
		s2<<tag("th",as_string(balls));
		for(auto hatches:range_st<SPACES+1>()){
			s2<<tag("td "+color(balls,hatches),f(balls,hatches));
		}
		ss<<tag(
			"tr",
			[&]()->string{
				if(balls==0){
					return tag("th rowspan="+as_string(SPACES+1),"Balls");
				}
				return "";
			}()+
			s2.str()
		);
	}

	int r=mkdir("output",0755);
	if(r!=0){
		assert(r==-1);
		//This happens when the directory already exists.
		if(errno!=EEXIST){
			perror("Failed to create output directory");
			exit(1);
		}
	}

	write_file("output/"+filename,
		tag(
			"html",
			tag("head",
				tag("title",title)
			)+
			tag(
				"body",
				tag("h1",title)+
				tag("table border",ss.str())
			)
		)
	);
}

int standard(){
	auto color=[](int balls,int hatches){
		auto p=points(balls,hatches);
		if(p<50){
			auto s=as_hex(rerange(0,50,0,255,points(balls,hatches)));
			return "bgcolor='#ff"+s+s+"'";
		}
		auto s=as_hex(rerange(50,100,255,0,points(balls,hatches)));
		return "bgcolor='#"+s+"ff"+s+"'";
	};

	auto color2=[](int balls,int hatches){
		auto s=as_hex(rerange(0,5,0,255,marginal_hatch(balls,hatches)));
		return "bgcolor='#ff"+s+s+"'";
	};

	auto color3=[](int balls,int hatches){
		auto s=as_hex(rerange(0,5,0,255,marginal_ball(balls,hatches)));
		return "bgcolor='#ff"+s+s+"'";
	};

	run(
		"basic.html",
		"Points earned",points,color
	);
	run(
		"hatch.html","Marginal value of a hatch",marginal_hatch,color2
	);
	run(
		"ball.html","Marginal value of a ball",marginal_ball,color3
	);
	return 0;
}

int points1(pair<int,int> a){
	return points(a.first,a.second);
}

void with_limit(string name,std::function<int(int,int)> time_used){
	set<pair<int,int>> bests;
	for(auto x:group(
		[=](auto p){ return time_used(p.first,p.second); },
		cross(range(SPACES+1),range(SPACES+1))
	)){
		auto best_score=max(MAP(points1,x.second));
		bests|=filter([best_score](auto y){ return points1(y)==best_score; },x.second);
	}

	for(int limit:range(1+TELEOP_LENGTH)){
		auto color=[&limit,bests,time_used](int balls,int hatches)->string{
			if(time_used(balls,hatches)>limit){
				return "bgcolor=black";
			}
			if(make_pair(balls,hatches)&bests){
				return "bgcolor=gold";
			}
			auto p=points(balls,hatches);
			if(p<50){
				auto s=as_hex(rerange(0,50,0,255,points(balls,hatches)));
				return "bgcolor='#ff"+s+s+"'";
			}
			auto s=as_hex(rerange(50,100,255,0,points(balls,hatches)));
			return "bgcolor='#"+s+"ff"+s+"'";
		};

		run(
			name+"_"+as_string(limit)+".html",
			"Points earned, limit "+as_string(limit)+" seconds",points,color
		);
	}
}

int by_pieces(){
	with_limit("1s1s",[](int a,int b){ return a+b; });
	with_limit("1s2s",[](int a,int b){ return a+2*b; });
	with_limit("2s1s",[](int a,int b){ return 2*a+b; });
	with_limit("3s2s",[](int a,int b){ return 3*a+2*b; });
	with_limit("2s3s",[](int a,int b){ return 2*a+3*b; });
	with_limit("5s4s",[](int a,int b){ return 5*a+4*b; });
	with_limit("7s4s",[](int a,int b){ return 7*a+4*b; });
	return 0;
}

/*template<typename A,typename B>
vector<pair<A,B>> zip(vector<A> const& a,vector<B> const& b){
	nyi
}*/

template<typename A,typename B,size_t LEN>
array<pair<A,B>,LEN> zip(array<A,LEN> const& a,array<B,LEN> const& b){
	array<pair<A,B>,LEN> r;
	std::transform(
		begin(a),end(a),begin(b),begin(r),
		[](auto a1,auto b1){ return make_pair(a1,b1); }
	);
	return r;
}

int points(Alliance_capabilities cap,Alliance_strategy strat){
	auto balls=sum(mapf([](auto a){ return a.balls; },strat));
	auto hatches=sum(mapf([](auto a){ return a.hatches; },strat));
	auto p=points(balls,hatches);
	auto c=sum(mapf(
		[](auto p){
			if(!p.second.climb){
				return 0;
			}
			switch(p.first.climb_type){
				case Climb_type::P3: return 3;
				case Climb_type::P6: return 6;
				case Climb_type::P12: return 12;
				default: assert(0);
			}
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
using Climb_result=std::optional<Climb_type>;

template<typename T>
std::optional<T> rand(const std::optional<T>*){
	if(rand()%2){
		return rand((T*)0);
	}
	std::optional<T> r;
	return r; //not sure why g++ was warning about returning in one step here.
}

#define ROBOT_MATCH_DATA_ITEMS(X)\
	X(Team,team,0)\
	X(Match,match,0)\
	X(unsigned,balls,0)\
	X(unsigned,hatches,0)\
	X(Climb_result,climb,{})

struct Robot_match_data{
	#define X(A,B,C) A B=C;
	ROBOT_MATCH_DATA_ITEMS(X)
	#undef X
};

std::ostream& operator<<(std::ostream& o,Robot_match_data const& a){
	o<<"Robot_match_data(";
	o<<"team:"<<a.team<<" ";
	o<<"match:"<<a.match<<" ";
	o<<"balls:"<<a.balls<<" ";
	o<<"hatches:"<<a.hatches<<" ";
	o<<"climb:"<<a.climb<<" ";
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

Robot_match_data rand(const Robot_match_data*){
	return Robot_match_data{
		#define X(A) rand((A*)nullptr),
		X(Team)
		X(Match)
		X(unsigned)
		X(unsigned)
		X(Climb_result)
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

template<typename T>
set<T> to_set(vector<T> v){
	return std::set<T>(begin(v),end(v));
}

template<typename T,size_t LEN>
std::array<T,LEN> sorted(std::array<T,LEN> a){
	std::sort(begin(a),end(a));
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
	//put in a filter here to only look at the ones that have the possiblity to be the best?

	auto m=mapf([=](auto x){ return points(cap,x); },as);
	auto r=max(m);
	cache[n]=r;
	return r;
}

static const int TOURNAMENT_SIZE=30; //old cmp division size
//TODO: Try out with fewer than 24 teams.

template<typename T>
set<T> choose(size_t num,set<T> a){
	if(num==0){
		return {};
	}
	assert(a.size());
	auto other=choose(num-1,a);
	auto left=filter([other](auto x){ return other.count(x)==0; },a);
	return other|set<T>{to_vec(left)[rand()%left.size()]};
}

string to_csv(Scouting_data data){
	stringstream ss;
	ss<<"team,match,balls,hatches,climb\n";
	for(auto d:data){
		ss<<d.team<<","<<d.match<<","<<d.balls<<","<<d.hatches<<","<<d.climb<<"\n";
	}
	return ss.str();
}

vector<string> split(string s,char target){
	vector<string> r;
	stringstream ss;
	for(auto c:s){
		if(c==target){
			r|=ss.str();
			ss.str("");
		}else{
			ss<<c;
		}
	}
	if(ss.str().size()){
		r|=ss.str();
	}
	return r;
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
		return Climb_result{};
	}
	return parse((Climb_type*)nullptr,s);
}

Scouting_data read_data(){
	ifstream f("example.csv");
	assert(f.good());
	string s;
	getline(f,s);
	assert(s=="team,match,balls,hatches,climb");
	Scouting_data r;
	while(f.good()){
		getline(f,s);
		auto sp=split(s,',');
		if(sp.empty()){
			continue;
		}
		assert(sp.size()==5);
		r|=Robot_match_data{
			parse((Team*)nullptr,sp[0]),
			parse((Match*)nullptr,sp[1]),
			parse((unsigned*)nullptr,sp[2]),
			parse((unsigned*)nullptr,sp[3]),
			parse((Climb_result*)nullptr,sp[4])
		};
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
	for(auto match:range(1,matches+1)){
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

map<Team,Robot_capabilities> interpret_data(Scouting_data d){
	//this is where the sausage is made

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
				auto action_time=(0.0+TELEOP_LENGTH-ENDGAME_USED)/actions;
				for(auto _:range(balls)){
					(void)_;
					ball_times|=action_time;
				}
				for(auto _:range(hatches)){
					(void)_;
					hatch_times|=action_time;
				}
			}else{
				auto penalty=TELEOP_LENGTH+1; //This is just chosen to be anything that is longer than the match.
				ball_times|=penalty;
				hatch_times|=penalty;
			}
		}
		float ball_time=mean(ball_times);
		float hatch_time=mean(hatch_times);
		r[team]=Robot_capabilities{
			climbs.size()?mode(climbs):Climb_type::P3,
			ENDGAME_USED,//climb_time
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
						vector<string>{"Team number","Climb type","Climb time","Ball time(s)","Hatch time (s)"}
					)))+
					join(mapf(
						[](auto p){
							auto [team,cap]=p;
							return tr(
								td(team)+
								td(cap.climb_type)+td(cap.climb_time)+td(cap.ball_time)+td(cap.hatch_time)
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

	//TODO: Optimize
	//TODO: Make the 2d pick table!

	//TODO: Show best strategy for given alliance
	//TODO: Make a whole tournament work for robots
	//TODO: Check division size for the world championship, and the size for PNW districts
	return 0;
}

int main(int argc,char **argv){
	vector<pair<const char *,std::function<int()>>> options{
		make_pair("--standard",standard),
		make_pair("--by_pieces",by_pieces),
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

