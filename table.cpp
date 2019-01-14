#include<functional>
#include<sys/stat.h>
#include "util.h"

using namespace std;

/*
This makes charts of how many points you getting scoring hatches vs scoring balls
*/

static const int SPACES=8+6*2;
static const int NULL_HATCHES=6;
static const int TELEOP_LENGTH=135;

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
	ss<<tr(
		tag("td colspan=2 rowspan=2","")+
		tag("th colspan="+as_string(SPACES+1),"Hatches")
	);
	ss<<tr(
		[](){
			stringstream ss;
			for(auto balls:range_st<SPACES+1>()){
				ss<<th(balls);
			}
			return ss.str();
		}()
	);
	for(auto balls:range_st<SPACES+1>()){
		stringstream s2;
		s2<<th(balls);
		for(auto hatches:range_st<SPACES+1>()){
			s2<<tag("td "+color(balls,hatches),f(balls,hatches));
		}
		ss<<tr(
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
		html(
			head(
				tag("title",title)
			)+
			body(
				h1(title)+
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

#ifdef TABLE_MAIN
int main(int argc,char **argv){
	vector<pair<const char *,std::function<int()>>> options{
		make_pair("--standard",standard),
		make_pair("--by_pieces",by_pieces)
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
#endif
