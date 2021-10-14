/* floats.h - header for a float types
 */
#ifndef _FLOATS_H
#define _FLOATS_H

#define HAVE_SIZEOF_FLOAT
#define HAVE_SIZEOF_DOUBLE
#define HAVE_SIZEOF_LONG_DOUBLE

#define SIZEOF_FLOAT         4
#define SIZEOF_DOUBLE        8
#define SIZEOF_LONG_DOUBLE   8

#if SIZEOF_FLOAT == 2
# define HAVE_FLOAT16_T
typedef float float16_t;
#elif SIZEOF_FLOAT == 4
# define HAVE_FLOAT32_T
typedef float float32_t;
#elif SIZEOF_FLOAT == 8
# define HAVE_FLOAT64_T
typedef float float64_t;
#else
# error "Non-standard size of float" # SIZEOF_FLOAT
#endif

#if SIZEOF_DOUBLE == 4
# define HAVE_FLOAT32_T
typedef double float32_t;
#elif SIZEOF_DOUBLE == 8
# define HAVE_FLOAT64_T
typedef double float64_t;
#elif SIZEOF_DOUBLE == 10
# define HAVE_FLOAT80_T
typedef double float80_t;
#elif SIZEOF_DOUBLE == 16
# define HAVE_FLOAT128_T
typedef double double128_t;
#else
# error "Non-standard size of double" # SIZEOF_DOUBLE
#endif

#ifdef HAVE_LONG_DOUBLE
# if SIZEOF_LONG_DOUBLE == 8
#  ifndef HAVE_FLOAT64_T
#   define HAVE_FLOAT64_T
typedef  long double float64_t;
#  endif
# elif SIZEOF_LONG_DOUBLE == 10
#  ifndef HAVE_FLOAT80_T
#   define HAVE_FLOAT80_T
typedef long double float80_t;
#  endif()
# elif SIZEOF_LONG_DOUBLE == 16
#  ifndef HAVE_FLOAT128_T
#   define HAVE_FLOAT128_T
typedef long double float128_t;
#  endif
#else
#  error "Non-standard size of long double" # SIZEOF_DOUBLE
# endif
#endif

#endif /* _FLOATS_H */
