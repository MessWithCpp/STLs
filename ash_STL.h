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
#define DEFAULT_VECTOR_SIZE 5000

void STL_error(string error)
{
        cout << "Error :" << error << endl;
}

/* Connector class connects the vector with data stored
 * for user to fetch data. It works exactly like iterator */
template<class T>
class Connector
{
        T *my_data;
        unsigned long int current_index;

        public:
        Connector()
        {
                current_index = 0;
        }

        /*Setting data of vector in connector as
          as each connector stands for each object*/

        void set_data(T* data)
        {
                my_data = data;
        }

        T* operator * ()
        {
                        return my_data;
        }

        void operator ++(int x)
        {
                my_data = my_data + 1;
        }

        void operator --(int x)
        {
                my_data = my_data - 1;
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
                bool i_am_resized;
                T *base;
                T *new_base;
                Connector<T> *conn;
                Reference_counter *ref;
                unsigned long int default_size;
                unsigned long int current_index;
                int clear_flag;

                public:
                vector()
                {
                        i_am_resized = false;
                        clear_flag = 0;
                        current_index = 0;
                        default_size = DEFAULT_VECTOR_SIZE;
                        try {
                                conn = new Connector<T>[default_size];
                                base = new T[default_size];
                                ref = new Reference_counter();
                        }
                        catch(std::bad_alloc &e) {
                                STL_error(e.what());
                        }
                        /*Increase reference count by 1*/
                        ref->up_count();
                }
		vector(const vector &vec)
                {
                        base = vec.base;
                        new_base = vec.new_base;
                        conn = vec.conn;
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
                        /*switch on resize flag*/
                        i_am_resized = true;
                        int loc_index = 0;
                        int default_orig = default_size;
                        default_size = default_size * 2;
                        try {
                                /*Resize connector*/
                                delete[]conn;
                                conn = new Connector<T>[default_size];
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
		
		/*Update connectors till current_index with new heap address*/
                void update_connectors_after_resize()
                {
                        unsigned long int tmp_index = 0;
                        for(tmp_index = 0 ; tmp_index < current_index ; tmp_index++)
                                (conn + tmp_index)->set_data(base + tmp_index);
                        i_am_resized = false;
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
                                        (conn + current_index)->set_data(base + current_index);
                                }
                                /*If resized then new heap addresses need to be updated in iterators*/
                                if(i_am_resized == true)
                                        update_connectors_after_resize();

                                (conn + current_index)->set_data(base + current_index);
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
                                return *(conn + 0);
                        STL_error("Vector is empty");
                }
		
		/*return end connector*/
                Connector<T>& end()
                {
                        if(!clear_flag)
                                return *(conn + current_index - 1);
                        STL_error("Vector is empty");
                }

                /*free mem in destructor*/
                ~vector()
                {
                        /*Putting down the reference count for object*/
                        ref->down_count();
                        if(!(ref->get_count())) {
                                delete[] base;
                                delete[] conn;
                                delete ref;
                        }
                }
        };
}
