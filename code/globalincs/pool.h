#pragma once

#include "globalincs/pstypes.h"
#include <tl/optional.hpp>
#include <csignal>
//#include <vector>

using  tl::optional;
struct pool_index{
	private:
	int index;
	int generation;
	bool is_null = true;
	public:
	pool_index(){ nullify();};
	pool_index(int i,int g,bool b = false){
		if(b){ nullify();
		}
		else{
			set(i,g);
		}
	};
	void set(int i,int g){
		is_null = false;
		index = i;
		generation = g;
	}
	void nullify()
	{
		index = -1;
		generation = -1;
		is_null=true;
	};
	bool operator==(pool_index &rhs) const {
		if(is_null && rhs.is_null)
			return true;
		if(is_null || rhs.is_null)
			return false;
		return (index == rhs.index && generation == rhs.generation);
	};
	pool_index& operator=(pool_index const&rhs) {
		if(rhs.is_null)
			nullify();
		else
			set(rhs.index, rhs.generation);
		return *this;
	};
	int i() const { return index;};
	int g() const { return generation;};
	bool null() const { return is_null;};
};



template  <typename T>
struct gEntry {
	int generation;
	optional<T> stored;

};
class PoolExceptionStaleIndex : public std::runtime_error
{
public:
	explicit PoolExceptionStaleIndex(const std::string& msg) : std::runtime_error(msg) {}
	~PoolExceptionStaleIndex() noexcept override = default;
};
class PoolExceptionNullValue : public std::runtime_error
{
public:
	explicit PoolExceptionNullValue(const std::string& msg) : std::runtime_error(msg) {}
	~PoolExceptionNullValue() noexcept override = default;
};
class PoolExceptionNullIndex : public std::runtime_error
{
public:
	explicit PoolExceptionNullIndex(const std::string& msg) : std::runtime_error(msg) {}
	~PoolExceptionNullIndex() noexcept override = default;
};
class PoolExceptionOutOfRange : public std::runtime_error
{
public:
	explicit PoolExceptionOutOfRange(const std::string& msg) : std::runtime_error(msg) {}
	~PoolExceptionOutOfRange() noexcept override = default;
};
class PoolExceptionFull : public std::runtime_error
{
public:
	explicit PoolExceptionFull(const std::string& msg) : std::runtime_error(msg) {}
	~PoolExceptionFull() noexcept override = default;
};




using std::vector;
template  <typename T>
//rules:
//Caps can only ever be set at the current size or smaller
//shrinking the vector would lead to forgetting the generation counts and then potential slot reuse if allowed to expand later.
class SCP_Pool {
	/**************************************************************************
	 * Private data members
	 *
	 *************************************************************************/
	vector<gEntry<T>> storage;
	vector<size_t> known_empty;

	bool capped = false;
	size_t cap = 0;

	/**************************************************************************
	 * Private member functions
	 *
	 *************************************************************************/

	/**
	 * @brief
	 * Internal function to expand the storage list with a new object
	 * for use when there are no empty slots and the storage is not capped
	 */
	pool_index add_new(T &in){
		gEntry<T> n;
		n.generation = 0;
		//Assuming trivial constructor behavior here...
		n.stored = optional<T>(in);
		storage.push_back(n);
		pool_index i((int) storage.size()-1,0);
		return i;
	};
	/**
	 * @brief
	 * 	//Internal function to use an existing free slot for a new object
	 * @return pool_index
	 */
	pool_index use_free(T &in){
		auto i = known_empty.back();
		Assertion(storage[i].stored == tl::nullopt ,"Attempted to use_free on a slot that was not free");
		//values are created as nullopt when expanding a list
		//and resest to nullopt here so we need to construct a new object to go in there.
		storage[i].stored = optional<T>(in);
		pool_index r((int) i,(int) storage[i].generation);
		known_empty.pop_back();
		return r;
	}

	//Internal function to fill out the empty list when the storage is known to be clear.
	void fill_empty_list(){
		known_empty.clear();
		if((int) storage.size()>0){
			for (int i=((int) storage.size())-1; i>=0;i--){
				known_empty.push_back(i);
			}
		}
	}
	//Internal to reset a slot, assumes generation checking done by interface;
	
	void reset_entry(gEntry<T> &e)
	{
		 
		if(e.stored == tl::nullopt)
			return;
		e.stored.reset();
		e.generation++;
	}
	void reset_slot(int i)
	{
		gEntry<T> &e = storage[i];
		reset_entry(e);
		known_empty.push_back(i);
	}
	public:
	SCP_Pool<T>() = default;
	SCP_Pool<T>(size_t cap){
		reset(cap);
	};

	void reset(){
		capped = false;
		cap = 0;
		for(gEntry<T> &entry: storage)
		{
			reset_entry(entry);
		}
		fill_empty_list();
	}

	void reset(size_t cap_in){
		capped = true;
		cap=MAX(cap_in,storage.size());//shrinking a container is illegal.

		for(gEntry<T> entry: storage){
			if (entry.stored != tl::nullopt)
				entry.stored.reset();
		}
		for (size_t i=storage.size(); i<cap;i++){
			gEntry<T> e;
			e.generation = 0;
			e.stored = tl::nullopt;
			storage.push_back(e);
		}
		fill_empty_list();
	}

