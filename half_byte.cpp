#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <iostream>
#include <pthread.h>
// -lpthread

#include <gmp.h>
// -lgmp
// mpz_nextprime

#include "./ext.hpp"

// TODO MAKE QUATERNION LIBRARY USE MPI

// TODO FIXME handle memory leaks

// TODO CONST IN FUNCTION ARGS

// TODO MAKE MULT AND ADD IN THREADS
// TODO use GMP for scalars
//
// DERIVE CLASS FROM HALFBYTEARRAY.. QUATERNION ARRAY

static const int g_maxArrLength = 1000000;
//static const int g_threadParamAdd = 10000; // TODO MAKE THREAD PARAM INTELLIGENT
// TODO BENCHMARK THREAD ADD VS SINGLE THREAD
//static const int g_threadParamMult = 1000000;

class HalfByteArray;
class QuaternionAlgOverZ_2Array;

HalfByteArray* AddHalfByteArrays(HalfByteArray *a, HalfByteArray *b);
typedef QuaternionAlgOverZ_2Array Quat;
Quat* MultQuatArrays(Quat *a, Quat *b);
uint8_t MultQuatByte(uint8_t *a , uint8_t *b);

struct addThreadPayload {
	HalfByteArray *m_aHBA;
	HalfByteArray *m_bHBA;
	int m_idx;
	int m_threadParamAdd;
	HalfByteArray *m_result;
	addThreadPayload() {}
	addThreadPayload(HalfByteArray *aHBA, HalfByteArray *bHBA, int idx, int threadParamAdd) {
		m_aHBA = aHBA;
		m_bHBA = bHBA;
		m_idx = idx;	
		m_threadParamAdd = threadParamAdd;	
	}
};


class HalfByteArray {
	public:
		HalfByteArray() {}

		HalfByteArray(int length) {
			printf("hello halfByteArray %d\n", length);
			m_length = length;
			m_halfByteArray = (uint8_t*)calloc(m_length, sizeof(uint8_t));
		}

		HalfByteArray(uint8_t *byteArray, int length) {
			m_length = length;
			m_halfByteArray = byteArray;
		}


		HalfByteArray* SubArray(int idx, int length) {
			HalfByteArray *ret = new(HalfByteArray);
			uint8_t *byteArray = (uint8_t*)calloc(length, sizeof(uint8_t));
			int modLength;
			(m_length >= length) ? modLength = length : modLength = m_length;
			memcpy(byteArray, m_halfByteArray, modLength);
			*ret = HalfByteArray(byteArray, length);
			return ret;	
		}

		HalfByteArray* CombineSubArrays(HalfByteArray **arr, int numberOfEntries) {
			int totalArrayLength = 0;
			for (int i = 0; i < numberOfEntries; ++i) {
				totalArrayLength += arr[i]->GetLength();	
			}
			uint8_t *totalByteArray = (uint8_t*)calloc(totalArrayLength, sizeof(uint8_t));
			int offset = 0;
			for (int i = 0; i < numberOfEntries; ++i) {
				int currentLength = arr[i]->GetLength();
				memcpy(totalByteArray + (offset * sizeof(uint8_t)), arr[i]->GetByteArray(), currentLength);
				offset += currentLength;
			}
			HalfByteArray *ret = new(HalfByteArray);
			*ret = HalfByteArray(totalByteArray, totalArrayLength);
			return new(HalfByteArray);	
		}

		HalfByteArray* CombineSubArrays(addThreadPayload **arr, int numberOfEntries) {
			int totalArrayLength = 0;
			for (int i = 0; i < numberOfEntries; ++i) {
				totalArrayLength += arr[i]->m_result->GetLength();	
			}
			uint8_t *totalByteArray = (uint8_t*)calloc(totalArrayLength, sizeof(uint8_t));
			int offset = 0;
			for (int i = 0; i < numberOfEntries; ++i) {
				int currentLength = arr[i]->m_result->GetLength();
				memcpy(totalByteArray + (offset * sizeof(uint8_t)), arr[i]->m_result->GetByteArray(), currentLength);
				offset += currentLength;
			}
			HalfByteArray *ret = new(HalfByteArray);
			*ret = HalfByteArray(totalByteArray, totalArrayLength);
			return new(HalfByteArray);	
		}

		int GetLength() {
			return m_length;
		}

		uint8_t* GetByteArray() {
			uint8_t *copyOfByteArray = (uint8_t*)calloc(m_length, sizeof(uint8_t));
			memcpy(copyOfByteArray, m_halfByteArray, m_length);
			return copyOfByteArray;
		}

	private:
		int m_length = 0;
		uint8_t *m_halfByteArray = NULL;
};


