#include <stdlib.h>
#include <stdint.h>

#include <iostream>

#include "./ext.hpp"

mpz_t* generatePrimeMPZs(const int numberToGen, 
			 const int lowerBitCnt, 
			 const int upperBitCnt,
			 const unsigned long int seed) 
{
	mpz_t *ret = (mpz_t*)malloc(numberToGen * sizeof(mpz_t));
	gmp_randstate_t state;
	gmp_randinit_mt(state); 
	// initialized using Mersienne Twister algorithm
	gmp_randseed_ui(state, seed);

	signed long int iOne = (signed long int)1;
	mpz_t one;
	mpz_init_set_si(one, iOne);
	mpz_t mpLowerBound, mpUpperBound;
	mpz_init(mpLowerBound);
	mpz_init(mpUpperBound);
	mp_bitcnt_t mpLowerBitCnt = mp_bitcnt_t(lowerBitCnt);
	mp_bitcnt_t mpUpperBitCnt = mp_bitcnt_t(upperBitCnt);
	mpz_mul_2exp(mpLowerBound, one, mpLowerBitCnt);
	mpz_mul_2exp(mpUpperBound, one, mpUpperBitCnt);

	for (int i = 0; i < numberToGen; ++i) {
		mpz_t rand;
		mpz_init(rand);
		
		mpz_urandomb(rand, state, upperBitCnt);
		while (mpz_cmp(mpLowerBound, rand) > 0) {
			// ensuring that rand is above mpLowerBound
			mpz_urandomb(rand, state, upperBitCnt);
		}
		
		mpz_t prime;
		mpz_init(prime);
		mpz_nextprime(prime, rand);

		if (mpz_cmp(prime, mpUpperBound) > 0) {
			--i; 
			// do this entry again since 
			// mpz_nextprime skipped over upperBound	
		} else {
			// success!
			mpz_set(ret[i], prime); 
		       	gmp_printf("debug prime %Zd\n", prime);	
			//std::cout << "debug prime " << prime << "\n"; 
			// TODO FIGURE OUT C++ FLAGS
		}
	}
	return ret;
}
