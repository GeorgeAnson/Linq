#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
using namespace std;
#include"Linq.h"

struct Person
{
	int age;
	string name;
	string address;
};

void TestLinqCpp()
{
	using namespace linq;
	vector<Person> v = {
		{ 21,  "a", "11" },
		{ 22,  "b", "22" },
		{ 23,  "c", "33" },
		{ 24,  "d", "44" },
		{ 25,  "e", "55" },
		{ 26,  "f", "66" },
	};
	vector<int> vec = {1,2,3,4,4};
	int cunt = from(vec).where([](const int& x){return x > 3; }).count();
	cout << cunt << endl;
	auto sm = from(vec).sum();
	cout << sm << endl;
	int av = from(vec).average();
	int min = from(vec).min();
	int max = from(vec).max();
	cout << av << "  " << min << " " << max << "  " << endl;
	// from(v).where([](const Person& p) {return p.age > 22; }).count();
	int b=from(vec).count([](const int& a){return a <= 3; });
	cout << b << endl;
}
int main()
{
	TestLinqCpp();
	return 0;
}