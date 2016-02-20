#ifndef RUBIK_BIGINT_H
#define RUBIK_BIGINT_H

#include <vector>
#include <string>
using namespace std;

// bigint structure to bypass the need to use std::vector, which is really slow in this kind of
// application, due to dynamic memory allocations and constant reallocations
struct bigint
{
    // data
    int d[48];
    // size of data really used
    int size;

    // implicit constructor
    bigint()
    {
        size = 0;
    }

    // constructor using size - also puts 0 to all data
    bigint(int size)
    {
        this->size = size;
        memset(d, 0, sizeof(int)*size);
    }

    // copy constructor
    bigint(bigint const& old)
    {
        size = old.size;
        memcpy(d, old.d, sizeof(int)*size);
    }

    // copy constructor with specified beginning and end
    bigint(bigint &src, int begin, int end)
    {
        size = end - begin + 1;
        memset(d, 0, sizeof(int)*size);
        for (int i = begin; i < end; i++)
            d[i - begin] = src.d[i];
    }

    // assignment operator - just copy data, not whole structure
    bigint& operator=(bigint &src)
    {
        size = src.size;
        memcpy(d, src.d, sizeof(int)*size);
        return *this;
    }

    // both structures are equal when they have equivalent size and
    // all the data are same
    bool operator==(bigint const& a)
    {
        if (size != a.size)
            return false;
        for (int i = 0; i < size; i++)
            if (d[i] != a.d[i])
                return false;
        return true;
    }

    // unequality is just negation of equality
    bool operator!=(bigint const& a)
    {
        return !(*this == a);
    }

    // less comparator is almost the same as the one within std::vector
    // it came out as most efficient one to this kind of application
    bool operator<(bigint const& a) const
    {
        if (size != a.size)
            return true;

        // lexicographical compare is done in std::vector also to ensure
        // all needed properties of less comparator
        return lexicographical_compare(d, d + size, a.d, a.d + a.size);
    }

    // just copy method (duplicating existing structure)
    void copy(bigint const& a)
    {
        size = a.size;
        memcpy(d, a.d, sizeof(int)*size);
    }
};

#endif
