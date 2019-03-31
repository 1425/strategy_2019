#include<functional>
#include<sys/stat.h>
#include "util.h"
#include "table.h"
#include "climb.h"
#include "data.h"
#include "climb2.h"

using namespace std;

template<typename T>
vector<T> tail(vector<T> v){
	vector<T> r;
	for(size_t i=1;i<v.size();i++){
		r|=v[i];
	}
	return r;
}

#define ROBOT_CAPABILITIES_ITEMS(X)\
	X(float,shelf_odds,0)\
	X(Climb_capabilities,climb,Climb_capabilities{})\
	X(float,ball_time,teleop_length()+1)\
	X(float,hatch_time,teleop_length()+1)\
	X(float,bonus,0)

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
	o<<"Robot_capabilities( ";
	#define X(A,B,C) o<<""#B<<":"<<a.B<<" ";
	ROBOT_CAPABILITIES_ITEMS(X)
	#undef X
	return o<<")";
}

Robot_capabilities operator/(Robot_capabilities a,float f){
	#define X(A,B,C) a.B/=f;
	ROBOT_CAPABILITIES_ITEMS(X)
	#undef X
	return a;
}

Robot_capabilities operator+(Robot_capabilities a,Robot_capabilities const& b){
	#define X(A,B,C) a.B+=b.B;
	ROBOT_CAPABILITIES_ITEMS(X)
	#undef X
	return a;
}

Robot_capabilities rand(const Robot_capabilities*){
	Robot_capabilities r;
	r.climb=rand((Climb_capabilities*)nullptr);
	r.ball_time=1+rand()%100;
	r.hatch_time=1+rand()%100;
	return r;
}

float odds(Climb_capabilities cap,Climb_type a){
	switch(a){
		case Climb_type::P3: return cap.self[Climb_action::L1_SELF];
		case Climb_type::P6: return cap.self[Climb_action::L2_SELF];
		case Climb_type::P12: return cap.self[Climb_action::L3_SELF];
		default: assert(0);
	}
}

using Climb_strat=pair<Climb_result,Help_given>;

vector<Climb_strat> climb_capabilities(Robot_capabilities const& capabilities){
	vector<Climb_strat> r;
	for(auto climb_result:climb_results()){
		if(!climb_result || odds(capabilities.climb,*climb_result)>0){
			//now loop over possible help given
			//assume that later on will say that people possible to help have level 1 climbed.
			for(auto help:help_givens()){
				auto ok=[&]()->bool{
					if(help==Help_given::NONE) return 1;

					auto f=capabilities.climb.help_given.find(help);
					return f!=capabilities.climb.help_given.end() && f->second>0;
				}();
				if(ok){
					r|=make_pair(climb_result,help);
				}
			}
		}
	}
	assert(r.size());
	return r;
}

/*vector<Climb_result> climb_capabilities(Robot_capabilities const& capabilities){
	//eventually, this will need to change form
	//probably do all the climbing seperate from the main part of the game.
	return filter(
		[=](auto climb_result)->bool{
			if(!climb_result) return 1;
			return odds(capabilities.climb,*climb_result)>0;
		},
		//climb_results()
		climb
	);
}*/

using Alliance_capabilities=std::array<Robot_capabilities,3>;

using Balls=int;
using Hatches=int;

#define ROBOT_STRATEGY_ITEMS(X)\
	X(bool,shelf)\
	X(Balls,balls)\
	X(Hatches,hatches)\
	X(Climb_strat,climb)

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

auto hatches(Robot_strategy a){
	return a.hatches;
}

auto balls(Robot_strategy a){
	return a.balls;
}

static const int CLIMB_TIME=20;

