#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
#include <tl/optional.hpp>
#include <vcruntime.h>
#include <iterator>

struct pool_index{
	
	private:
	int index;
	int generation;
	int vector_uid;
	bool is_null = true;
	public:
	pool_index(){ nullify();};
	pool_index(int i,int g,int id, bool b = false){
		if(b){ nullify();
		}
		else{
			set(i,g,id);
		}
	};
	inline void set(int i,int g, int id){
		is_null = false;
		index = i;
		generation = g;
		vector_uid = id;
	}
	inline void nullify()
	{
		index = -1;
		generation = -1;
		is_null=true;
	};
	inline bool operator==(pool_index &rhs) const {
		if(is_null && rhs.is_null)
			return true;
		if(is_null || rhs.is_null)
			return false;
		return (index == rhs.index 
				&& generation == rhs.generation
				&& vector_uid == rhs.vector_uid);
	};
	inline pool_index& operator=(pool_index const&rhs) {
		if(rhs.is_null)
			nullify();
		else
			set(rhs.index, rhs.generation, rhs.vector_uid);
		return *this;
	};
	inline int i() const { return index;};
	inline int g() const { return generation;};
	inline int u() const { return vector_uid;};
	inline bool null() const { return is_null;};
	inline bool has_value() const { return !is_null;};
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
class PoolExceptionWrongPool : public std::runtime_error
{
public:
	explicit PoolExceptionWrongPool(const std::string& msg) : std::runtime_error(msg) {}
	~PoolExceptionWrongPool() noexcept override = default;
};


namespace scp_pool{
	int new_uid(); 
}


template  <typename T>
//rules:
//Caps can only ever be set at the current size or larger
//shrinking the vector would lead to forgetting the generation counts and then potential index reuse if allowed to rexpand later.
class SCP_Pool {
	struct gEntry {
		int generation;
		tl::optional<T> stored;

	};
	/**************************************************************************
	 * Private data members
	 *
	 *************************************************************************/
	SCP_vector<gEntry> storage;
	SCP_vector<size_t> known_empty;
	static_assert(!std::is_reference<T>::value, "SCP_Pool<T> can not be used to store a reference type.");

	bool capped = false;
	int cap = -1;
	int uid = -1;

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
		gEntry n;
		n.generation = 0;
		//Assuming trivial constructor behavior here...
		n.stored = tl::optional<T>(in);
		storage.push_back(n);
		pool_index i((int) storage.size()-1,0,uid);
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
		storage[i].stored = tl::optional<T>(in);
		pool_index r((int) i,(int) storage[i].generation,uid);
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
	
	void reset_entry(gEntry &e)
	{
		 
		if(e.stored == tl::nullopt)
			return;
		e.stored.reset();
		e.generation++;
	}
	void reset_slot(int i)
	{
		gEntry &e = storage[i];
		reset_entry(e);
		known_empty.push_back(i);
	}


	//Store a value in the vector
	//TODO: Handle capped stuff
	//As public this kind of inappropriate for the actual usage this is primarily intended to replace
	//to the point where if we want this functionality it might need to be a seperate data structure for sake of strictness.
	//Moved it to private unless and until a good reason for external access comes up.
	tl::optional<pool_index> store(T &n){
		if (known_empty.empty()) {
			if (capped && storage.size()>=cap) {
				return tl::nullopt;
			}
			return add_new(n);
		}
		return use_free(n);
	};
	SCP_string debug_name;
	public:
	SCP_Pool<T>(int cap = -1, SCP_string name = "") : cap(cap), debug_name(name){
		uid = scp_pool::new_uid(); 
		if(cap>0)
			reset(cap);
		else reset();
	};

	void reset(){
		capped = false;
		cap = 0;
		for(gEntry &entry: storage)
		{
			reset_entry(entry);
		}
		fill_empty_list();
	}

	void reset(int cap_in){
		if( cap_in<0) return reset();
		capped = true;
		cap= cap_in;
		MAX(cap,(int) storage.size());//shrinking a container is illegal.

		for(gEntry entry: storage){
			if (entry.stored != tl::nullopt)
				entry.stored.reset();
		}
		for (size_t i=storage.size(); i<cap;i++){
			gEntry e;
			e.generation = 0;
			e.stored = tl::nullopt;
			storage.push_back(e);
		}
		fill_empty_list();
	}

