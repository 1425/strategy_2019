#include<iostream>
#include<cassert>
#include<sstream>
#include<vector>
#include<fstream>
#include<array>
#include<map>
#include<set>
#include<functional>
#include<numeric>
#include<sys/stat.h>
#include<iterator>

#define nyi { std::cout<<"nyi "<<__LINE__<<"\n"; exit(44); }
#define PRINT(X) { std::cout<<""#X<<":"<<(X)<<"\n"; }

template<typename T,typename T2>
std::vector<T>& operator|=(std::vector<T> &a,T2 t){
	a.push_back(std::move(t));
	return a;
}

template<typename T>
std::vector<T>& operator|=(std::vector<T> &a,std::vector<T> const& b){
	a.insert(a.end(),b.begin(),b.end());
	return a;
}

template<typename T>
std::set<T>& operator|=(std::set<T>& a,std::vector<T> const& b){
	a.insert(b.begin(),b.end());
	return a;
}

template<typename T>
bool operator&(T t,std::set<T> const& s){
	return s.count(t)!=0;
}

template<typename T,size_t LEN>
std::ostream& operator<<(std::ostream& o,std::array<T,LEN> const& a){
	o<<"[ ";
	std::ostream_iterator<T> out_it(o," ");
	std::copy(begin(a),end(a),out_it);
	return o<<"]";
}

template<typename A,typename B>
std::ostream& operator<<(std::ostream& o,std::pair<A,B> const& a){
	return o<<"("<<a.first<<","<<a.second<<")";
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::vector<T> const& a){
	o<<"[ ";
	std::ostream_iterator<T> out_it(o," ");
	std::copy(begin(a),end(a),out_it);
	return o<<"]";
}

std::vector<int> range(unsigned lim){
	std::vector<int> r(lim);
	std::iota(begin(r),end(r),0);
	return r;
}

void write_file(std::string filename,std::string data){
	std::ofstream f(filename);
	assert(f.good());
	f<<data;
}

int min(int a,unsigned b){
	return std::min(a,(int)b);
}

char as_hex_digit(int i){
	assert(i>=0);
	assert(i<16);
	if(i<=9) return '0'+i;
	return 'a'+(i-10);
}

std::string as_hex(int i){
	assert(i>=0);
	assert(i<=255);
	return std::string()+as_hex_digit(i>>4)+as_hex_digit(i&0xf);
}

int rerange(int min_a,int max_a,int min_b,int max_b,int value){
	auto f=(value-min_a)/(max_a-min_a+0.0);
	auto out=min_b+f*(max_b-min_b);
	return out;
}

int sum(std::pair<int,int> p){
	return p.first+p.second;
}

template<typename T,size_t LEN>
T sum(std::array<T,LEN> const& a){
	return std::accumulate(begin(a),end(a),T{});
}

template<typename A,typename B>
std::vector<std::pair<A,B>> cross(std::vector<A> const& a,std::vector<B> const& b){
	std::vector<std::pair<A,B>> r;
	for(auto const& a1:a){
		for(auto const& b1:b){
			r|=std::make_pair(a1,b1);
		}
	}
	return r;
}

template<typename Func,typename T>
auto group(Func f,std::vector<T> v)->std::map<decltype(f(v[0])),std::vector<T>>{
	std::map<decltype(f(v[0])),std::vector<T>> r;
	for(auto elem:v){
		r[f(elem)]|=elem;
	}
	return r;
}

template<typename Func,typename T>
T filter(Func f,T const& t){
	T r;
	std::copy_if(begin(t),end(t),back_inserter(r),f);
	return r;
}

template<typename Func,typename T>
auto mapf(Func f,std::vector<T> v) -> std::vector< decltype(f(v[0])) > {
	std::vector<decltype(f(v[0]))> r(v.size());
	std::transform(begin(v),end(v),begin(r),f);
	return r;
}

template<typename Func,typename T,size_t LEN>
auto mapf(Func f,std::array<T,LEN> v) -> std::array< decltype(f(v[0])) , LEN> {
	std::array<decltype(f(v[0])),LEN> r;
	std::transform(begin(v),end(v),begin(r),f);
	return r;
}

#define MAP(F,V) mapf([&](auto elem){ return F(elem); },V)

template<typename T>
T max(std::vector<T> const& a){
	assert(a.size());
	return *std::max_element(begin(a),end(a));
}

template<typename T>
std::string as_string(T const& t){
	std::stringstream ss;
	ss<<t;
	return ss.str();
}

std::string first_word(std::string const& s){
	auto f=s.find(' ');
	if(f==std::string::npos) return s;
	return s.substr(0,f);
}

template<typename T>
std::string tag(std::string const& t,T const& body){
	std::stringstream ss;
	ss<<"<"<<t<<">";
	ss<<body;
	ss<<"</"<<first_word(t)<<">";
	return ss.str();
}

std::string html(auto a){ return tag("html",a); }
std::string table(auto a){ return tag("table",a); }
std::string tr(auto a){ return tag("tr",a); }
std::string td(auto a){ return tag("td",a); }
std::string head(auto a){ return tag("head",a); }
auto body(auto a){ return tag("body",a); }
auto title(auto a){ return tag("title",a); }
auto h1(auto a){ return tag("h1",a); }

std::vector<bool> bools(){
	std::vector<bool> r;
	r|=0;
	r|=1;
	return r;
}

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

struct Robot_capabilities{
	//by default, can do almost nothing.
	Climb_type climb_type=Climb_type::P3;
	//all the times are assumed to be >0.
	float climb_time=TELEOP_LENGTH;
	float ball_time=TELEOP_LENGTH,hatch_time=TELEOP_LENGTH;
};

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
	r.climb_time=rand()%30;
	r.ball_time=rand()%100;
	r.hatch_time=rand()%100;
	return r;
}

using Alliance_capabilities=std::array<Robot_capabilities,3>;

struct Robot_strategy{
	int balls,hatches;
	bool climb;
};

std::ostream& operator<<(std::ostream& o,Robot_strategy const& a){
	o<<"Robot_strategy(";
	o<<"b:"<<a.balls<<" h:"<<a.hatches<<" "<<a.climb;
	return o<<")";
}

vector<Robot_strategy> available_strategies(Robot_capabilities const& capabilities){
	vector<Robot_strategy> r;
	for(auto climb:bools()){
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
			for(auto balls:range(SPACES+1)){
				ss<<tag("th",balls);
			}
			return ss.str();
		}()
	);
	for(auto balls:range(SPACES+1)){
		stringstream s2;
		s2<<tag("th",as_string(balls));
		for(auto hatches:range(SPACES+1)){
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

int by_alliance(){
	Alliance_capabilities cap;
	PRINT(cap);
	auto as=available_strategies(cap);
	PRINT(as);
	auto m=mapf([=](auto x){ return points(cap,x); },as);
	PRINT(m);
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