vector<Robot_strategy> available_strategies(Robot_capabilities const& capabilities){
	vector<Robot_strategy> r;
	for(auto shelf:{0}){ //TODO: At some point will want to put the option for the shelf here once perf is better.
		for(auto climb:climb_capabilities(capabilities)){
			auto remaining_time=teleop_length()-(climb==Climb_strat{{},Help_given::NONE}?CLIMB_TIME:0);
			unsigned max_hatches=remaining_time/capabilities.hatch_time;
			for(auto hatches:range(1+max_hatches)){
				auto time_for_balls=remaining_time-hatches*capabilities.hatch_time;
				int balls=time_for_balls/capabilities.ball_time;
				r|=Robot_strategy{shelf,balls,(int)hatches,climb};
			}
		}
	}
	return r;
}

vector<pair<int,int>> main_strats(Robot_capabilities capabilities,unsigned remaining_time){
	vector<pair<int,int>> r;
	unsigned max_hatches=remaining_time/capabilities.hatch_time;
	for(auto hatches:range(1+max_hatches)){
		auto time_for_balls=remaining_time-hatches*capabilities.hatch_time;
		int balls=time_for_balls/capabilities.ball_time;
		r|=make_pair(balls,(int)hatches);
	}
	return r;
}

vector<pair<int,int>> frontier(vector<pair<int,int>> in){
	map<int,int> r;
	for(auto [a,b]:in){
		a=min(20,a);
		b=min(20,b);
		auto f=r.find(a);
		if(f==r.end()){
			r[a]=b;
		}else{
			f->second=max(f->second,b);
		}
	}

	//delete the items where the first element doesn't have any advantage over the next
	auto f1=filter(
		[&](auto x){
			auto [a,b]=x;
			auto f=r.find(a+1);
			if(f!=r.end() && f->second==b){
				return 0;
			}
			return 1;
		},
		r
	);
	return to_vec(f1);
}

using Alliance_strategy=std::array<Robot_strategy,3>;

auto balls(Alliance_strategy a){
	return MAP(balls,a);
}

auto hatches(Alliance_strategy a){
	return MAP(hatches,a);
}

vector<Alliance_strategy> available_strategies(Alliance_capabilities const& a){
	vector<Alliance_strategy> r;
	/*
	This is how the options were originally generated.  Creates lots of options that are known to be suboptimal.
	for(auto r0:available_strategies(a[0])){
		for(auto r1:available_strategies(a[1])){
			for(auto r2:available_strategies(a[2])){
				r|=Alliance_strategy{r0,r1,r2};
			}
		}
	}*/

	std::vector<array<Climb_strat,3>> climb_strats;
	auto ccap=mapf(climb_capabilities,a);
	for(auto a:ccap[0]){
		//PRINT(a);
		//auto b_opt=to_set(ccap[1]);

		auto b_opt=filter(
			[a](Climb_strat b)->bool{
				if(a.first==Climb_type::P12){
					return b.first!=Climb_type::P12;
				}
				return 1;
			},
			ccap[1]
		);

		for(auto b:b_opt){
			//auto c_opt=to_set(ccap[2]);
			auto c_opt=filter(
				[a,b](auto const& c)->bool{
					if(a.first==Climb_type::P12 || b.first==Climb_type::P12){
						return c.first!=Climb_type::P12;
					}
					if(a.first==Climb_type::P6 && b.first==Climb_type::P6){
						return c.first!=Climb_type::P6;
					}
					return 1;
				},
				ccap[2]
			);
			for(auto c:c_opt){
				climb_strats|=std::array<Climb_strat,3>{{a,b,c}};
			}
		}
	}

	for(auto climb_strat:climb_strats){
		auto main_by_robot=mapf(
			[](auto p){
				auto [climb_strat,robot_cap]=p;
				auto time_left=teleop_length()-(climb_strat.first?CLIMB_TIME:0);
				return main_strats(robot_cap,time_left);
			},
			zip(climb_strat,a)
		);
		auto c=MAP(sum,cross3(main_by_robot));
		auto f=frontier(c);
		for(auto item:f){
			auto [balls,hatches]=item;
			for(auto off_shelf:range_st<3>()){
				//"off_shelf"=which robot to not leave a spot for on level 2 at the start of the match

				r|=Alliance_strategy{
					//For now, going to put all the balls and hatches on one robot
					//will not change alliance selection answers, and can be worked
					//out later if ask this to do in-match strategy.
					Robot_strategy{off_shelf!=0,balls,hatches,climb_strat[0]},
					Robot_strategy{off_shelf!=1,0,0,climb_strat[1]},
					Robot_strategy{off_shelf!=2,0,0,climb_strat[2]}
				};
			}
		}
	}
	return r;
}

