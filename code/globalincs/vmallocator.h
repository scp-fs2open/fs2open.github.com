#ifndef _VMALLOCATOR_H_INCLUDED_
#define _VMALLOCATOR_H_INCLUDED_

#include <algorithm>
#include <deque>
#include <iterator>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <typename T>
class SCP_vector : public std::vector<T, std::allocator<T>>
{
public:
	using std::vector<T, std::allocator<T>>::vector;	// inherit all constructors

	// this is needed as a workaround to a GCC compiler bug when using a templated accessor...
	// error: dereferencing type-punned pointer will break strict-aliasing rules [-Werror=strict-aliasing]
	inline auto size() const noexcept { return std::vector<T, std::allocator<T>>::size(); }

	bool contains(const T& item) const
	{
		return std::find(this->begin(), this->end(), item) != this->end();
	}

	bool in_bounds(int idx) const
	{
		return (idx >= 0) && (static_cast<size_t>(idx) < this->size());
	}

	bool in_bounds(size_t idx) const
	{
		return idx < this->size();
	}
};

template <typename T>
bool SCP_vector_contains(const SCP_vector<T>& vector, const T& item) {
	return std::find(vector.begin(), vector.end(), item) != vector.end();
}

template <typename T>
inline bool SCP_vector_inbounds(const SCP_vector<T>& vector, int idx) {
	return ((idx >= 0) && (static_cast<size_t>(idx) < vector.size()));
}

template <typename T>
inline bool SCP_vector_inbounds(const SCP_vector<T>& vector, size_t idx) {
	return idx < vector.size();
}

template <typename T>
class SCP_list : public std::list<T, std::allocator<T>>
{
public:
	using std::list<T, std::allocator<T>>::list;	// inherit all constructors

	bool contains(const T& item) const
	{
		return std::find(this->begin(), this->end(), item) != this->end();
	}
};


extern std::locale SCP_default_locale;

template <class charT>
[[nodiscard]] charT SCP_toupper(charT ch) { return std::toupper(ch, SCP_default_locale); }

template <class charT>
[[nodiscard]] charT SCP_tolower(charT ch) { return std::tolower(ch, SCP_default_locale); }

typedef std::basic_string<char, std::char_traits<char>, std::allocator<char>> SCP_string;
typedef std::basic_stringstream<char, std::char_traits<char>, std::allocator<char>> SCP_stringstream;

extern void SCP_tolower(char *str);
extern void SCP_toupper(char *str);
extern void SCP_totitle(char *str);

extern void SCP_tolower(SCP_string &str);
extern void SCP_toupper(SCP_string &str);
extern void SCP_totitle(SCP_string &str);

extern bool SCP_truncate(SCP_string &str, size_t len);

extern bool lcase_equal(const SCP_string& _Left, const SCP_string& _Right);
extern bool lcase_lessthan(const SCP_string& _Left, const SCP_string& _Right);


template <typename T, typename U>
using SCP_map = std::map<T, U, std::less<T>, std::allocator<std::pair<const T, U>>>;

template <typename T, typename U>
using SCP_multimap = std::multimap<T, U, std::less<T>, std::allocator<std::pair<const T, U>>>;

template <typename T>
using SCP_queue = std::queue<T, std::deque<T, std::allocator<T>>>;

template <typename T>
using SCP_deque = std::deque<T, std::allocator<T>>;

template <typename T>
class SCP_set : public std::set<T, std::less<T>, std::allocator<T>>
{
public:
	using std::set<T, std::less<T>, std::allocator<T>>::set;	// inherit all constructors

	bool contains(const T& item) const
	{
		return this->find(item) != this->end();
	}
};

template <typename T>
class SCP_multiset : public std::multiset<T, std::less<T>, std::allocator<T>>
{
public:
	using std::multiset<T, std::less<T>, std::allocator<T>>::multiset;	// inherit all constructors

	bool contains(const T& item) const
	{
		return this->find(item) != this->end();
	}
};

// Now that the codebase is on C++17, we can use the standard hash.  (Enum classes are not hashable on C++ < 14.)
template <typename T>
using SCP_hash = std::hash<T>;

struct SCP_string_lcase_hash {
	size_t operator()(const SCP_string& elem) const {
		SCP_string lcase_copy;
		std::transform(elem.begin(), elem.end(), std::back_inserter(lcase_copy),
			[](char c) {
				return SCP_tolower(c);
			});
		return SCP_hash<SCP_string>()(lcase_copy);
	}
};

struct SCP_string_lcase_equal_to {
	bool operator()(const SCP_string& _Left, const SCP_string& _Right) const {
		return lcase_equal(_Left, _Right);
	}
};

struct SCP_string_lcase_less_than {
	bool operator()(const SCP_string& _Left, const SCP_string& _Right) const {
		return lcase_lessthan(_Left, _Right);
	}
};

template <typename Key, typename T, typename Hash = SCP_hash<Key>, typename KeyEqual = std::equal_to<Key>>
using SCP_unordered_map = std::unordered_map<Key, T, Hash, KeyEqual, std::allocator<std::pair<const Key, T>>>;

template <typename Key, typename Hash = SCP_hash<Key>, typename KeyEqual = std::equal_to<Key>>
class SCP_unordered_set : public std::unordered_set<Key, Hash, KeyEqual, std::allocator<Key>>
{
public:
	using std::unordered_set<Key, Hash, KeyEqual, std::allocator<Key>>::unordered_set;	// inherit all constructors

	bool contains(const Key& item) const
	{
		return this->find(item) != this->end();
	}
};

template <typename T, typename... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
template <typename T, typename... Args>
typename std::enable_if<std::is_array<T>::value, std::unique_ptr<T>>::type make_unique(std::size_t n) {
	return std::unique_ptr<T>(new typename std::remove_extent<T>::type[n]());
}

template <typename T, typename... Args>
typename std::enable_if<!std::is_array<T>::value, std::shared_ptr<T>>::type make_shared(Args&&... args) {
	return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
}
template <typename T, typename... Args>
typename std::enable_if<std::is_array<T>::value, std::shared_ptr<T>>::type make_shared(std::size_t n) {
	return std::shared_ptr<T>(new typename std::remove_extent<T>::type[n]());
}

#endif // _VMALLOCATOR_H_INCLUDED_
