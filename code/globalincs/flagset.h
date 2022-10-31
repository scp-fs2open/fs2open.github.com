#ifndef _FLAGSET_
#define _FLAGSET_

#include <bitset>
#include <cstdint>

#include "osapi/dialogs.h"

constexpr size_t UBYTE_SIZE = 8;

template<typename TEnum, size_t SIZE>
struct flag_combinator;

template<class T, size_t SIZE = static_cast < size_t >(T::NUM_VALUES)>
class flagset {
 protected:
	std::bitset<SIZE> values;
 public:
	flagset() {}

	flagset(const std::initializer_list<T>& init_list) {
		for (auto& val : init_list) {
			set(val);
		}
	}

	bool operator[](const T idx) const { return values[(static_cast < size_t >(idx))]; };

	template<size_t COMB_SIZE>
	bool operator[](const flag_combinator<T, COMB_SIZE>& combinator) const {
		return combinator.check_flagset(*this);
	};

	flagset<T> operator&(const flagset<T>& other) const {
		flagset<T> result;
		result.values = this->values & other.values;
		return result;
	}

	flagset<T> operator+(const T flag) const {
		flagset<T> result = *this;
		result.set(flag);
		return result;
	}

	flagset<T>& operator+=(const T flag) {
		this->set(flag);
		return *this;
	}

	flagset<T> operator-(const T flag) const {
		flagset<T> result = *this;
		result.remove(flag);
		return result;
	}

	flagset<T>& operator-=(const T flag) {
		this->remove(flag);
		return *this;
	}

	flagset<T> operator|(const flagset<T>& other) const {
		flagset<T> result;
		result.values = this->values | other.values;
		return result;
	}

	flagset<T>& operator|=(const flagset<T>& other) {
		this->values |= other.values;

		return *this;
	}

	void operator|=(const T flag) {
		set(flag);
	}

	bool operator==(const flagset<T>& other) const { return this->values == other.values; }
	bool operator!=(const flagset<T>& other) const { return this->values != other.values; }

	void reset() { values.reset(); }

	flagset<T>& set(T idx, bool value = true) {
		Assertion(static_cast<size_t>(idx) < values.size(),
				  "Invalid value passed to flagset::set(), get a stacktrace, a coder, an old priest and a young priest.");

		values.set(static_cast < size_t >(idx), value);
		return *this;
	}

	template<typename TIter>
	flagset<T>& set_multiple(TIter begin, TIter end) {
		auto current = begin;
		while (current != end) {
			set(*current, true);

			current = std::next(current);
		}

		return *this;
	}

	flagset<T>& remove(T idx) {
		return set(idx, false);
	}

	template<typename TIter>
	flagset<T>& remove_multiple(TIter begin, TIter end) {
		auto current = begin;
		while (current != end) {
			set(*current, false);

			current = std::next(current);
		}

		return *this;
	}

	size_t to_ubyte_vector(SCP_vector<ubyte>& vector_out)
	{
		int counter = 0;		// a counter to determine how far to bitshift
		ubyte  ubyte_out= 0;	// the holder for our bitshifted flags

		for (size_t i = 0; i < SIZE; i++) {

			// this needs to be at the beginning for push_back after the loop
			// to work properly.
			if (counter == UBYTE_SIZE) {
				vector_out.push_back(ubyte_out);
				counter = 0;
				ubyte_out = 0;
			}
			
			// check whether that flag was set
			// there is probably a more efficient way to do this,
			// but not unless we can copy the underlying data safely.
			if (values[i]) {
				ubyte_out |= 1 << counter; 
			}	

			// iterate
			++counter;
		}

		vector_out.push_back(ubyte_out);
		
		return vector_out.size();
	}

	void set_from_vector(const SCP_vector<ubyte>& vector_in) 
	{
		size_t flag = 0; 

		// just iterate through the entries, and set flags as appropriate.
		for (auto& entry : vector_in) {

			for (uint bitshift = 0; bitshift < UBYTE_SIZE; bitshift++){
				if (entry & (1 << bitshift)) {
					values.set(flag, true);
				}

				++flag;

				// this function is used in multi where the size of flagsets can be different
				// on server and client.  Small differences in flagset sizes should not affect
				// gameplay, but we have to make sure not to exceed the size of the flagset.
				if (flag >= SIZE){
					return;
				} 
			}
		}
	}

	flagset<T>& toggle(T idx) {
		values[static_cast < size_t >(idx)] = !values[static_cast < size_t >(idx)];

		return *this;
	}

	bool any_set() { return values.any(); }
	bool none_set() { return values.none(); }

	void from_u64(std::uint64_t num) { values = (unsigned long) num; }

	std::uint64_t to_u64() const { return (std::uint64_t) values.to_ulong(); }

	size_t hash() const { return std::hash<std::bitset<SIZE>>()(values); }
};

namespace std {
template <typename T, size_t N>
struct hash<::flagset<T, N>> {
	size_t operator()(const ::flagset<T, N>& val) const { return val.hash(); }
};
} // namespace std

#define FLAG_LIST(Type) enum class Type : size_t

template<typename TEnum, size_t SIZE>
struct flag_combinator {
	static_assert(SIZE > 2, "SIZE must be greater than 2!");

	// This check makes sure that TEnum is actually a flag type
	static_assert(SIZE <= static_cast<size_t>(TEnum::NUM_VALUES), "Size must be less than NUM_VALUES");

	flag_combinator(const flag_combinator<TEnum, SIZE - 1>& left, TEnum right) : _otherValues(left) {
		_value = right;
	}

	bool check_flagset(const flagset<TEnum>& flagset) const {
		return flagset[_value] || _otherValues.check_flagset(flagset);
	}
 protected:
	flag_combinator<TEnum, SIZE - 1> _otherValues;
	TEnum _value;
};

template<typename TEnum>
struct flag_combinator<TEnum, 2> {
	TEnum left;
	TEnum right;

	flag_combinator(TEnum in_left, TEnum in_right) {
		left = in_left;
		right = in_right;
	}

	bool check_flagset(const flagset <TEnum>& flagset) const {
		return flagset[left] || flagset[right];
	}
};

template<typename TEnum>
struct flag_enum_checker
{
	static const bool value = std::is_enum<TEnum>::value;
};

template<typename TEnum>
typename std::enable_if<flag_enum_checker<TEnum>::value, flag_combinator<TEnum, 2>>::type
operator,(TEnum left, TEnum right) {
	return flag_combinator<TEnum, 2>(left, right);
};

template<typename TEnum, size_t SIZE>
typename std::enable_if<flag_enum_checker<TEnum>::value, flag_combinator<TEnum, SIZE + 1>>::type
operator,(const flag_combinator<TEnum, SIZE>& left, TEnum right) {
	return flag_combinator<TEnum, SIZE + 1>(left, right);
};

#endif
