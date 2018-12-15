#include <iostream>
#include <string.h>
#include "ash_STL.h"

#define OBJECT_LIMITATION 50;

using namespace std;

class Target
{
        static int visible;
        int var;
        public:
        Target()
        {
                visible++;
                var = visible;
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

int Target::visible = 0;

void print_vector(ash_vec::vector<Target> &v)
{
        cout << "Start printing..." << endl;
        Connector_vector<Target> c_st = v.begin();
        Connector_vector<Target> c_end = v.end();

        while(*c_st != *c_end)
        {
                cout << "Data from vector : " << (*c_end)->get_var() << endl;
                c_end--;
        }
                cout << "Data from vector : " << (*c_end)->get_var() << endl;
}
int main()
{
#if 0
        ash_vec::vector<int>v;
        /*Pushing data in*/
        for(int i = 0 ; i < 20 ; i++)
        {
                v.push_back(i);
        }
        print_vector(v);

        /*Poping data out*/
        for(int i = 0 ; i < 5 ; i++)
        {
                v.pop_back();
        }
        print_vector(v);
#endif
        int obj_count = OBJECT_LIMITATION;
        Target *tar = new Target[obj_count];
        ash_vec::vector<Target> v;
        int index = 0;
	
	while(index != obj_count)
        {
                v.push_back(tar[index++]);
        }
        print_vector(v);
        /*Poping data out*/
        for(int i = 0 ; i < 10 ; i++)
        {
                v.pop_back();
        }
        print_vector(v);

        return 0;
}
