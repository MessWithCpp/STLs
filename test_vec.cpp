#include <iostream>
#include <string.h>
#include "ash_STL.h"
using namespace std;

class Target
{
	int var;
	public:
	Target()
	{
	}
	Target(int x)
	{
		this->var = x;
	}
	int get_var()
	{
		return var;
	}
};

void print_vector(ash_vec::vector<int> &v)
{
	cout << "Start printing..." << endl;
	Connector<int> c_st = v.begin();
	Connector<int> c_end = v.end();
	while(*c_st != *c_end)
	{
		cout << "Data from vector : " << *(*c_st) << endl;
		c_st++;
	}
		cout << "Data from vector : " << *(*c_st) << endl;
}

int main()
{
	ash_vec::vector<int>v;
	for(int i = 0 ; i < 20 ; i++)
	{
		v.push_back(i);
	}	

	print_vector(v);

	for(int i = 0 ; i < 10 ; i++)
	{
		v.pop_back();
	}

	print_vector(v);
	return 0;
}
