
#ifndef _FSO_UTILITY_H
#define _FSO_UTILITY_H

#include <vector>

#include "globalincs/globals.h"
#include "globalincs/toolchain.h"


// Goober5000
// A sort for use with small or almost-sorted lists.  Iteration time is O(n) for a fully-sorted list.
// This uses a type-safe version of the function prototype for stdlib's qsort, although the size is an int rather than a size_t (for the reasons that j is an int).
// The fncompare function should return <0, 0, or >0 as the left item is less than, equal to, or greater than the right item.
template <typename array_t, typename T>
void insertion_sort(array_t& array_base, int array_size, int (*fncompare)(const T*, const T*))
{
	// NOTE: j *must* be a signed type because j reaches -1 and j+1 must be 0.
	int i, j;
	T *current, *current_buf;

	// allocate space for the element being moved
	// (Taylor says that for optimization purposes malloc/free should be used rather than vm_malloc/vm_free here)
	current_buf = new T();
	if (current_buf == nullptr)
	{
		UNREACHABLE("Malloc failed!");
		return;
	}

	// loop
	for (i = 1; i < array_size; i++)
	{
		// grab the current element
		// this does a lazy move/copy because if the array is mostly sorted,
		// there's no sense copying sorted items to their own places
		bool lazily_copied = false;
		current = &array_base[i];

		// bump other elements toward the end of the array
		for (j = i - 1; (j >= 0) && (fncompare(&array_base[j], current) > 0); j--)
		{
			if (!lazily_copied)
			{
				// this may look strange but it is just copying the data
				// into the buffer, then pointing to the buffer
				*current_buf = std::move(*current);
				current = current_buf;
				lazily_copied = true;
			}

			array_base[j + 1] = std::move(array_base[j]);
		}

		if (lazily_copied)
		{
			// insert the current element at the correct place
			array_base[j + 1] = std::move(*current);
		}
	}

	// free the allocated space
	delete current_buf;
}

//
// See https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C++
//
template<typename T>
typename T::size_type GeneralizedLevenshteinDistance(const T &source,
	const T &target,
	typename T::size_type insert_cost = 1,
	typename T::size_type delete_cost = 1,
	typename T::size_type replace_cost = 1) {
	if (source.size() > target.size()) {
		return GeneralizedLevenshteinDistance(target, source, delete_cost, insert_cost, replace_cost);
	}

	using TSizeType = typename T::size_type;
	const TSizeType min_size = source.size(), max_size = target.size();
	std::vector<TSizeType> lev_dist(min_size + 1);

	lev_dist[0] = 0;
	for (TSizeType i = 1; i <= min_size; ++i) {
		lev_dist[i] = lev_dist[i - 1] + delete_cost;
	}

	for (TSizeType j = 1; j <= max_size; ++j) {
		TSizeType previous_diagonal = lev_dist[0], previous_diagonal_save;
		lev_dist[0] += insert_cost;

		for (TSizeType i = 1; i <= min_size; ++i) {
			previous_diagonal_save = lev_dist[i];
			if (source[i - 1] == target[j - 1]) {
				lev_dist[i] = previous_diagonal;
			}
			else {
				lev_dist[i] = std::min(std::min(lev_dist[i - 1] + delete_cost, lev_dist[i] + insert_cost), previous_diagonal + replace_cost);
			}
			previous_diagonal = previous_diagonal_save;
		}
	}

	return lev_dist[min_size];
}

// Lafiel
template<typename T>
typename T::size_type stringcost(const T& op, const T& input, typename T::size_type max_expected_length = NAME_LENGTH) {
	using TSizeType = typename T::size_type;

    if(input.empty())
        return T::npos;

    struct string_search_it {
		TSizeType count;
		TSizeType lastpos;
		TSizeType cost;
    };
    std::vector<string_search_it> iterators;

    //Go through the input. If we find it split up into parts, prefer things that are least split, and within this prefer things that are closer together
    for (TSizeType i = 0; i < op.length(); i++) {
		std::vector<string_search_it> insert;
        for (auto& it : iterators) {
            if (it.count < input.length() && op[i] == input[it.count]) {
                //We found something. There may be a better match for this later, so only make a copy.
                insert.emplace_back(string_search_it{it.count + 1, i, i - it.lastpos <= 1 ? it.cost : max_expected_length + i - it.lastpos - 1});
            }
        }

        iterators.insert(iterators.end(), insert.begin(), insert.end());

        if (op[i] == input[0])
            iterators.emplace_back(string_search_it{1, i, i});
    }

    auto cost = T::npos;

    for (const auto& it : iterators) {
        //Things that are missing letters are considered worse by default
        auto localcost = (input.length() - it.count) * (max_expected_length * max_expected_length) + it.cost;
        if (localcost < cost)
            cost = localcost;
    }

    return cost;
}