	//Index into the storage... bare reference
	T& operator[](pool_index i) {
		if (i.null())
			throw PoolExceptionNullIndex("null index SCP_Pool access attempted");
		if (i.i() >= storage.size())
			throw PoolExceptionOutOfRange("out of range SCP_Pool access attempted");
		if (storage[i.i()].generation!= i.g())
			throw PoolExceptionStaleIndex("Stale SCP_Pool access attempted");
		if(storage[i.i()].stored==tl::nullopt)
			throw PoolExceptionNullValue("Empty SCP_Pool access attempted");
//		v = (storage[i.i()].stored).operator->();
		return storage[i.i()].stored.operator*();
	};

	optional<T&> get(pool_index i){
		SCP_Pool<T> &self = this;
		try{
			T& r = self[i];
			return optional<T&>(r);
		}
		catch(const PoolExceptionNullIndex& e) {
			return tl::nullopt;
		}
		catch(const PoolExceptionOutOfRange& e) {
			return tl::nullopt;
		}
		catch(const PoolExceptionStaleIndex& e) {
			return tl::nullopt;
		}
		catch(const PoolExceptionNullValue& e) {
			return tl::nullopt;
		}
	};
	optional<T*> get_pointer(pool_index i){
		if (i.i() >= storage.size()) {
			return tl::nullopt;
		}
		if (storage[i.i()].generation!= i.g()){
			return tl::nullopt;
		}
		optional<T> s = storage[i.i()].stored;
		T* p = s.operator->();
		return p;
	};

	optional<pool_index> get_new(){
		T* n = new T();
		return store(*n);
	};

	//Store a value in the vector
	//TODO: Handle capped stuff
	//This is kind of in appropriate for the actual usage this is primarily intended to replace
	//to the point where if we want this functionality it might need to be a seperate data structure for sake of strictness
	optional<pool_index> store(T &n){
		if (known_empty.empty()) {
			if (capped && storage.size()>=cap) {
				return tl::nullopt;
			}
			return add_new(n);
		}
		return use_free(n);
	};

	void remove(optional<pool_index> i){
		if (!i.has_value())
			return;
		remove(i.value());
	};
	void remove(pool_index i){
		if (i.null())
			throw PoolExceptionNullIndex("Attempted to remove from SCP_Pool with a null index");
		if (i.i() >= storage.size())
			throw PoolExceptionOutOfRange("Attempted to remove from SCP_Pool with out-of-range index");
		if (storage[i.i()].generation != i.g() ) 
			throw PoolExceptionStaleIndex("Attempted to remove from SCP_Pool with stale index");
		reset_slot(i.i());
	};
	bool check(pool_index i){
		if(i.null())
			return false;
		auto * t = &operator[](i);
		return (t!=nullptr);
	}



	//iterator shit.
	struct iter{
		using iterator_category = std::input_iterator_tag;
		using index = int;
		using storage_t = vector<gEntry<T>>;
		using storage_p = vector<gEntry<T>>*;

		iter(SCP_Pool<T> *vec_ptr, storage_p storage_ptr){
			storage = storage_ptr;
			vec = vec_ptr;
		};
		iter(SCP_Pool<T> *vec_ptr, storage_p storage_ptr, int start_pos){
			storage = storage_ptr;
			vec = vec_ptr;
			inner_position = start_pos;
			if(inner_position < storage->size())
			{
				if(storage->at(inner_position).stored == tl::nullopt){
					(*this)++;
				}
				else {
					full_position =pool_index(inner_position,storage->at(inner_position).generation);
				}
			}
		};
		bool operator==(const iter& rhs){
			return inner_position == rhs.inner_position;
		};
		bool operator<(const iter& rhs){
			return inner_position < rhs.inner_position;
		};
		iter& operator++(){
			inner_position++;
			//we want to skip empty slots
			while(inner_position < storage->size()
					&& storage->at(inner_position).stored == tl::nullopt 
				  ){
				inner_position++;
				//the while condition isn't short-circuiting so we're getting invalid accesses.
				if(inner_position==storage->size()) break;
			} 
			if(inner_position < storage->size()){
			   if(storage->at(inner_position).stored != tl::nullopt){
				full_position.set(inner_position,storage->at(inner_position).generation);
			   }
			   else{
				 full_position.nullify();
			   }
			}
			else{full_position.nullify();}
			return *this;
		};
		iter& operator++(int i){
			operator++();
			return *this;
		}
		T* operator*(){
			//vec->get_pointer(full_position);
			if(vec->check(full_position))
				return &vec->operator[](full_position);
			return nullptr;
		};
		T& operator&(){
//			T t = *vec[full_position];
			return vec->operator[](full_position);
		};
		pool_index position(){return full_position;}
	private:
		index inner_position;
		pool_index full_position;
		storage_p storage;
		SCP_Pool<T> *vec;
	};
	SCP_Pool<T>::iter begin(){
		return iter(this,&storage,0);
		//if (known_empty_) {
		//statements
		//}
	};
	SCP_Pool<T>::iter end(){
		return iter(this,&storage,(int) storage.size());
	}
};

template  <typename T>
class gRef{
   SCP_Pool<T> *vec;
   pool_index ind;
   public:
   gRef(){ vec = nullptr; ind = pool_index();};
   gRef(SCP_Pool<T> *v,pool_index i){
	vec = v;
	ind = i;

   };
   T* get_pointer(){
	 if(vec==nullptr){return nullptr;}
	 auto r = vec->get_pointer(ind);
	 if(r == tl::nullopt)
	 {
		return nullptr;
	 }
	 assert(r.value() != nullptr);
	 T *v = r.value();
	 return v;
   };
   bool check(){
	 return vec->check(ind);
   }
   void destroy(){
	 vec->remove(ind);
   }
};