	//Index into the storage... bare reference
	T& operator[](pool_index const& i) {
		if (i.null())
			throw PoolExceptionNullIndex("null index SCP_Pool access attempted");
		if (i.u() != uid)
			throw PoolExceptionWrongPool("SCP_Pool access attempted on wrong pool");
		if (i.i() >= storage.size())
			throw PoolExceptionOutOfRange("out of range SCP_Pool access attempted");
		if (storage[i.i()].generation!= i.g())
			throw PoolExceptionStaleIndex("Stale SCP_Pool access attempted");
		Assertion(storage[i.i()].stored!=tl::nullopt, "Empty SCP Pool access");
		return *(storage[i.i()].stored);
	};
	bool value_at(pool_index i){
		return !(i.null()
			|| i.u()!=uid
			|| i.i() >= storage.size()
			|| storage[i.i()].generation!= i.g()
			|| storage[i.i()].stored==tl::nullopt);
	}
	tl::optional<T&> get(pool_index i){
		SCP_Pool<T> &self = this;

		if(! value_at(i))
			return tl::nullopt;
		T& r = self[i];
		return tl::optional<T&>(r);

	};
	tl::optional<T*> get_pointer(pool_index i){
		if(! value_at(i))
			return tl::nullopt;
		tl::optional<T> s = storage[i.i()].stored;
		T* p = s.operator->();
		return p;
	};

	tl::optional<pool_index> get_new(){
		T n = T();
		return store(n);
	};

	template<typename... args_t>
	tl::optional<pool_index> get_new(args_t&&... args){
		T n = T(std::forward<args_t>(args)...);
		return store(n);
	};


	void remove(tl::optional<pool_index> i){
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
	struct iterator{
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using index = int;
		using storage_t = SCP_vector<gEntry>;
		using t = SCP_Pool<T>;


		iterator( t* parent,index i) : vec(parent), inner_position(i)
		{
			storage_t &st = vec->storage;
			if(inner_position < st.size())
			{
				if(st.at(inner_position).stored == tl::nullopt){
					(*this)++;
				}
				else {
					full_position =pool_index(inner_position,
						st.at(inner_position).generation,
						vec->uid);
				}
			}
		};
		bool operator==(const iterator& rhs){
			return inner_position == rhs.inner_position;
		};
		bool operator!=(const iterator& rhs){
			return !(inner_position == rhs.inner_position);
		};
		bool operator<(const iterator& rhs){
			return inner_position < rhs.inner_position;
		};
		iterator& operator++(){
			inner_position++;
			MAX(inner_position,0);
			storage_t &st = vec->storage;
			//we want to skip empty slots
			while(inner_position < st.size()
					&& st.at(inner_position).stored == tl::nullopt 
				  ){
				inner_position++;
				//the while condition isn't short-circuiting so we're getting invalid accesses.
				if(inner_position==st.size()) break;
			} 
			if(inner_position < st.size()){
			   if(st.at(inner_position).stored != tl::nullopt){
				full_position.set(inner_position,st.at(inner_position).generation, vec->uid);
			   }
			   else{
				 full_position.nullify();
			   }
			}
			else{full_position.nullify();}
			return *this;
		};
		iterator operator++(int i){
			iterator copy = *this;
			operator++();
			return copy;
		}
		reference operator*() {
			Assertion(vec->check(full_position), "scp_pool iterator attempted to get a pointer to an empty slot somehow");
			return vec->operator[](full_position);
		};

		reference operator->() {
			Assertion(vec->check(full_position), "scp_pool iterator attempted to get a pointer to an empty slot somehow");
			return vec->operator[](full_position);
		};
//		pointer operator&(){
			//	T t = *vec[full_position];
//			return &operator->();
//		};
		pool_index position(){return full_position;}
	private:
		index inner_position;
		pool_index full_position;
		t *vec;
	};
	iterator cbegin() const{
		return iterator(this,0);
	};
	iterator cend() const
	{
		return iterator(this,(int) storage.size());
	};
	SCP_Pool<T>::iterator begin() {
		return SCP_Pool<T>::iterator(this,0);
	};
	SCP_Pool<T>::iterator end()
	{
		return SCP_Pool<T>::iterator(this,(int) storage.size());
	};
	/*
	SCP_Pool<T>::iterator begin(){
		return iterator(this,0);
		//if (known_empty_) {
		//statements
		//}
	};
	SCP_Pool<T>::iterator end(){
		return iterator(this,(int) storage.size());
	}*/
};