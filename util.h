#ifndef UTIL_H
#define UTIL_H

#include<vector>
#include<set>
#include<array>
#include<iostream>
#include<iterator>
#include<numeric>
#include<cassert>
#include<fstream>
#include<climits>
#include<map>
#include<algorithm>
#include<sstream>

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
std::set<T>& operator|=(std::set<T>& a,T t){
	a.insert(t);
	return a;
}

template<typename T>
std::vector<T> to_vec(std::set<T> a){
	return std::vector<T>{begin(a),end(a)};
}

template<typename T>
std::set<T> operator|(std::set<T> a,std::set<T> const& b){
	a.insert(begin(b),end(b));
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
	/*
	Not sure why this commented part doesn't work sometimes.
	std::ostream_iterator<T> out_it(o," ");
	std::copy(begin(a),end(a),out_it);*/
	for(auto elem:a){
		o<<elem<<" ";
	}
	return o<<"]";
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::set<T> const& a){
	o<<"{ ";
	std::ostream_iterator<T> out_it(o," ");
	std::copy(begin(a),end(a),out_it);
	return o<<"}";
}

template<typename T>
std::vector<T> range(T lim){
	std::vector<T> r(lim);
	std::iota(begin(r),end(r),0);
	return r;
}

template<size_t LEN>
std::array<size_t,LEN> range_st(){
	std::array<size_t,LEN> r;
	std::iota(begin(r),end(r),0);
	return r;
}

template<typename T>
std::vector<T> range(T start,T lim){
	assert(lim>=start);
	std::vector<T> r(lim-start);
	std::iota(begin(r),end(r),start);
	return r;
}

void write_file(std::string filename,std::string data);
int min(int a,unsigned b);
char as_hex_digit(int);
std::string as_hex(int);
int rerange(int min_a,int max_a,int min_b,int max_b,int value);
int sum(std::pair<int,int>);

template<typename T,size_t LEN>
T sum(std::array<T,LEN> const& a){
	return std::accumulate(begin(a),end(a),T{});
}

template<typename T>
T sum(std::vector<T> v){
	return std::accumulate(begin(v),end(v),T{});
}

