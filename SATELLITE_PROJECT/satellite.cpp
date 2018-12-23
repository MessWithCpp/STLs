#include <unordered_map>
#include <algorithm>
#include <string>
#include <istream>
#include <ostream>
#include <list>
#include <bits/stdc++.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#define CONFIG_FILE "satellite.conf"
#define TOTAL_DISTANCE 100

static unsigned int *min_speed = NULL;
using namespace std;
static bool all_sats_ready = false;
static map<int , struct Sat_info> *satdata_global;

void Sat_error(string err)
{
	cout << "Satellite_error : " << err << endl;
}

struct Sat_info
{
	string satellite_name;
	int satellite_id;
	unsigned int satellite_speed;
};

void Satellites_stopped(int signum)
{
	Sat_error("Satellites stopped so no more tracing needed");
	exit(1);
}

void register_sigalarm()
{
	signal(SIGALRM , Satellites_stopped);
}

class Satellite_base
{
	protected:
	string satname;
	int satid;
	unsigned int speed;/*In km per sec*/
	unsigned int start_time;/*In sec*/
	unsigned int end_time;
	unsigned int current_loc;/*In km from 0 to ahead*/
	unsigned int max_distance;

	public:
	Satellite_base()
	{
		start_time = 0;
		current_loc = 0;
		max_distance = TOTAL_DISTANCE;
	}
	virtual string get_satname()
	{
		return satname;
	}
	virtual int get_satid()
	{
		return satid;
	}
	virtual unsigned int get_current_loc()
	{
		return current_loc;
	}
	virtual void update_location(time_t time_passed)
	{
		current_loc = speed*time_passed;
		if(current_loc > max_distance)
			current_loc = max_distance;
	}
	virtual void setup(int id){}
	virtual ~Satellite_base()
	{
	}
};

struct Sat_info get_sat_info_from_global(int id)
{
	struct Sat_info satinfos;
	map<int , struct Sat_info>::iterator it;
	it = (*satdata_global).find(id);
	satinfos = it->second;
	return satinfos;
}

class Satellite : public Satellite_base
{

	public:
	Satellite(int sat_id) : Satellite_base()
	{
		this->satid = sat_id;
	}
	void setup(int id)
	{
		struct Sat_info satinfos;
		satinfos = get_sat_info_from_global(satid);
		this->speed = satinfos.satellite_speed;
		this->satname = satinfos.satellite_name;
	}
	~Satellite()
	{
	}
};
class Satellite_tracker
{
	static map<int , struct Sat_info> satdata;
	static map<int , struct Sat_info>::iterator satdata_it;
	static list<Satellite_base*> sat_list;
	Satellite_base *sats;
	
	public:
	Satellite_tracker()
	{
		this->store_sat_info_from_file();
	}
	void store_sat_info_from_file()
	{
		fstream sat_file;
		bool data_found_flag = false;
		struct Sat_info sat_struct;
		int sat_id , sat_speed , colon_index;
		string sat_name , line_data , sat_id_str , sat_speed_str;
		string sat_filename = CONFIG_FILE;
		sat_file.open(sat_filename.c_str());
		while(sat_file >> line_data)
		{
			if(line_data.find("SATELLITE_NAME") != string::npos)
			{
				colon_index = line_data.find(":");
				sat_name = line_data.substr((colon_index + 1) , (line_data.length() - 1));
				sat_struct.satellite_name = sat_name;
			}
			else if(line_data.find("SATELLITE_ID") != string::npos)
			{
				colon_index = line_data.find(":");
				sat_id_str = line_data.substr((colon_index + 1) , (line_data.length() - 1));
				sat_id = stoi(sat_id_str);
				sat_struct.satellite_id = sat_id;
			}
			else if(line_data.find("SATELLITE_SPEED") != string::npos)
			{
				data_found_flag = true;
				colon_index = line_data.find(":");
				sat_speed_str = line_data.substr((colon_index + 1) , (line_data.length() - 1));
				sat_speed = stoi(sat_speed_str);
				sat_struct.satellite_speed = sat_speed;
			}
			else
			{
			}
			if(data_found_flag) {
				satdata[sat_id] = sat_struct;
				memset(&sat_struct , 0 , sizeof(Sat_info));
				data_found_flag = false;
			}
		}
		satdata_global = &satdata;
	}

	void register_satellites()
	{
		int satid_loc = 0 , index = 0;
		int satnums = satdata.size();
		unsigned int sat_speeds[satnums];
		map<int , struct Sat_info>::iterator it;
		for(it = satdata.begin() ; it != satdata.end() ; it++)
		{
			sat_speeds[index] = it->second.satellite_speed;
			satid_loc = it->first;
			sats = new Satellite(satid_loc);
			sats->setup(satid_loc);
			sat_list.push_back(sats);
			index++;
		}

		/*Calculate min speed out of all satellites*/
		min_speed = min_element(sat_speeds , sat_speeds + (sizeof(sat_speeds)/sizeof(sat_speeds[0])-1));			
	}

	void start_satellites()
	{
		/*We have created virtual satellite movement using timer*/
		list<Satellite_base*>::iterator sat_it;
		struct itimerval it_val,remaining_time;
		time_t time_spent;
		unsigned int max_sat_time = TOTAL_DISTANCE/(*min_speed);//Need to chnge this logic speed here taken is 1km/s
		register_sigalarm();
		it_val.it_value.tv_sec = max_sat_time;
		it_val.it_value.tv_usec = 0;
		it_val.it_interval = it_val.it_value;

		if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
			perror("error calling setitimer()");
			exit(1);
		}
		all_sats_ready = true;
		while(1)
		{
			if (getitimer(ITIMER_REAL , &remaining_time) == -1) {
				perror("error calling getitimer()");
				exit(1);
			}
			time_spent = it_val.it_value.tv_sec - remaining_time.it_value.tv_sec;
			for(sat_it = sat_list.begin() ; sat_it != sat_list.end() ; sat_it++)
			{
				(*sat_it)->update_location(time_spent);
			}
		}
	}

	static struct Sat_info get_sat_info(int sat_ids)
	{
		struct Sat_info satinf;
		satdata_it = satdata.find(sat_ids);
		satinf = satdata_it->second;
		return satinf;
	}

	static void track_satellites()
	{
		list<Satellite_base*>::iterator sat_it;
		if(all_sats_ready == true) {
			for(sat_it = sat_list.begin() ; sat_it != sat_list.end() ; sat_it++)
			{
				cout << "Satellite_name : " << (*sat_it)->get_satname() << ", Satellite_id : " << (*sat_it)->get_satid() << ", Satellite_loc: "<< (*sat_it)->get_current_loc() << endl;
			}
			cout << "-------------------------------------------------------------------------------" << endl;
		}
	}

	~Satellite_tracker()
	{
	}	
};

map<int , struct Sat_info> Satellite_tracker::satdata;
map<int , struct Sat_info>::iterator Satellite_tracker::satdata_it;
list<Satellite_base*> Satellite_tracker::sat_list;

void* location_tracker(void *args)
{
	while(1)
	{
		Satellite_tracker::track_satellites();
		sleep(5);
	}
}

int main()
{
	Satellite_tracker stracer;
	pthread_t tid;
	int index;
	stracer.register_satellites();
	/*Starting a thread to read satellite locations*/
	pthread_create(&tid , NULL , location_tracker , NULL);
	/*Start sattelites*/
	stracer.start_satellites();
	return 0;
}
