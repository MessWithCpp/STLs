#include <iostream>
#include "ash_STL.h"
using namespace std;

int main()
{
	ash_list::list<int> ldata;
	int arr[20]={0};
	int index;
	for(index = 0 ; index < sizeof(arr)/sizeof(arr[index]) ; index++)
		arr[index] = index;

	index = 0;
	for(index = 0 ; index < sizeof(arr)/sizeof(arr[index]) ; index++)
		ldata.push_back(arr[index]);

	Connector_list<int> con_start = ldata.begin();
	Connector_list<int> con_end = ldata.end();
	
	while(*con_start != *con_end) {
		cout << "Data : " << *(*con_start) << endl;
		con_start++;
	}
	return 0;
}
