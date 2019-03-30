#include "util.h"
#include<cmath>
/*#include<iostream>
#include<vector>
#include<map>
#include<cassert>
#include<fstream>*/

using namespace std;

/*This is for exploring some of the data that we've scouted so far and seeing if we get interesting correlations
For example, does the defense flag mean anything to the opposing score (or that alliance's score)

And it would be interesting to see just what the correlation coefficients are between each of the columns

That might be a little bit wierd given that not all of the columns are necessarily numeric, but we'll see what happens

Also would like to have OPR for each of the teams as one of the things to correlate against
would also be nice to have match scores to comare against
*/

//start generic code

template<typename K,typename V>
map<K,V> to_map(vector<pair<K,V>> in){
	map<K,V> r;
	for(auto [k,v]:in){
		r[k]=v;
	}
	return r;
}

template<typename T>
T car(vector<T> v){
	assert(v.size());
	return v[0];
}

template<typename K,typename V>
set<K> keys(map<K,V> const& a){
	set<K> r;
	for(auto elem:a){
		r|=elem.first;
	}
	return r;
}

template<typename T>
set<pair<T,T>> unique_pairs(set<T> in){
	set<pair<T,T>> r;
	for(auto at=begin(in);at!=in.end();++at){
		auto at2=at;
		at2++;
		for(;at2!=in.end();++at2){
			r|=make_pair(*at,*at2);
		}
	}
	return r;
}

template<typename T>
vector<pair<T,T>> cross(vector<T> a){
	vector<pair<T,T>> r;
	for(auto elem:a){
		for(auto b:a){
			r|=make_pair(elem,b);
		}
	}
	return r;
}

double std_dev(vector<double> const& in){
	assert(in.size());
	auto u=mean(in);
	return sqrt(mean(mapf(
		[u](auto x){ return pow(u-x,2); },
		in
	)));
}

double covariance(vector<double> const& a,vector<double> const& b){
	auto ea=mean(a);
	auto eb=mean(b);
	return mean(mapf(
		[=](auto p){
			auto [a1,b1]=p;
			return (a1-ea)*(b1-eb);
		},
		zip(a,b)
	));
}

double correlation_coefficient(vector<double> const& a,vector<double> const& b){
	assert(a.size());
	assert(b.size());
	//could check that the standard deviations are nonzero
	auto da=std_dev(a);
	auto db=std_dev(b);
	if(da==0 || db==0) return 0;
	/*PRINT(da);	
	PRINT(db);*/
	assert(da>0);
	assert(db>0);
	return covariance(a,b)/(da*db);
}

template<typename T>
auto take(size_t n,set<T> s){
	return take(n,to_vec(s));
}

template<typename T>
T min(set<T> s){
	assert(s.size());
	return *min_element(begin(s),end(s));
}

template<typename T>
T max(set<T> s){
	return max(to_vec(s));
}

//start program-specific code

string unquote(string s){
	if(s.size()>1 && s[0]=='"' && s[s.size()-1]=='"'){
		return s.substr(1,s.size()-2);
	}
	return s;
}

vector<map<string,string>> load_csv(string filename){
	vector<map<string,string>> r;
	ifstream f(filename);
	assert(f.good());
	string s;
	getline(f,s);
	auto labels=split(s,',');
	labels=mapf(unquote,labels);

	while(f.good()){
		getline(f,s);
		if(s.size()){
			r|=to_map(zip(labels,split(s,',')));
		}
	}
	return r;
}

void autocorrelate_file(string filename){
	auto d=load_csv(filename);
	PRINT(d.size());
	//PRINT(d);
	PRINT(car(d).size());
	//auto k=keys(car(d));
	//PRINT(k);

	auto get_col=[=](auto key){
		return mapf([=](auto row){
			try{
				return stod(row[key]);
			}catch(...){
				//PRINT(row[key]);
				//nyi
				throw;
			}
		},d);
	};

	map<string,vector<double>> as_data;

	for(auto k:keys(car(d))){
		try{
			as_data[k]=get_col(k);
		}catch(...){
			cout<<"Non-numerical row:"<<k<<"\n";
		}
	}

	for(auto [k,data]:as_data){
		PRINT(k);
		auto s=to_set(data);
		//PRINT(min(s));
		//PRINT(max(s));
		PRINT(s);
	}

	//other interesting columns to add:
	//totals for the two alliances
	//match scores
	//averages for robots vs OPR for that robot
	//OPR-based expected score for each alliance

/*	group(
		[](auto x){
			return make_pair(x["Match"],x["Alliance"]);
		},
		d
	);*/

	vector<pair<double,pair<string,string>>> found;
	for(auto x:unique_pairs(keys(as_data))){
		//PRINT(x);
		auto const& a=as_data[x.first];//get_col(x.first);
		auto const& b=as_data[x.second]; //get_col(x.second);
		//PRINT(take(5,to_set(a)));
		//PRINT(take(5,to_set(b)));
		auto f=correlation_coefficient(a,b);
		found|=make_pair(f,x);
	}
	found=sorted(found);
	//print_lines(take(10,found));
	//print_lines(take(10,reversed(found)));
	print_lines(found);
}

