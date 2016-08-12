#ifndef _FLAGSET_
#define _FLAGSET_

#include <bitset>
#include <cstdint>

template <class T, size_t SIZE = static_cast < size_t >(T::NUM_VALUES)>
class flagset {
protected:
    std::bitset<SIZE> values;
public:

    bool operator [] (const T idx) const { return values[(static_cast < size_t >(idx))]; };

    flagset<T> operator & (const flagset<T>& other) const {
        flagset<T> result;
        result.values = this->values & other.values;
        return result;
    }

    flagset<T> operator + (const T flag) const {
        flagset<T> result = *this;
        result.set(flag);
        return result;
    }

    flagset<T>& operator += (const T flag) {
        this->set(flag);
        return *this;
    }

    flagset<T> operator - (const T flag) const {
        flagset<T> result = *this;
        result.remove(flag);
        return result;
    }

    flagset<T>& operator -= (const T flag) {
        this->remove(flag);
        return *this;
    }

    flagset<T> operator | (const flagset<T>& other) const {
        flagset<T> result;
        result.values = this->values | other.values;
        return result;
    }

    flagset<T>& operator |= (const flagset<T>& other) {
        this->values |= other.values;

        return *this;
    }

    void operator |= (const T flag) {
        set(flag);
    }

    bool operator == (const flagset<T>& other) const { return this->values == other.values; }
    bool operator != (const flagset<T>& other) const { return this->values != other.values; }

    void reset() { values.reset(); }

    flagset<T>& set(T idx, bool value = true) {
        Assert(idx >= T::NUM_VALUES);

        values.set(static_cast < size_t >(idx), value);
        return *this;
    }

    template<typename TIter>
    flagset<T>& set_multiple(TIter begin, TIter end) {
        auto current = begin;
        while (current != end)
        {
            set(*current, true);

            current = std::next(current);
        }

        return *this;
    }

    flagset<T>&  remove(T idx) {
        return set(idx, false);
    }

    template<typename TIter>
    flagset<T>& remove_multiple(TIter begin, TIter end) {
        auto current = begin;
        while (current != end)
        {
            set(*current, false);

            current = std::next(current);
        }

        return *this;
    }

    flagset<T>&  toggle(T idx) {
        values[static_cast < size_t >(idx)] = !values[static_cast < size_t >(idx)];

        return *this;
    }

    bool any_set() const { return values.any(); }
    bool none_set() const { return values.none(); }

    void from_long(std::uint64_t num) { values = num; }
    std::uint64_t to_long() const { return values.to_ulong(); }
};

#define FLAG_LIST(Type) enum class Type : size_t


#endif