void *addThread(void *ptr) {
	addThreadPayload *tp = (addThreadPayload*)ptr;
	HalfByteArray *hChild;  
	hChild = tp->m_aHBA->SubArray(tp->m_idx, tp->m_threadParamAdd);
	HalfByteArray *bbChild;
	bbChild = tp->m_bHBA->SubArray(tp->m_idx, tp->m_threadParamAdd); 

	HalfByteArray *sum = new(HalfByteArray);
	sum = AddHalfByteArrays(hChild, bbChild);
	tp->m_result = sum;
	
	void *ret;
	return ret;
}

class QuaternionAlgOverZ_2Array : public HalfByteArray {
	public:
		QuaternionAlgOverZ_2Array() {}
		QuaternionAlgOverZ_2Array(uint8_t *byteArray, int length) : HalfByteArray(byteArray, length) {}

		QuaternionAlgOverZ_2Array* operator+(QuaternionAlgOverZ_2Array &b) {
			HalfByteArray *h = (HalfByteArray*)this;	
			HalfByteArray *bb = (HalfByteArray*)&b;
			HalfByteArray *cHBA;
			int hLength = h->GetLength();
		        int bbLength = bb->GetLength();
			cHBA = AddHalfByteArrays(h, bb);
			return (QuaternionAlgOverZ_2Array*)cHBA;
		}
		QuaternionAlgOverZ_2Array* operator*(QuaternionAlgOverZ_2Array &b) {
			QuaternionAlgOverZ_2Array *cQ = new(QuaternionAlgOverZ_2Array);
			cQ = MultQuatArrays(this, &b);
			return cQ;
		}

	
	private:
};

typedef QuaternionAlgOverZ_2Array Quat;

std::ostream& operator<<(std::ostream& os, HalfByteArray *h) {
	int hLength = h->GetLength();
	uint8_t *hArray = h->GetByteArray();
	
	os << "{length: " << hLength
	       << ", byteArray: " << hArray[0];
	for (int i = 1; i < hLength; ++i) {
		os << ", " << hArray[i];
	}
	os << "}";

	return os;
}

HalfByteArray* AddHalfByteArrays(HalfByteArray *a, HalfByteArray *b) {
	uint8_t* aArray = a->GetByteArray();
	uint8_t* bArray = b->GetByteArray();
	int aLength = a->GetLength();
	int bLength = b->GetLength();
	
	int sumLength; 
	(aLength > bLength) ? sumLength = aLength : sumLength = bLength;
	uint8_t* sum = (uint8_t*)calloc(sumLength, sizeof(uint8_t));
	if (aLength > bLength) {
		for (int i = 0; i < bLength; ++i) {
			aArray[i] = aArray[i] ^ bArray[i];
		}
		sum = aArray;	
	} else {
		for (int i = 0; i < aLength; ++i) {
			bArray[i] = aArray[i] ^ bArray[i];
		}	
		sum = bArray;
	}

	HalfByteArray *ret = new(HalfByteArray);
	*ret = HalfByteArray(sum, sumLength);
	return ret;
}

Quat* MultQuatArrays(Quat *a, Quat *b) {
	uint8_t* aArray = a->GetByteArray();
	uint8_t* bArray = b->GetByteArray();
	int aLength = a->GetLength();
	int bLength = b->GetLength();
	
	int prodLength;
	(aLength > bLength) ? prodLength = aLength : prodLength = bLength;

	uint8_t* prod = (uint8_t*)calloc(prodLength, sizeof(uint8_t));
	if (aLength > bLength) {
		for (int i = 0; i < bLength; ++i) {
			aArray[i] = MultQuatByte(&aArray[i], &bArray[i]);
		}
		prod = aArray;	
	} else {
		for (int i = 0; i < aLength; ++i) {
			bArray[i] = MultQuatByte(&aArray[i], &bArray[i]);
		}	
		prod = bArray;
	}
	Quat *ret = new(Quat);
	*ret = Quat(prod, prodLength);
	return ret;
}

