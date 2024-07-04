#pragma once

// Migrated from boost::sync_bounded_queue which can be found here:
// https://github.com/boostorg/thread/blob/d7251f2/include/boost/thread/concurrent_queues/sync_bounded_queue.hpp
// See original license below:

//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Vicente J. Botet Escriba 2013-2014. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/thread for documentation.
//
//////////////////////////////////////////////////////////////////////////////


#include <mutex>
#include <condition_variable>
#include <exception>


enum queue_op_status
{
	success = 0,
	empty,
	full,
	closed,
	busy
};

class sync_queue_is_closed : public std::runtime_error
{
public:
	sync_queue_is_closed() : std::runtime_error("Sync Bounded Queue is closed") {}
	~sync_queue_is_closed() noexcept override = default;
};

template <typename ValueType>
class sync_bounded_queue
{
public:
	typedef ValueType value_type;
	typedef std::size_t size_type;

	// Constructors/Assignment/Destructors
	explicit sync_bounded_queue(size_type max_elems);
	template <typename Range>
	sync_bounded_queue(size_type max_elems, Range range);
	~sync_bounded_queue();

	// Observers
	inline bool empty() const;
	inline bool full() const;
	inline size_type capacity() const;
	inline size_type size() const;
	inline bool closed() const;

	// Modifiers
	inline void close();

    inline void push_back(const value_type& x);
    inline void push_back(value_type&& x);
    inline queue_op_status try_push_back(const value_type& x);
    inline queue_op_status try_push_back(value_type&& x);
    inline queue_op_status nonblocking_push_back(const value_type& x);
    inline queue_op_status nonblocking_push_back(value_type&& x);
    inline queue_op_status wait_push_back(const value_type& x);
    inline queue_op_status wait_push_back(value_type&& x);

	// Observers/Modifiers
	inline void pull_front(value_type&);
	// enable_if is_nothrow_copy_movable<value_type>
	inline value_type pull_front();
	inline queue_op_status try_pull_front(value_type&);
	inline queue_op_status nonblocking_pull_front(value_type&);

	inline queue_op_status wait_pull_front(ValueType& elem);

private:
	mutable std::mutex mtx_;
	std::condition_variable not_empty_;
	std::condition_variable not_full_;
	size_type waiting_full_;
	size_type waiting_empty_;
	value_type* data_;
	size_type in_;
	size_type out_;
	size_type capacity_;
	bool closed_;

	inline size_type inc(size_type idx) const
	{
		return (idx + 1) % capacity_;
	}

	inline bool empty(std::unique_lock<std::mutex>& ) const
	{
		return in_ == out_;
	}
	inline bool empty(std::lock_guard<std::mutex>& ) const
	{
		return in_ == out_;
	}
	inline bool full(std::unique_lock<std::mutex>& ) const
	{
		return (inc(in_) == out_);
	}
	inline bool full(std::lock_guard<std::mutex>& ) const
	{
		return (inc(in_) == out_);
	}
	inline size_type capacity(std::lock_guard<std::mutex>& ) const
	{
		return capacity_-1;
	}
	inline size_type size(std::lock_guard<std::mutex>& lk) const
	{
		if (full(lk)) return capacity(lk);
		return ((out_+capacity(lk)-in_) % capacity(lk));
	}

	inline void throw_if_closed(std::unique_lock<std::mutex>&);
	inline bool closed(std::unique_lock<std::mutex>&) const;

    inline queue_op_status try_pull_front(value_type& x, std::unique_lock<std::mutex>& lk);
    inline queue_op_status try_push_back(const value_type& x, std::unique_lock<std::mutex>& lk);
    inline queue_op_status try_push_back(value_type&& x, std::unique_lock<std::mutex>& lk);

    inline queue_op_status wait_pull_front(value_type& x, std::unique_lock<std::mutex>& lk);
    inline queue_op_status wait_push_back(const value_type& x, std::unique_lock<std::mutex>& lk);
    inline queue_op_status wait_push_back(value_type&& x, std::unique_lock<std::mutex>& lk);

	inline void wait_until_not_empty(std::unique_lock<std::mutex>& lk);
	inline void wait_until_not_empty(std::unique_lock<std::mutex>& lk, bool&);
	inline size_type wait_until_not_full(std::unique_lock<std::mutex>& lk);
	inline size_type wait_until_not_full(std::unique_lock<std::mutex>& lk, bool&);


	inline void notify_not_empty_if_needed(std::unique_lock<std::mutex>& lk)
	{
		if (waiting_empty_ > 0)
		{
			--waiting_empty_;
			lk.unlock();
			not_empty_.notify_one();
		}
	}
	inline void notify_not_full_if_needed(std::unique_lock<std::mutex>& lk)
	{
		if (waiting_full_ > 0)
		{
			--waiting_full_;
			lk.unlock();
			not_full_.notify_one();
		}
	}

