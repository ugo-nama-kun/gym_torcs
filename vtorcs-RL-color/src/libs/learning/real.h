#ifndef REAL_H
#define REAL_H

#ifdef real

#error "Real already defined in another header! Binaries might not link properly!"

#else /* real */

#ifdef USE_DOUBLE
typedef double real;
#else /* USE_DOUBLE */
typedef float real;
#endif /* USE_DOUBLE */

#endif /* real */


#endif /* REAL_H */
