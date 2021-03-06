/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Templates and utility-routines for strings and numbers.
 */

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include "global.hpp"
#include <cmath>
#include <math.h> // cmath may not provide round()
#include <vector>
#include <sstream>

template<typename T>
inline bool is_even(T num) { return num % 2 == 0; }

template<typename T>
inline bool is_odd(T num) { return !is_even(num); }

/**
 * Returns base + increment, but will not increase base above max_sum, nor
 * decrease it below min_sum.
 * (If base is already beyond the applicable limit, base will be returned.)
 */
inline int bounded_add(int base, int increment, int max_sum, int min_sum=0) {
	if ( increment >= 0 )
		return std::min(base+increment, std::max(base, max_sum));
	else
		return std::max(base+increment, std::min(base, min_sum));
}

/** Guarantees portable results for division by 100; round towards 0 */
inline int div100rounded(int num) {
	return (num < 0) ? -(((-num) + 50) / 100) : (num + 50) / 100;
}

/**
 *  round (base_damage * bonus / divisor) to the closest integer,
 *  but up or down towards base_damage
 */
inline int round_damage(int base_damage, int bonus, int divisor) {
	if (base_damage==0) return 0;
	const int rounding = divisor / 2 - (bonus < divisor || divisor==1 ? 0 : 1);
	return std::max<int>(1, (base_damage * bonus + rounding) / divisor);
}

// not guaranteed to have exactly the same result on different platforms
inline int round_double(double d) {
#ifdef HAVE_ROUND
	return static_cast<int>(round(d)); //surprisingly, not implemented everywhere
#else
	return static_cast<int>((d >= 0.0)? std::floor(d + 0.5) : std::ceil(d - 0.5));
#endif
}

// Guaranteed to have portable results across different platforms
inline double round_portable(double d) {
	return (d >= 0.0) ? std::floor(d + 0.5) : std::ceil(d - 0.5);
}

struct bad_lexical_cast {};