template<typename T>
T mean(std::vector<T> v){
	assert(v.size());
	return sum(v)/v.size();
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

template<typename Func,typename T,size_t LEN>
std::vector<T> filter(Func f,std::array<T,LEN> const& t){
	std::vector<T> r;
	std::copy_if(begin(t),end(t),back_inserter(r),f);
	return r;
}

template<typename Func,typename T>
std::set<T> filter(Func f,std::set<T> const& t){
	std::set<T> r;
	std::copy_if(begin(t),end(t),inserter(r,r.end()),f);
	return r;
}

template<typename Func,typename T>
std::multiset<T> filter(Func f,std::multiset<T> const& t){
	std::multiset<T> r;
	std::copy_if(begin(t),end(t),inserter(r,r.end()),f);
	return r;
}

template<typename T>
T id(T t){ return t; }

#define FILTER(F,X) filter([&](auto x){ return F(x); },X)

template<typename Func,typename T>
auto mapf(Func f,std::vector<T> v) -> std::vector< decltype(f(v[0])) > {
	std::vector<decltype(f(v[0]))> r(v.size());
	std::transform(begin(v),end(v),begin(r),f);
	return r;
}

template<typename Func,typename K,typename V>
auto mapf(Func f,std::map<K,V> v) -> std::vector< decltype(f(*begin(v))) > {
	std::vector<decltype(f(*begin(v)))> r(v.size());
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
class Nonempty_vector{
	std::vector<T> data;
};

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

std::string first_word(std::string const&);

template<typename T>
std::string tag(std::string const& t,T const& body){
	std::stringstream ss;
	ss<<"<"<<t<<">";
	ss<<body;
	ss<<"</"<<first_word(t)<<">";
	return ss.str();
}

template<typename T>
std::string html(T a){ return tag("html",a); }

template<typename T>
std::string table(T a){ return tag("table",a); }

template<typename T>
std::string tr(T a){ return tag("tr",a); }

template<typename T>
std::string td(T a){ return tag("td",a); }

template<typename T>
std::string th(T a){ return tag("th",a); }

template<typename T>
std::string head(T a){ return tag("head",a); }

template<typename T>
auto body(T a){ return tag("body",a); }

template<typename T>
auto title(T a){ return tag("title",a); }

template<typename T>
auto h1(T a){ return tag("h1",a); }

template<typename T>
auto small(T a){ return tag("small",a); }

std::string join(std::vector<std::string>);
std::vector<bool> bools();

#define RM_REF(X) typename std::remove_reference<X>::type

template<typename T>
void print_lines(T const& a){
	/*std::ostream_iterator<RM_REF(decltype(*begin(a)))> out_it(std::cout,"\n");
	std::copy(begin(a),end(a),out_it);*/
	for(auto const& elem:a){
		std::cout<<elem<<"\n";
	}
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::optional<T> const& a){
	if(a) return o<<*a;
	return o<<"NULL";
}

template<typename K,typename V>
std::map<K,V> without_key(std::map<K,V> a,K key){
	a.erase(a.find(key));
	return a;
}

template<typename T>
std::vector<T> sorted(std::vector<T> a){
	std::sort(begin(a),end(a));
	return a;
}

template<typename T>
std::vector<T> reversed(std::vector<T> a){
	std::reverse(begin(a),end(a));
	return a;
}

template<typename T>
std::vector<std::pair<size_t,T>> enumerate_from(size_t i,std::vector<T> const& a){
	std::vector<std::pair<size_t,T>> r;
	for(auto elem:a){
		r|=make_pair(i++,elem);
	}
	return r;
}

template<typename Func,typename T>
T take_first(Func f,std::multiset<T> m){
	for(auto elem:m){
		if(f(elem)){
			return elem;
		}
	}
	assert(0);
}

#define RM_CONST(X) typename std::remove_const<X>::type

#define ELEM(X) RM_CONST(RM_REF(decltype(*begin(X))))

template<typename Func,typename T>
auto mapf(Func f,std::multiset<T> a)->
	std::vector<decltype(f(*begin(a)))>
{
	std::vector<decltype(f(*begin(a)))> r;
	for(auto elem:a){
		r|=f(elem);
	}
	return r;
}

template<typename Func,typename T>
auto mapf(Func f,std::set<T> a)->
	std::vector<decltype(f(*begin(a)))>
{
	std::vector<decltype(f(*begin(a)))> r;
	for(auto elem:a){
		r|=f(elem);
	}
	return r;
}

template<typename T>
T mode(std::vector<T> v){
	assert(v.size());
	auto m=std::multiset<T>(begin(v),end(v));
	auto max_count=max(mapf([&](auto x){ return m.count(x); },m));
	return take_first([&](auto x){ return m.count(x)==max_count; },m);
}

template<typename T>
std::vector<T> non_null(std::vector<std::optional<T>> a){
	std::vector<T> r;
	for(auto elem:a){
		if(elem){
			r|=*elem;
		}
	}
	return r;
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T>& a,T t){
	a.insert(t);
	return a;
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::multiset<T> const& a){
	o<<"{ ";
	for(auto elem:a){
		o<<elem<<" ";
	}
	return o<<"}";
}

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,std::map<K,V> const& a){
	o<<"{ ";
	for(auto elem:a){
		o<<elem<<" ";
	}
	return o<<"}";
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T>& a,std::multiset<T> b){
	a.insert(b.begin(),b.end());
	return a;
}

template<typename T>
std::multiset<T> operator+(std::multiset<T> a,std::multiset<T> b){
	a|=b;
	return a;
}

template<typename A,typename B>
std::vector<std::pair<A,B>> zip(std::vector<A> const& a,std::vector<B> const& b){
	return mapf(
		[&](auto i){ return std::make_pair(a[i],b[i]); },
		range(min(a.size(),b.size()))
	);
}

template<typename A,typename B,size_t LEN>
std::array<std::pair<A,B>,LEN> zip(std::array<A,LEN> const& a,std::array<B,LEN> const& b){
	std::array<std::pair<A,B>,LEN> r;
	std::transform(
		begin(a),end(a),begin(b),begin(r),
		[](auto a1,auto b1){ return std::make_pair(a1,b1); }
	);
	return r;
}

template<typename Func,typename K,typename V>
auto map_values(Func f,std::map<K,V> const& in){
	std::map<K,decltype(f(in.begin()->second))> r;
	for(auto [k,v]:in){
		r[k]=f(v);
	}
	return r;
}

#define MAP_VALUES(f,v) map_values([&](auto x){ return f(x); },v)

template<typename K,typename V>
std::vector<V> values(std::map<K,V> const& a){
	return mapf([](auto x){ return x.second; },a);
}

template<typename T>
std::multiset<T> flatten(std::vector<std::multiset<T>> a){
	std::multiset<T> r;
	for(auto elem:a){
		r|=elem;
	}
	return r;
}

void indent_by(unsigned indent);

template<typename T>
void print_r(unsigned indent,T const& t){
	indent_by(indent);
	std::cout<<t<<"\n";
}

template<typename A,typename B>
void print_r(unsigned indent,std::pair<A,B> const& p){
	indent_by(indent);
	std::cout<<"pair\n";
	print_r(indent+1,p.first);
	print_r(indent+1,p.second);
}

template<typename K,typename V>
void print_r(unsigned indent,std::map<K,V> const& a){
	indent_by(indent);
	std::cout<<"map\n";
	for(auto elem:a){
		print_r(indent+1,elem);
	}
}

template<typename T>
void print_r(unsigned indent,std::vector<T> const& a){
	indent_by(indent);
	std::cout<<"vector\n";
	for(auto elem:a){
		print_r(indent+1,elem);
	}
}

template<typename T>
void print_r(T t){
	print_r(0,t);
}

template<typename T>
std::set<T> to_set(std::vector<T> const& v){
	return std::set<T>(begin(v),end(v));
}

template<typename T,size_t LEN>
std::array<T,LEN> sorted(std::array<T,LEN> a){
	std::sort(begin(a),end(a));
	return a;
}

template<typename T>
std::set<T> choose(size_t num,std::set<T> a){
	if(num==0){
		return {};
	}
	assert(a.size());
	auto other=choose(num-1,a);
	auto left=filter([other](auto x){ return other.count(x)==0; },a);
	return other|std::set<T>{to_vec(left)[rand()%left.size()]};
}

template<typename T1,typename T2>
std::vector<T1>& operator|=(std::vector<T1>& a,std::vector<T2> const& b){
	for(auto elem:b){
		a|=elem;
	}
	return a;
}

template<typename T>
std::set<T>& operator-=(std::set<T> &a,T const& t){
	auto it=a.find(t);
	if(it!=a.end()){
		a.erase(it);
	}
	return a;
}

template<typename T,typename T2>
std::set<T>& operator-=(std::set<T> &a,T2 b){
	auto it=a.find(T{b});
	if(it!=a.end()){
		a.erase(it);
	}
	return a;
}

template<typename T>
std::set<T>& operator-=(std::set<T> &a,std::set<T> const& b){
	for(auto elem:b){
		a-=elem;
	}
	return a;
}

std::vector<std::string> split(std::string const& s,char target);

template<typename T>
std::vector<std::tuple<T,T,T>> cross3(std::array<std::vector<T>,3> in){
	std::vector<std::tuple<T,T,T>> r;
	for(auto a:in[0]){
		for(auto b:in[1]){
			for(auto c:in[2]){
				r|=make_tuple(a,b,c);
			}
		}
	}
	return r;
}

template<typename A,typename B>
std::pair<A,B> operator+(std::pair<A,B> a,std::pair<A,B> b){
	return std::make_pair(a.first+b.first,a.second+b.second);
}

template<typename T>
T sum(std::tuple<T,T,T> t){
	return std::get<0>(t)+std::get<1>(t)+std::get<2>(t);
}

template<typename Func,typename K,typename V>
std::map<K,V> filter(Func f,std::map<K,V> a){
	std::map<K,V> r;
	for(auto p:a){
		if(f(p)){
			r[p.first]=p.second;
		}
	}
	return r;
}

template<typename T>
std::vector<T> take(size_t lim,std::vector<T> in){
	if(in.size()<=lim) return in;
	return std::vector<T>(begin(in),begin(in)+lim);
}

template<typename T>
T mean_else(std::vector<T> v,T t){
	if(v.size()){
		return mean(v);
	}
	return t;
}

template<typename T>
std::vector<T> skip(size_t i,std::vector<T> v){
	//note: this is implemented in a very slow way.
	for(auto _:range(i)){
		(void)_;
		if(v.size()){
			v.erase(v.begin());
		}
	}
	return v;
}

template<typename A,typename B>
std::vector<B> seconds(std::vector<std::pair<A,B>> a){
	return mapf([](auto x){ return x.second; },a);
}

int atoi(std::string const&);
std::vector<std::string> args(int argc,char **argv);

template<typename K,typename V>
std::vector<std::pair<K,V>> to_vec(std::map<K,V> a){
	std::vector<std::pair<K,V>> r;
	for(auto p:a){
		r|=p;
	}
	return r;
}

size_t sum(std::multiset<bool> const&);
float mean(std::multiset<bool> const&);
std::string pop(std::vector<std::string>&);

#endif