	inline void pull_front(value_type& elem, std::unique_lock<std::mutex>& lk)
	{
		elem = std::move(data_[out_]);
		out_ = inc(out_);
		notify_not_full_if_needed(lk);
	}
	inline value_type pull_front(std::unique_lock<std::mutex>& lk)
	{
		value_type elem = std::move(data_[out_]);
		out_ = inc(out_);
		notify_not_full_if_needed(lk);
		return std::move(elem);
	}

	inline void set_in(size_type in, std::unique_lock<std::mutex>& lk)
	{
		in_ = in;
		notify_not_empty_if_needed(lk);
	}

	inline void push_at(const value_type& elem, size_type in_p_1, std::unique_lock<std::mutex>& lk)
	{
		data_[in_] = elem;
		set_in(in_p_1, lk);
	}

	inline void push_at(value_type&& elem, size_type in_p_1, std::unique_lock<std::mutex>& lk)
	{
		data_[in_] = std::move(elem);
		set_in(in_p_1, lk);
	}
};

template <typename ValueType>
sync_bounded_queue<ValueType>::sync_bounded_queue(typename sync_bounded_queue<ValueType>::size_type max_elems) :
	waiting_full_(0), waiting_empty_(0), data_(new value_type[max_elems + 1]), in_(0), out_(0), capacity_(max_elems + 1),
			closed_(false)
{
}

template <typename ValueType>
sync_bounded_queue<ValueType>::~sync_bounded_queue()
{
	delete[] data_;
}

template <typename ValueType>
void sync_bounded_queue<ValueType>::close()
{
	{
		std::lock_guard<std::mutex> lk(mtx_);
		closed_ = true;
	}
	not_empty_.notify_all();
	not_full_.notify_all();
}

template <typename ValueType>
bool sync_bounded_queue<ValueType>::closed() const
{
	std::lock_guard<std::mutex> lk(mtx_);
	return closed_;
}
template <typename ValueType>
bool sync_bounded_queue<ValueType>::closed(std::unique_lock<std::mutex>& ) const
{
	return closed_;
}

template <typename ValueType>
bool sync_bounded_queue<ValueType>::empty() const
{
	std::lock_guard<std::mutex> lk(mtx_);
	return empty(lk);
}
template <typename ValueType>
bool sync_bounded_queue<ValueType>::full() const
{
	std::lock_guard<std::mutex> lk(mtx_);
	return full(lk);
}

template <typename ValueType>
typename sync_bounded_queue<ValueType>::size_type sync_bounded_queue<ValueType>::capacity() const
{
	std::lock_guard<std::mutex> lk(mtx_);
	return capacity(lk);
}

template <typename ValueType>
typename sync_bounded_queue<ValueType>::size_type sync_bounded_queue<ValueType>::size() const
{
	std::lock_guard<std::mutex> lk(mtx_);
	return size(lk);
}


template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::try_pull_front(ValueType& elem, std::unique_lock<std::mutex>& lk)
{
	if (empty(lk))
	{
		if (closed(lk)) return queue_op_status::closed;
		return queue_op_status::empty;
	}
	pull_front(elem, lk);
	return queue_op_status::success;
}

template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::try_pull_front(ValueType& elem)
{
		std::unique_lock<std::mutex> lk(mtx_);
		return try_pull_front(elem, lk);
}


template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::nonblocking_pull_front(ValueType& elem)
{
		std::unique_lock<std::mutex> lk(mtx_, std::try_to_lock);
		if (!lk.owns_lock())
		{
			return queue_op_status::busy;
		}
		return try_pull_front(elem, lk);
}

template <typename ValueType>
void sync_bounded_queue<ValueType>::throw_if_closed(std::unique_lock<std::mutex>&)
{
	if (closed_)
	{
		throw sync_queue_is_closed();
	}
}

template <typename ValueType>
void sync_bounded_queue<ValueType>::wait_until_not_empty(std::unique_lock<std::mutex>& lk)
{
	for (;;)
	{
		if (out_ != in_) break;
		throw_if_closed(lk);
		++waiting_empty_;
		not_empty_.wait(lk);
	}
}
template <typename ValueType>
void sync_bounded_queue<ValueType>::wait_until_not_empty(std::unique_lock<std::mutex>& lk, bool & _closed)
{
	for (;;)
	{
		if (out_ != in_) break;
		if (closed_) {_closed=true; return;}
		++waiting_empty_;
		not_empty_.wait(lk);
	}
}


template <typename ValueType>
void sync_bounded_queue<ValueType>::pull_front(ValueType& elem)
{
		std::unique_lock<std::mutex> lk(mtx_);
		wait_until_not_empty(lk);
		pull_front(elem, lk);
}