uint8_t MultQuatByte(uint8_t *a , uint8_t *b) {
	uint8_t ret = 0x00;
	// 1st quat
	{
		// TODO DOUBLE CHECK FORMULA	
		// RECALL
		//  (A + jB) * (C + jD)
		//     = AC + (j^2)conj(B)D + jBC + jconj(A)D
		
		uint8_t A1 =  *a & 0x01;
		uint8_t A2 = (*a & 0x02) >> 1;
		
		uint8_t B1 = (*a & 0x04) >> 2;
		uint8_t B2 = (*a & 0x08) >> 3;	

		
		uint8_t C1 =  *b & 0x01;
		uint8_t C2 = (*b & 0x02) >> 1;
		
		uint8_t D1 = (*b & 0x04) >> 2;
		uint8_t D2 = (*b & 0x08) >> 3;

		// CHANGED OPS
		// * -> &, + -> ^ since over Z_2
		
		uint8_t one = (A1 & C1) ^ (A2 & C2) ^ (B2 & D2) ^ (B1 & D1);
		uint8_t i   = (A1 & C2) ^ (A2 & C1) ^ (B1 & D2) ^ (B2 & D1);
	       	uint8_t j   = (B1 & C1) ^ (B2 & C2) ^ (A1 & D1) ^ (A2 & D2);
		uint8_t k   = (A2 & D1) ^ (B1 & C2) ^ (B2 & C1) ^ (A1 & D2);

		ret |= one;
		ret |= (i << 1);	
		ret |= (j << 2);
		ret |= (k << 3);
	}
	// 2nd quat
	uint8_t halfbyte = 4;
	{
		uint8_t A1 =  *a & 0x10  >>  halfbyte;
		uint8_t A2 = (*a & 0x20) >> (1 + halfbyte);
		
		uint8_t B1 = (*a & 0x40) >> (2 + halfbyte);
		uint8_t B2 = (*a & 0x80) >> (3 + halfbyte);	

		
		uint8_t C1 =  *b & 0x10  >>  halfbyte;
		uint8_t C2 = (*b & 0x20) >> (1 + halfbyte);
		
		uint8_t D1 = (*b & 0x40) >> (2 + halfbyte);
		uint8_t D2 = (*b & 0x80) >> (3 + halfbyte);
	
		uint8_t one = (A1 & C1) ^ (A2 & C2) ^ (B2 & D2) ^ (B1 & D1);
		uint8_t i   = (A1 & C2) ^ (A2 & C1) ^ (B1 & D2) ^ (B2 & D1);
	       	uint8_t j   = (B1 & C1) ^ (B2 & C2) ^ (A1 & D1) ^ (A2 & D2);
		uint8_t k   = (A2 & D1) ^ (B1 & C2) ^ (B2 & C1) ^ (A1 & D2);
	
		ret |= one << halfbyte;
		ret |= (i << (1 + halfbyte));	
		ret |= (j << (2 + halfbyte));
		ret |= (k << (3 + halfbyte));
	}
	return ret;
}

int main() {
//	printf("test %s \n", TESTFUNCTION());
	/*
	signed long int sli = (signed long int)22;
	printf("sli %Zd \n", sli);
	mpz_t integ;
	mpz_init(integ);
	mpz_init_set_si(integ, sli);
	gmp_printf("my first mpi %Zd \n", integ);
	//std::cout << integ << "\n";*/
	
	printf("Generation random inputs\n");
	// build a
	int aLength = rand() % g_maxArrLength;
	uint8_t *aArray = (uint8_t*)calloc(aLength, sizeof(uint8_t));
	for (int i = 0; i < aLength; ++i) {
		aArray[i] = (uint8_t)rand();	
	}
	HalfByteArray *aHBA = new(HalfByteArray);
	*aHBA = HalfByteArray(aArray, aLength);

	//std::cout << aHBA;
	
	// build b
	int bLength = rand() % g_maxArrLength;
	uint8_t *bArray = (uint8_t*)calloc(bLength, sizeof(uint8_t));
	for (int i = 0; i < bLength; ++i) {
		bArray[i] = (uint8_t)rand();	
	}
	HalfByteArray *bHBA = new(HalfByteArray);
	*bHBA = HalfByteArray(bArray, bLength);

	//std::cout << bHBA;	

	// c = a + b
	HalfByteArray *cHBA = new(HalfByteArray);	
	cHBA = AddHalfByteArrays(aHBA, bHBA);

	//std::cout << cHBA;

	Quat *q = new(Quat);
	Quat *qb = new(Quat);

	*q = Quat(aArray, aLength);
	*qb = Quat(bArray, bLength);
/*
	Quat *qc;
	std::cout << "before add";
	qc = *q + *qb;

	std::cout << "\n---\n";
	std::cout << qc << "\n";
*/
	Quat *qProd;
	std::cout << "before mult";
	qProd = *q * *qb;
	std::cout << "\n---\n";
	std::cout << qProd;
		
	/*	
	int numberToGenerate = 100;
	int lowerBitCnt = 128;
	int upperBitCnt = 256;
	unsigned long int seed = 123; // this is likely key later
	printf("genprimes...\n");
	mpz_t *primes = generatePrimeMPZs(numberToGenerate, lowerBitCnt, upperBitCnt, seed);	
	printf("genprimes\n");
	*/	
	
	return 0;
}