template<typename T,size_t MAX>
class Fixed_vector{
	std::array<T,MAX> data;
	size_t size_;

	public:
	Fixed_vector():size_(0){}

	size_t size()const{
		return size_;
	}

	T operator[](size_t i)const{
		assert(i<size_);
		return data[i];
	}

	void pop_back(){
		assert(size_);
		size_--;
	}

	Fixed_vector& operator|=(T t){
		assert(size_<MAX);
		data[size_]=t;
		size_++;
		return *this;
	}

	auto begin(){ return data.begin(); }
	auto end(){ return data.begin()+size_; }
};

template<typename T,size_t MAX>
Fixed_vector<T,MAX> sorted(Fixed_vector<T,MAX> a){
	sort(begin(a),end(a));
	return a;
}

int points(Alliance_capabilities const& cap,Alliance_strategy const& strat){
	auto z=zip(cap,strat);
	auto shelf_points=sum(mapf(
		[](auto const& p)->float{
			//[](Robot_capabilities rc,Robot_strategy rs)->float{
			auto const& [rc,rs]=p;
			if(rs.shelf){
				return 3+3*rc.shelf_odds;
			}
			return 3; //just assume that Sandstorm Bonus 1 will be scored.
		},
		z//zip(cap,strat)
	));

	auto balls=sum(mapf([](auto a){ return a.balls; },strat));
	auto hatches=sum(mapf([](auto a){ return a.hatches; },strat));
	auto p=points(balls,hatches);

	//nyi (void)shelf_points; (void)p; 

	Fixed_vector<float,6> l2_help;
	Fixed_vector<float,6> l3_help;
	//vector<float> l2_help;
	//vector<float> l3_help;

	for(auto const& [r_cap,r_strat]:z/*zip(cap,strat)*/){
		//auto p=r_cap.climb.help_given[r_strat.climb.second];
		auto f=r_cap.climb.help_given.find(r_strat.climb.second);
		if(f==r_cap.climb.help_given.end()){
			continue;
		}
		auto p=f->second;
		switch(r_strat.climb.second){
			case Help_given::NONE:
				break;
			case Help_given::L2:
				l2_help|=p;
				break;
			case Help_given::L3:
				l3_help|=p;
				break;
			case Help_given::L23:
				l2_help|=p;
				l3_help|=p;
				break;
			case Help_given::L22:
				l2_help|=p;
				l2_help|=p;
				break;
			case Help_given::L33:
				l3_help|=p;
				l3_help|=p;
				break;
			default: assert(0);
		}
	}

	l2_help=sorted(l2_help);
	l3_help=sorted(l3_help);

	auto climb_points=sum(mapf(
		[&](auto const& p)->float{
			if(!p.second.climb.first){
				return 0;
			}
			//nyi //TODO: Deal w/ robots that are helped.
			auto type_odds=odds(p.first.climb,*p.second.climb.first);
			float type_points;
			if(p.second.climb.first==Climb_type::P3 && l3_help.size()){
				auto p=l3_help[l3_help.size()-1];
				l3_help.pop_back();//l3_help=tail(l3_help); //take from front not super efficient
				type_points=12*p;
			}else if(p.second.climb.first==Climb_type::P3 && l2_help.size()){
				auto p=l2_help[l2_help.size()-1];
				l2_help.pop_back();//l3_help=tail(l2_help); //take from front not super efficient
				type_points=12*p;
			}else{
				type_points=points(p.second.climb.first);//FIXME
			}
			return type_odds*type_points;
		},
		z//zip(cap,strat)
	));

	auto bonus_points=sum(mapf([](auto const& a){ return a.bonus; },cap));
	return shelf_points+p+climb_points+bonus_points;
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

map<Team,Climb_odds> show_climb_summary1(Scouting_data d){
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

map<Team,Climb_capabilities> show_climb_summary(Scouting_data a){
	return analyze_climb(a);
}

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
	
	return MAP_VALUES([](auto x){ return mean_else(x,0); },by_team);
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
		float ball_time=mean_else(ball_times,teleop_length()+1.0f);
		float hatch_time=mean_else(hatch_times,teleop_length()+1.0f);
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

int by_alliance(Team target_team,optional<string> path){
	//csv_test();
	auto data_to_use=[=](){
		if(!path) return example_input();
		return read_data(*path);
	}();
	check_consistency(data_to_use);
	auto robots=interpret_data(data_to_use);
	{
		auto f=robots.find(Team{1425});
		if(f!=robots.end()){
			f->second.climb.self[Climb_action::L3_SELF]=.8;
		}
	}
	for(auto& p:robots){
		p.second.shelf_odds=(p.second.shelf_odds>.05)?1:p.second.shelf_odds;
		if(p.first==Team{5085}){
			p.second.bonus=-5;
		}/*
		if(p.first==Team{2733}){
			p.second.bonus=.5;
		}
		if(p.first==Team{6831}){
			p.second.bonus=-3;
		}
		if(p.first==Team{2550}){
			p.second.bonus-=3;
		}*/
		if(p.first==Team{3636}){
			p.second.bonus-=3;
		}
		if(p.first==Team{2471}){
			p.second.bonus+=.5;
		}
		if(p.first==Team{2915}){
			p.second.bonus-=2;
		}
		if(p.first==Team{6465}){
			p.second.bonus+=4;
		}
		if(p.first==Team{2898}){
			p.second.bonus-=2;
		}
		if(p.first==Team{3673}){
			p.second.bonus+=6;
		}
	}

	assert(robots.size());
	if(robots.count(target_team)!=1){
		cout<<"Warning: Did not find team "<<target_team<<".  Will create a dummy entry for them and continue.\n";
		robots[target_team]={};
	}

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
							"Ball time(s)","Hatch time (s)",
							"P(Climb L1)<br> | (not L2 or L3)",
							"P(Climb L2)<br> | (available & !L3)",
							"P(Climb L3)<br> | available",
							"P(l2_helpee)",
							"P(l3_helpee)",
							"Help given",
							"Bonus"
							/*#define X(A,B,C) ""#B,
							ROBOT_CAPABILITIES_ITEMS(X)
							#undef X*/
						}
					)))+
					join(mapf(
						[](auto p){
							auto [team,cap]=p;
							//put new climb stuff here.
							return tr(
								td(team)+td(cap.shelf_odds)+
								//td(cap.climb.l1)+td(cap.climb.l2)+td(cap.climb.l3)+
								td(cap.ball_time)+td(cap.hatch_time)+
								td(cap.climb.self[Climb_action::L1_SELF])+
								td(cap.climb.self[Climb_action::L2_SELF])+
								td(cap.climb.self[Climb_action::L3_SELF])+
								td(cap.climb.self[Climb_action::L2_HELPEE])+
								td(cap.climb.self[Climb_action::L3_HELPEE])+
								td(cap.climb.help_given)+
								td(cap.bonus)
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

	auto own_cap=robots[target_team];
	PRINT(own_cap);
	auto other_teams=without_key(robots,target_team);

	auto first_pass=reversed(sorted(mapf(
		[=](auto p){
			auto [team,cap]=p;
			return make_pair(
				points(Alliance_capabilities{own_cap,cap,Robot_capabilities{}}),
				team
			);
		},
		other_teams
	)));

	using Points=int;
	auto placeholder_robot=[=](){
		auto ex=mapf(
			[=](pair<Points,Team> p){
				auto f=robots.find(p.second);
				assert(f!=robots.end());
				return f->second;
			},
			take(4,skip(20,first_pass))
		);
		return mean_else(ex,Robot_capabilities{});
	}();
	//PRINT(placeholder_robot);
	auto first_picks=reversed(sorted(mapf(
		[=](auto p){
			auto [team,cap]=p;
			return make_pair(
				points(Alliance_capabilities{own_cap,cap,placeholder_robot}),
				team
			);
		},
		other_teams
	)));
	//print_lines(zip(seconds(first_pass),seconds(first_picks)));

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
					join(mapf(
						[&](auto p)->string{
							auto [i,first_pick_elem]=p;
							//auto first_pick=first_picks[i].second;
							auto first_pick=first_pick_elem.second;
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
									[&](auto d){
										//return td("2nd pick:"+as_string(i)+" "+as_string(j));
										//return cell(second_pick[first_pick][j]);
										return cell(d);
									},
									take(22,second_pick[first_pick])
								))
							);
						},
						zip(range(23),first_picks)
					))
				)
			)
		)
	);

	//TODO: Show best strategy for given alliance
	return 0;
}