// enable if ValueType is nothrow movable
template <typename ValueType>
ValueType sync_bounded_queue<ValueType>::pull_front()
{
		std::unique_lock<std::mutex> lk(mtx_);
		wait_until_not_empty(lk);
		return pull_front(lk);
}

template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::wait_pull_front(ValueType& elem, std::unique_lock<std::mutex>& lk)
{
		if (empty(lk) && closed(lk)) {return queue_op_status::closed;}
		wait_until_not_empty(lk);
		pull_front(elem, lk);
		return queue_op_status::success;
}
template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::wait_pull_front(ValueType& elem)
{
	std::unique_lock<std::mutex> lk(mtx_);
	return wait_pull_front(elem, lk);
}


template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::try_push_back(const ValueType& elem, std::unique_lock<std::mutex>& lk)
{
	if (closed(lk)) return queue_op_status::closed;
	size_type in_p_1 = inc(in_);
	if (in_p_1 == out_)  // full()
	{
		return queue_op_status::full;
	}
	push_at(elem, in_p_1, lk);
	return queue_op_status::success;
}

template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::try_push_back(const ValueType& elem)
{
	std::unique_lock<std::mutex> lk(mtx_);
	return try_push_back(elem, lk);
}

template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::wait_push_back(const ValueType& elem, std::unique_lock<std::mutex>& lk)
{
	if (closed(lk)) return queue_op_status::closed;
	push_at(elem, wait_until_not_full(lk), lk);
	return queue_op_status::success;
}
template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::wait_push_back(const ValueType& elem)
{
	std::unique_lock<std::mutex> lk(mtx_);
	return wait_push_back(elem, lk);
}


template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::nonblocking_push_back(const ValueType& elem)
{
	std::unique_lock<std::mutex> lk(mtx_, std::try_to_lock);
	if (!lk.owns_lock()) return queue_op_status::busy;
	return try_push_back(elem, lk);
}

template <typename ValueType>
typename sync_bounded_queue<ValueType>::size_type sync_bounded_queue<ValueType>::wait_until_not_full(std::unique_lock<std::mutex>& lk)
{
	for (;;)
	{
		throw_if_closed(lk);
		size_type in_p_1 = inc(in_);
		if (in_p_1 != out_) // ! full()
		{
			return in_p_1;
		}
		++waiting_full_;
		not_full_.wait(lk);
	}
}

template <typename ValueType>
void sync_bounded_queue<ValueType>::push_back(const ValueType& elem)
{
		std::unique_lock<std::mutex> lk(mtx_);
		push_at(elem, wait_until_not_full(lk), lk);
}

template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::try_push_back(ValueType&& elem, std::unique_lock<std::mutex>& lk)
{
	if (closed(lk)) return queue_op_status::closed;
	size_type in_p_1 = inc(in_);
	if (in_p_1 == out_) // full()
	{
		return queue_op_status::full;
	}
	push_at(std::move(elem), in_p_1, lk);
	return queue_op_status::success;
}
template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::try_push_back(ValueType&& elem)
{
		std::unique_lock<std::mutex> lk(mtx_);
		return try_push_back(std::move(elem), lk);
}

template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::wait_push_back(ValueType&& elem, std::unique_lock<std::mutex>& lk)
{
	if (closed(lk)) return queue_op_status::closed;
	push_at(std::move(elem), wait_until_not_full(lk), lk);
	return queue_op_status::success;
}
template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::wait_push_back(ValueType&& elem)
{
		std::unique_lock<std::mutex> lk(mtx_);
		return try_push_back(std::move(elem), lk);
}

template <typename ValueType>
queue_op_status sync_bounded_queue<ValueType>::nonblocking_push_back(ValueType&& elem)
{
		std::unique_lock<std::mutex> lk(mtx_, std::try_to_lock);
		if (!lk.owns_lock())
		{
			return queue_op_status::busy;
		}
		return try_push_back(std::move(elem), lk);
}

template <typename ValueType>
void sync_bounded_queue<ValueType>::push_back(ValueType&& elem)
{
		std::unique_lock<std::mutex> lk(mtx_);
		push_at(std::move(elem), wait_until_not_full(lk), lk);
}

template <typename ValueType>
sync_bounded_queue<ValueType>& operator<<(sync_bounded_queue<ValueType>& sbq, ValueType&& elem)
{
	sbq.push_back(std::forward<ValueType>(elem));
	return sbq;
}

template <typename ValueType>
sync_bounded_queue<ValueType>& operator<<(sync_bounded_queue<ValueType>& sbq, ValueType const&elem)
{
	sbq.push_back(elem);
	return sbq;
}

template <typename ValueType>
sync_bounded_queue<ValueType>& operator>>(sync_bounded_queue<ValueType>& sbq, ValueType &elem)
{
	sbq.pull_front(elem);
	return sbq;
}
