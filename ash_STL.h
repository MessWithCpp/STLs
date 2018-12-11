/* This software is designed inorder to learn STL basics and 
 * should not be used for any firmare handling. The learning 
 * package includes vectors , list , map and list with minimal 
 * base required code*/

/* Below is basic layout of vector.
 * How connector i.e. iterator and vector works together
 * You can make changes as required in code for furthur
 * learning */

#include <string>
#include <iostream>
using namespace std;

void STL_error(string error)
{
	cout << "Error :" << error << endl;
	abort();
}

/* Connector class connects the vector with data stored 
 * for user to fetch data. It works exactly like iterator */
template<class T>
class Connector
{
	T *start;
	T *last;
	unsigned long int current_index;
	int i_am_for_last;

	public:
	Connector()
	{
		current_index = 0;
		i_am_for_last = 0;
	}
	/*Begin of link*/
	void set_start(T* init_con)
	{
		start = init_con;
	}

	/*End of link*/
	void set_last(T* last_con)
	{
		last = last_con;
	}

	void set_me_last()
	{
		i_am_for_last = 1;
	}

	T* operator * ()
	{
		if(i_am_for_last)
			return last;
		else		
			return (start+current_index);		
	}

	void operator ++(int x)
	{
		current_index++;
	}

	~Connector()
	{
	}
};

class Reference_counter
{
	int counter;
	public:
	Reference_counter()
	{
		counter = 0;
	}
	void up_count()
	{
		counter++;
	}
	void down_count()
	{
		counter--;
	}
	int get_count()
	{
		return counter;
	}
	~Reference_counter()
	{
	}
};

namespace ash_vec
{
	template<class T>
	class vector
	{
		T *base;
		T *new_base;
		Connector<T> *conn; 
		Connector<T> *conn_last;
		Reference_counter *ref;
		unsigned long int default_size;
		unsigned long int current_index;
		int clear_flag;
		
		public:
		vector()
		{
			clear_flag = 0;
			current_index = 0;
			default_size = 500000;
			try {
				conn = new Connector<T>();
				base = new T[default_size];
				ref = new Reference_counter();
			}
			catch(std::bad_alloc &e) {
				STL_error(e.what());				
			}
			/*Increase reference count by 1*/
			ref->up_count();
		}

		/* We cant change the addresses of pointers directly
		 * as it is not permissible.It means we cant write
		 * &base = &vec.base....
		 * So try to pass the vectors by reference and not
		 * by value as it may lead to undefined behavior as
		 * we are doing shallow copy here in below code*/

		vector(const vector &vec)
		{
			base = vec.base;
			new_base = vec.new_base;
			conn = vec.conn;
			conn_last = vec.conn_last;
			ref = vec.ref;
			default_size = vec.default_size;
			current_index = vec.current_index;
			clear_flag = vec.clear_flag;

			/*Putting up the reference count*/
			ref->up_count();
		}

		/*Resize functionality here*/
		void resize_vector(T preserved_data)
		{
			int loc_index = 0;
			int default_orig = default_size;
			default_size = default_size * 2;
			try {
				new_base = new T[default_size];
			}
			catch(std::bad_alloc &e) {
				STL_error(e.what());
			}

			while(loc_index != default_orig)
			{
				new_base[loc_index] = base[loc_index];
				loc_index++;
			}
			delete[] base;
			try {
				base = new T[default_size];
			}
			catch(std::bad_alloc &e)
			{
				STL_error(e.what());
			}
			loc_index = 0;
			while(loc_index != default_size)
			{
				base[loc_index] = new_base[loc_index];
				loc_index++;
			}
			delete[] new_base;

			/*Calling back push_back() to fill back unfilled data*/
			push_back(preserved_data);
		}

		/*Push back data in vector bucket here*/
		void push_back(T data)
		{
			unsigned long int tmp_index = 0;
			if(current_index != default_size)
			{
				base[current_index] = data;
				if(!current_index)
				{
					conn->set_start(base);
					conn->set_last(base);
				}
				conn->set_start(base);
				conn->set_last(base + current_index);
				current_index++;
				return;
			}
			resize_vector(data);
		}

		/*Clear whole vector*/
		void clear()
		{
			clear_flag = 1;
			delete[] base;
		}

		/*Pop back data from end*/
		void pop_back()
		{
			if(current_index >= 1)
			{
				if(current_index == 1)
				{
					clear();
					return;
				}
				conn->set_last(base + (current_index - 2));
				current_index--;
				return;			
			}
			return;
		}

		/*index operator to get data*/
		T operator [] (int target_index)
		{
			if(target_index < current_index)
				return base[target_index];
			else
				STL_error("No such index");
		}

		/*return begin connector*/
		Connector<T>& begin()
		{
			if(!clear_flag)
				return *conn;		
			STL_error("Vector is empty");
		}

		/*return end connector*/
		Connector<T>& end()
		{
			if(!clear_flag) {
				conn_last = new Connector<T>(*conn);
				conn_last->set_me_last();
				return *conn_last;
			}
			STL_error("Vector is empty");
		}

		/*free mem in destructor*/
		~vector()
		{
			/*Putting down the reference count for object*/
			ref->down_count();
			if(!(ref->get_count())) {
				delete[] base;
				delete conn;
				delete ref;
			}
		}
	};
}