using Alliance_teams=tuple<Team,Team,Team>;
using Schedule_entry=pair<Alliance_teams,Alliance_teams>;

vector<string> split(string s){
	vector<string> r;
	stringstream ss;
	for(auto c:s){
		if(isblank(c)){
			if(ss.str().size()){
				r|=ss.str();
				ss.str("");
			}
		}else{
			ss<<c;
		}
	}
	if(ss.str().size()){
		r|=ss.str();
	}
	return r;
}

/*vector<Schedule_entry> parse_schedule(){
	vector<Schedule_entry> r;
	ifstream f("data/schedule.txt");
	while(f.good()){
		string s;
		getline(f,s);
		auto sp=split(s);
		if(sp.size()==6){
			auto teams=mapf([](auto s){ return atoi(s.c_str()); },sp);
			auto a1=take(3,teams);
			auto a2=skip(3,teams);
			r|=make_pair(a1,a2);
		}
	}
	return r;
}*/

int main(int argc,char **argv){
	//print_lines(parse_schedule());
	Team team=1425;
	auto set_team=[&](vector<string>& a)->int{
		assert(a.size());
		team=atoi(a[0].c_str());
		a=skip(1,a);
		return 0;
	};
	std::optional<std::string> path;
	auto set_path=[&](vector<string>& a)->int{
		assert(a.size());
		path=a[0];
		a=skip(1,a);
		return 0;
	};
	vector<tuple<
		const char *, //name
		const char *, //arg info
		const char *, //msg
		std::function<int(vector<string>&)>
	>> options{
		make_tuple(
			"--team","<TEAM NUMBER>","Create picklist for the given team number",
			std::function<int(vector<string>&)>(set_team)
		),
		make_tuple(
			"--file","<PATH>","Create picklist for the given team number",
			std::function<int(vector<string>&)>(set_path)
		)
	};
	
	auto help=[&](vector<string>&)->int{
		cout<<"Available arguments:\n";
		for(auto [arg,arg_info,msg,func]:options){
			(void)func;
			cout<<"\t"<<arg<<" "<<arg_info<<"\n";
			cout<<"\t\t"<<msg<<"\n";
		}
		exit(0);
	};
	options|=make_tuple(
		"--help","","Display this message",
		std::function<int(vector<string>&)>(help)
	);

	auto arg_list=args(argc,argv);
	arg_list=skip(1,arg_list); //skip name of the binary.

	while(arg_list.size()){
		bool found=0;
		for(auto [arg,arg_info,msg,func]:options){
			(void) arg_info;
			(void) msg;
			if(arg_list[0]==arg){
				found=1;
				arg_list=skip(1,arg_list);
				auto r=func(arg_list);
				if(r){
					cout<<"Failed.\n";
					return r;
				}
			}
		}
		if(!found){
			cerr<<"Unrecognized arg:"<<arg_list;
			return 1;
		}
	}
	return by_alliance(team,path);
}

