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

/* Connector_vector class connects the vector with data stored
 * for user to fetch data. It works exactly like iterator */
template<class T>
class Connector_vector
{
        T *my_data;
        unsigned long int current_index;

        public:
        Connector_vector()
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
	
	~Connector_vector()
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
                Connector_vector<T> *conn;
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
                                conn = new Connector_vector<T>[default_size];
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
                                conn = new Connector_vector<T>[default_size];
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
                Connector_vector<T>& begin()
                {
                        if(!clear_flag)
                                return *(conn + 0);
                        STL_error("Vector is empty");
                }
		
		/*return end connector*/
                Connector_vector<T>& end()
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

/* Doubly linked list structure and these nodes
 * will be sent to iterators/list_connectors for
 * storage */

template<class T>
struct Node
{
	struct Node *next;
	struct Node *prev;
	T data;
};

template<class T>
class Connector_list
{
	struct Node<T> *self_data;

	public:
	Connector_list()
	{
		self_data = NULL;
	}

	void set_data(struct Node<T> *data)
	{
		self_data = data;
	}

	void operator ++ (int x)
	{
		this->self_data = this->self_data->next;
	}

	void operator -- (int x)
	{
		this->self_data = this->self_data->prev;
	}

	T* operator * ()
	{
		return &(self_data->data);
	}

	~Connector_list()
	{
	}
};

namespace ash_list
{
	template<class T>
	class list
	{

		struct Node<T> *head;
		struct Node<T> *tail;
		Connector_list<T> *conn;
		Connector_list<T> *con_start_node;
		Connector_list<T> *con_end_node;

		public:
		list()
		{
			head = NULL;
			tail = NULL;
			conn = NULL;
			con_start_node = NULL;
			con_end_node = NULL;	
		}
		
		struct Node<T>* create_node_from_data(T data_copy)
		{
			struct Node<T> *new_node = NULL;
			new_node = new struct Node<T>();
			new_node->data = data_copy;
			return new_node;
		}

		Connector_list<T> *create_new_con_node(struct Node<T> *node_data)
		{
			Connector_list<T> *new_con = new Connector_list<T>();
			new_con->set_data(node_data);
			return new_con;
		}

		void push_back(T node_data)
		{
			struct Node<T> *tmp_node = NULL;
			struct Node<T> *traversal_node = NULL;
			Connector_list<T> *con_tmp_node = NULL;
			Connector_list<T> *con_trav_node = NULL;

			/* When we have single node then head and tail
			 * points to same location */
			if(!head) {
				head = new struct Node<T>();
				head->data = node_data;
				tail = head;
				head->next = NULL;
				head->prev = NULL;

				con_start_node = new Connector_list<T>();
				con_start_node->set_data(head);
				con_end_node = con_start_node;
				return;
			}
			/*Create new node*/
			tmp_node = create_node_from_data(node_data);
			
			/*When we have more than one nodes then traverse list and store it*/
			traversal_node = head;

			while(traversal_node->next != NULL)
			{
				traversal_node = traversal_node->next;
			}
			traversal_node->next = tmp_node;
			tmp_node->next = NULL;
			tmp_node->prev = traversal_node;
			tail = tmp_node;

			/*Allocating connector new chunk*/
			con_tmp_node = create_new_con_node(tail);
			con_end_node = con_tmp_node;		
		}
		
		Connector_list<T>& begin()
		{
			return *con_start_node;
		}

		Connector_list<T>& end()
		{
			return *con_end_node;
		}

		~list()
		{
			/*Delete in chain access each element*/
		}
	};
}