template <typename T>
int count_items_with_name(const char* name, const T* item_array, int num_items)
{
	if (!name || !item_array)
		return 0;

	int count = 0;
	for (int i = 0; i < num_items; ++i)
		if (!stricmp(name, item_array[i].name))
			++count;

	return count;
}

template <typename T>
int count_items_with_name(const char* name, const T& item_vector)
{
	if (!name)
		return 0;

	int count = 0;
	for (const auto& item : item_vector)
		if (!stricmp(name, item.name))
			++count;

	return count;
}

template <typename T>
int count_items_with_scp_string_name(const char* name, const T& item_vector)
{
	if (!name)
		return 0;

	int count = 0;
	for (const auto& item : item_vector)
		if (!stricmp(name, item.name.c_str()))
			++count;

	return count;
}

template <typename VECTOR_T, typename ITEM_T, typename FIELD_T>
int find_item_with_field(const VECTOR_T& item_vector, FIELD_T ITEM_T::* field, const char* str)
{
	if (!str)
		return -1;

	int index = 0;
	for (const ITEM_T& item : item_vector)
	{
		if (!stricmp(item.*field, str))
			return index;
		else
			++index;
	}

	return -1;
}

template <typename VECTOR_T, typename ITEM_T, typename FIELD_T>
int find_item_with_field(const VECTOR_T& item_vector, FIELD_T ITEM_T::* field, const SCP_string& str)
{
	int index = 0;
	for (const ITEM_T& item : item_vector)
	{
		if (lcase_equal(item.*field, str))
			return index;
		else
			++index;
	}

	return -1;
}

template <typename VECTOR_T, typename ITEM_T, typename FIELD_T>
int find_item_with_field(const VECTOR_T& item_vector, FIELD_T ITEM_T::* field, const FIELD_T& search)
{
	int index = 0;
	for (const ITEM_T& item : item_vector)
	{
		if (item.*field == search)
			return index;
		else
			++index;
	}

	return -1;
}

template <typename ITEM_T, typename FIELD_T>
int find_item_with_field(const ITEM_T* item_array, int num_items, FIELD_T ITEM_T::* field, const char* str)
{
	if (!str)
		return -1;

	for (int i = 0; i < num_items; ++i)
		if (!stricmp(item_array[i].*field, str))
			return i;

	return -1;
}

template <typename ITEM_T, typename FIELD_T>
int find_item_with_field(const ITEM_T* item_array, int num_items, FIELD_T ITEM_T::* field, const SCP_string& str)
{
	for (int i = 0; i < num_items; ++i)
		if (lcase_equal(item_array[i].*field, str))
			return i;

	return -1;
}

template <typename ITEM_T, typename FIELD_T>
int find_item_with_field(const ITEM_T* item_array, int num_items, FIELD_T ITEM_T::* field, const FIELD_T& search)
{
	for (int i = 0; i < num_items; ++i)
		if (item_array[i].*field == search)
			return i;

	return -1;
}

template <typename VECTOR_T>
int find_item_with_name(const VECTOR_T& item_vector, const char* str)
{
	if (!str)
		return -1;

	int index = 0;
	for (const auto& item : item_vector)
	{
		if (!stricmp(item.name, str))
			return index;
		else
			++index;
	}

	return -1;
}

template <typename ITEM_T>
int find_item_with_name(const ITEM_T* item_array, int num_items, const char* str)
{
	return find_item_with_field(item_array, num_items, &ITEM_T::name, str);
}

template <typename VECTOR_T>
int find_item_with_name(const VECTOR_T& item_vector, const SCP_string& str)
{
	int index = 0;
	for (const auto& item : item_vector)
	{
		if (lcase_equal(item.name, str))
			return index;
		else
			++index;
	}

	return -1;
}

template <typename NULLISH_T>
NULLISH_T coalesce(NULLISH_T possibly_null, NULLISH_T value_if_null)
{
	Assertion(value_if_null != nullptr, "value_if_null can never be null itself!");

	return (possibly_null != nullptr) ? possibly_null : value_if_null;
}

#endif