template<typename To, typename From>
To lexical_cast(From a)
{
	To res;
	std::stringstream str;

	if(!(str << a && str >> res)) {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

template<typename To, typename From>
To lexical_cast_default(From a, To def=To())
{
	To res;
	std::stringstream str;

	if(!(str << a && str >> res)) {
		return def;
	} else {
		return res;
	}
}

template<>
size_t lexical_cast<size_t, const std::string&>(const std::string& a);

template<>
size_t lexical_cast<size_t, const char*>(const char* a);

template<>
size_t lexical_cast_default<size_t, const std::string&>(const std::string& a, size_t def);

template<>
size_t lexical_cast_default<size_t, const char*>(const char* a, size_t def);

template<>
long lexical_cast<long, const std::string&>(const std::string& a);

template<>
long lexical_cast<long, const char*>(const char* a);

template<>
long lexical_cast_default<long, const std::string&>(const std::string& a, long def);

template<>
long lexical_cast_default<long, const char*>(const char* a, long def);

template<>
int lexical_cast<int, const std::string&>(const std::string& a);

template<>
int lexical_cast<int, const char*>(const char* a);

template<>
int lexical_cast_default<int, const std::string&>(const std::string& a, int def);

template<>
int lexical_cast_default<int, const char*>(const char* a, int def);

template<>
double lexical_cast<double, const std::string&>(const std::string& a);

template<>
double lexical_cast<double, const char*>(const char* a);

template<>
double lexical_cast_default<double, const std::string&>(const std::string& a, double def);

template<>
double lexical_cast_default<double, const char*>(const char* a, double def);

template<>
float lexical_cast<float, const std::string&>(const std::string& a);

template<>
float lexical_cast<float, const char*>(const char* a);

template<>
float lexical_cast_default<float, const std::string&>(const std::string& a, float def);

template<>
float lexical_cast_default<float, const char*>(const char* a, float def);

template<typename From>
std::string str_cast(From a)
{
	return lexical_cast<std::string,From>(a);
}

template<typename To, typename From>
To lexical_cast_in_range(From a, To def, To min, To max)
{
	To res;
	std::stringstream str;

	if(!(str << a && str >> res)) {
		return def;
	} else {
		if(res < min) {
			return min;
		}
		if(res > max) {
			return max;
		}
		return res;
	}
}

template<typename Cmp>
bool in_ranges(const Cmp c, const std::vector<std::pair<Cmp, Cmp> >&ranges) {
	typename std::vector<std::pair<Cmp,Cmp> >::const_iterator range,
		range_end = ranges.end();
	for (range = ranges.begin(); range != range_end; ++range) {
		if(range->first <= c && c <= range->second) {
			return true;
		}
	}
	return false;
}

inline bool chars_equal_insensitive(char a, char b) { return tolower(a) == tolower(b); }
inline bool chars_less_insensitive(char a, char b) { return tolower(a) < tolower(b); }

#ifdef __GNUC__
#define LIKELY(a)    __builtin_expect((a),1) // Tells GCC to optimize code so that if is likely to happen
#define UNLIKELY(a)  __builtin_expect((a),0) // Tells GCC to optimize code so that if is unlikely to happen
#else
#define LIKELY(a)    a
#define UNLIKELY(a)  a
#endif

namespace util {

template<class T>
class unique_ptr
{
	T *ptr_;
	// Neither copyable nor assignable.
	unique_ptr(const unique_ptr &);
	unique_ptr &operator=(const unique_ptr &);
public:
	unique_ptr(T *p = NULL): ptr_(p) {}
	~unique_ptr() { delete ptr_; }

	void reset(T *p = NULL)
	{
		delete ptr_;
		ptr_ = p;
	}

	T *release()
	{
		T *p = ptr_;
		ptr_ = NULL;
		return p;
	}

	T *get() { return ptr_; }
	T *operator->() const { return ptr_; }
	T &operator*() const { return *ptr_; }
};

namespace detail {
	/// A struct that exists to implement a generic wrapper for std::find.
	/// Container should "look like" an STL container of Values.
	template<typename Container, typename Value> struct contains_impl {
		static bool eval(const Container & container, const Value & value)
		{
			typename Container::const_iterator end = container.end();
			return std::find(container.begin(), end, value) != end;
		}
	};
	/// A struct that exists to implement a generic wrapper for the find()
	/// member of associative containers.
	/// Container should "look like" an STL associative container.
	template<typename Container>
	struct contains_impl<Container, typename Container::key_type>  {
		static bool eval(const Container & container,
		                 const typename Container::key_type & value)
		{
			return container.find(value) != container.end();
		}
	};
}//namespace detail

/// Returns true iff @a value is found in @a container.
/// This should work whenever Container "looks like" an STL container of Values.
/// Normally this uses std::find(), but a simulated partial template specialization
/// exists when Value is Container::key_type. In this case, Container is assumed
/// an associative container, and the member function find() is used.
template<typename Container, typename Value>
inline bool contains(const Container & container, const Value & value)
{
	return detail::contains_impl<Container, Value>::eval(container, value);
}

}//namespace util


#if 1
# include <SDL_types.h>
typedef Sint32 fixed_t;
# define fxp_shift 8
# define fxp_base (1 << fxp_shift)

/** IN: float or int - OUT: fixed_t */
# define ftofxp(x) (fixed_t((x) * fxp_base))

/** IN: unsigned and fixed_t - OUT: unsigned */
# define fxpmult(x,y) (((x)*(y)) >> fxp_shift)

/** IN: unsigned and int - OUT: fixed_t */
# define fxpdiv(x,y) (((x) << fxp_shift) / (y))

/** IN: fixed_t - OUT: int */
# define fxptoi(x) ( ((x)>0) ? ((x) >> fxp_shift) : (-((-(x)) >> fxp_shift)) )

#else
typedef float fixed_t;
# define ftofxp(x) (x)
# define fxpmult(x,y) ((x)*(y))
# define fxpdiv(x,y) (static_cast<float>(x) / static_cast<float>(y))
# define fxptoi(x) ( static_cast<int>(x) )
#endif

#endif
