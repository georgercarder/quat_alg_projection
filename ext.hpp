#include <gmp.h>
#include <stdint.h>

mpz_t* generatePrimeMPZs(const int numberToGen, 
			 const int lowerBitCnt, 
			 const int upperBitCnt,
			 const unsigned long int seed);