using Team=int;
using Column_name=string;

template<typename A,typename B>
A first(pair<A,B>){
	nyi
}

Team get_team(map<string,string> m){
	auto s=[&](){
		auto f=m.find("Team");
		if(f!=m.end()){
			return f->second;
		}
		f=m.find("team");
		if(f!=m.end()){
			return f->second;
		}
		nyi
	}();
	return stoi(s);
}

double format_col(string col_name,string data){
	if(col_name=="climb"){
		if(data=="NULL") return 0;
		if(data=="P3") return 3;
		if(data=="P6") return 6;
		if(data=="P12") return 12;
		/*if(data=="NULL") return 0;
		if(data=="P3") return 1;
		if(data=="P6") return 2;
		if(data=="P12") return 3;*/
		nyi
	}
	return stod(data);
}

map<Column_name,map<Team,double>> averages(string filename){
	auto c=load_csv(filename);

	assert(c.size());
	auto columns=keys(c[0]);
	map<Column_name,map<Team,double>> r;
	for(auto col:columns){
		try{
			r[col]=to_map(mapf(
				[](auto p)->pair<Team,double>{
					auto values=seconds(p.second);
					return make_pair(p.first,mean(values));
				},
				group(
					[](auto x){ return x.first; },
					mapf(
						[&](auto x){
							try{
								return make_pair(
									get_team(x),//stoi(x["Team"]),
									format_col(col,x[col])//stod(x[col])
								);
							}catch(...){
								cout<<"Bad:"<<x["Team"]<<": \""<<x[col]<<"\"\n";
								throw;
							}
						},
						c
					)
				)
			));
		}catch(...){
			cout<<"Column failed:"<<col<<"\n";
			PRINT(mapf([=](auto x){ return x[col]; },c));
		}
	}
	return r;
}

template<typename T>
set<T> operator&(set<T> a,set<T> b){
	set<T> r;
	for(auto elem:a){
		if(b.count(elem)){
			r|=elem;
		}
	}
	return r;
}

template<typename A,typename B>
vector<A> firsts(vector<pair<A,B>> const& v){
	return mapf([](auto x){ return x.first; },v);
}

template<typename Func,typename T>
vector<T> sorted(Func f,vector<T> v){
	sort(begin(v),end(v),[=](auto a,auto b){ return f(a)<f(b); });
	return v;
}

void correlate_files(string file1,string file2){
	auto x=averages(file1);
	auto y=averages(file2);

	vector<pair<double,pair<string,string>>> out;
	for(auto [x_colname,x_data]:x){
		for(auto [y_colname,y_data]:y){
			vector<pair<double,double>> p;
			for(auto k:keys(x_data)&keys(y_data)){
				p|=make_pair(x_data[k],y_data[k]);
			}
			out|=make_pair(
				correlation_coefficient(firsts(p),seconds(p)),
				make_pair(x_colname,y_colname)
			);
		}
	}

	print_lines(reversed(sorted([](auto x){ return fabs(x.first); },out)));
}

int main(){
	//autocorrelate_file("data/pit_data_export_orore_2019.csv");

	/*correlate_files(
		"data/pit_data_export_orore_2019.csv",
		"data/match_data_export_orore_2019.csv"
	);*/

	/*correlate_files(
		"data/pit_data_export_orwil_2019.csv",
		"data/pit_data_export_orore_2019.csv"
	);*/

	/*correlate_files(
		"data/match_data_export_orwil_2019.csv",
		"data/match_data_export_orore_2019.csv"
	);*/

	/*correlate_files(
		"data/pit_data_export_orore_2019.csv",
		"data/2019OrOre1029.csv"
	);*/

	correlate_files(
		"data/pit_data_export_orwil_2019.csv",
		"data/2019OrWil1127.csv"
	);
	return 0;
}
