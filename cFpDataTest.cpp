#include <iostream>
#include <stdint.h>
#include "cFpData.h"
#include "intrin.h"

using namespace nFp;

class cFpDataTest{
public:
	static uint32_t TestAddDouble(uint32_t numTests, cFpData::eRoundMode rmode = cFpData::ROUND_NEAREST_EVEN);
	static uint32_t TestSubDouble(uint32_t numTests, cFpData::eRoundMode rmode = cFpData::ROUND_NEAREST_EVEN);
	static uint32_t TestMulDouble(uint32_t numTests, cFpData::eRoundMode rmode = cFpData::ROUND_NEAREST_EVEN);
	static uint32_t TestDivDouble(uint32_t numTests, cFpData::eRoundMode rmode = cFpData::ROUND_NEAREST_EVEN);

	static uint32_t TestCompareDouble(uint32_t numTests);

	static void SetX86RMode(cFpData::eRoundMode rmode = cFpData::ROUND_NEAREST_EVEN);

	static double GetRandomDouble(uint32_t upperLimit) {
		double d0 = static_cast <double> (rand()) / static_cast <double> (RAND_MAX) * upperLimit;
		d0 = (rand() & 0x1) ? d0 : -d0;

		return d0;
	}

	static uint64_t double_to_uint64(double number) {
		return *(uint64_t*)&number;
	}

	static double uint64_to_double(uint64_t number) {
		return *(double*)&number;
	}

};

void cFpDataTest::SetX86RMode(cFpData::eRoundMode rmode) {
	unsigned int csr = _mm_getcsr();

	if (rmode == cFpData::ROUND_NEAREST_EVEN) {
		_mm_setcsr(csr & 0x9FFF);
	}
	else if (rmode == cFpData::ROUND_TOWARDS_ZERO) {
		_mm_setcsr((csr & 0x9FFF) | 0x6000);
	}
	else if (rmode == cFpData::ROUND_POS_INF) {
		_mm_setcsr((csr & 0x9FFF) | 0x4000);
	}
	else if (rmode == cFpData::ROUND_NEG_INF) {
		_mm_setcsr((csr & 0x9FFF) | 0x2000);
	}

}

uint32_t cFpDataTest::TestAddDouble(uint32_t numTests, cFpData::eRoundMode rmode) {
	uint32_t errors = 0;

	SetX86RMode(rmode);

	for (uint32_t i = 0; i < numTests; i++) {

		double d0 = GetRandomDouble(15);
		double d1 = GetRandomDouble(15);

		double d_result = d0 + d1;

		cFpData fp0(d0);
		cFpData fp1(d1);

		cFpData fp_result = fp0 + fp1;
		fp_result.round_mode = rmode;

		double fp_result_d = fp_result.to_double();

		if (d_result != fp_result_d) {
			uint64_t d_result_int = *(uint64_t*)&d_result;
			uint64_t fp_result_d_int = *(uint64_t*)&fp_result_d;
			uint64_t diff = d_result_int ^ fp_result_d_int;
			printf("Error Add d0:%f(0x%llx) d1:%f(0x%llx) d_result:%f(0x%llx) fp_result_d:%f(0x%llx) diff:0x%llx\n", d0, double_to_uint64(d0),
				                                                                                                     d1, double_to_uint64(d1),
				                                                                                                     d_result, double_to_uint64(d_result), 
				                                                                                                     fp_result_d, double_to_uint64(fp_result_d), 
				                                                                                                     diff);

			errors++;

		}

	}

	SetX86RMode();

	return errors;
}

uint32_t cFpDataTest::TestSubDouble(uint32_t numTests, cFpData::eRoundMode rmode) {
	uint32_t errors = 0;

	SetX86RMode(rmode);

	for (uint32_t i = 0; i < numTests; i++) {

		double d0 = GetRandomDouble(15);
		double d1 = GetRandomDouble(15);

		double d_result = d0 - d1;

		cFpData fp0(d0);
		cFpData fp1(d1);

		cFpData fp_result = fp0 - fp1;
		fp_result.round_mode = rmode;

		double fp_result_d = fp_result.to_double();

		if (d_result != fp_result_d) {
			uint64_t d_result_int = *(uint64_t*)&d_result;
			uint64_t fp_result_d_int = *(uint64_t*)&fp_result_d;
			uint64_t diff = d_result_int ^ fp_result_d_int;
			printf("Error Add d0:%f(0x%llx) d1:%f(0x%llx) d_result:%f(0x%llx) fp_result_d:%f(0x%llx) diff:0x%llx\n", d0, double_to_uint64(d0),
				d1, double_to_uint64(d1),
				d_result, double_to_uint64(d_result),
				fp_result_d, double_to_uint64(fp_result_d),
				diff);

			errors++;

		}

	}

	SetX86RMode();

	return errors;
}

uint32_t cFpDataTest::TestMulDouble(uint32_t numTests, cFpData::eRoundMode rmode) {
	uint32_t errors = 0;

	SetX86RMode(rmode);

	for (uint32_t i = 0; i < numTests; i++) {
		double d0 = GetRandomDouble(15);
		double d1 = GetRandomDouble(15);

		double d_result = d0 * d1;

		cFpData fp0(d0);
		cFpData fp1(d1);

		cFpData fp_result = fp0 * fp1;
		fp_result.round_mode = rmode;

		double fp_result_d = fp_result.to_double();

		if (d_result != fp_result_d) {
			uint64_t d_result_int = *(uint64_t*)&d_result;
			uint64_t fp_result_d_int = *(uint64_t*)&fp_result_d;
			uint64_t diff = d_result_int ^ fp_result_d_int;
			printf("Error Mul d0:%f d1:%f d_result:%f(0x%llx) fp_result_d:%f(0x%llx) diff:0x%llx\n", d0, d1, d_result, d_result_int, fp_result_d, fp_result_d_int, diff);

			errors++;

		}

	}

	SetX86RMode();

	return errors;
}

uint32_t cFpDataTest::TestDivDouble(uint32_t numTests, cFpData::eRoundMode rmode) {
	uint32_t errors = 0;

	SetX86RMode(rmode);

	for (uint32_t i = 0; i < numTests; i++) {
		double d0 = GetRandomDouble(15);
		double d1 = GetRandomDouble(15);

		double d_result = d0 / d1;

		cFpData fp0(d0);
		cFpData fp1(d1);

		cFpData fp_result = fp0 / fp1;
		fp_result.round_mode = rmode;

		double fp_result_d = fp_result.to_double();

		if (d_result != fp_result_d) {
			uint64_t d_result_int = *(uint64_t*)&d_result;
			uint64_t fp_result_d_int = *(uint64_t*)&fp_result_d;
			uint64_t diff = d_result_int ^ fp_result_d_int;
			printf("Error Div d0:%f d1:%f d_result:%f(0x%llx) fp_result_d:%f(0x%llx) diff:0x%llx\n", d0, d1, d_result, d_result_int, fp_result_d, fp_result_d_int, diff);

			errors++;
		}
	}

	SetX86RMode();

	return errors;
}

uint32_t cFpDataTest::TestCompareDouble(uint32_t numTests) {
	uint32_t errors = 0;

	double posZero = +0.0f;
	double negZero = -0.0f;

	if (cFpData(posZero) != cFpData(negZero) ||
		!(cFpData(posZero) == cFpData(negZero))) {
		printf("Error: 1. +Zero should equal to -Zero.\n");
		errors++;
	}

	if (cFpData(posZero) > cFpData(negZero)) {
		printf("Error: 2. +Zero should equal to -Zero.\n");
		errors++;
	}

	if (cFpData(posZero) < cFpData(negZero)) {
		printf("Error: 3. +Zero should equal to -Zero.\n");
		errors++;
	}

	for (uint32_t i = 0; i < numTests; i++) {
		double d0 = GetRandomDouble(15);
		double d1 = GetRandomDouble(15);

		if (d0 < d1) {
			if (!(cFpData(d0) < cFpData(d1))) {
				printf("Error Compare d0:%f d1:%f\n", d0, d1);
				errors++;
			}
		}

		if (d0 > d1) {
			if (!(cFpData(d0) > cFpData(d1))) {
				printf("Error Compare d0:%f d1:%f\n", d0, d1);
				errors++;
			}
		}
	}


	return errors;

}

int main() {
	//cFpData d0(cFpDataTest::uint64_to_double(0xc01fcee79dcf3b9f));
	//cFpData d1(cFpDataTest::uint64_to_double(0x400121c243848709));

	//cFpData d_result = d0 + d1;
	//d_result.Print();

	cFpDataTest::TestCompareDouble(100000);

	cFpDataTest::TestAddDouble(100000, cFpData::ROUND_NEAREST_EVEN);
	cFpDataTest::TestAddDouble(100000, cFpData::ROUND_POS_INF);
	cFpDataTest::TestAddDouble(100000, cFpData::ROUND_NEG_INF);
	cFpDataTest::TestAddDouble(100000, cFpData::ROUND_TOWARDS_ZERO);

	cFpDataTest::TestSubDouble(100000, cFpData::ROUND_NEAREST_EVEN);
	cFpDataTest::TestSubDouble(100000, cFpData::ROUND_POS_INF);
	cFpDataTest::TestSubDouble(100000, cFpData::ROUND_NEG_INF);
	cFpDataTest::TestSubDouble(100000, cFpData::ROUND_TOWARDS_ZERO);

	cFpDataTest::TestMulDouble(100000, cFpData::ROUND_NEAREST_EVEN);
	cFpDataTest::TestMulDouble(100000, cFpData::ROUND_POS_INF);
	cFpDataTest::TestMulDouble(100000, cFpData::ROUND_NEG_INF);
	cFpDataTest::TestMulDouble(100000, cFpData::ROUND_TOWARDS_ZERO);

	cFpDataTest::TestDivDouble(100000, cFpData::ROUND_NEAREST_EVEN);
	cFpDataTest::TestDivDouble(100000, cFpData::ROUND_POS_INF);
	cFpDataTest::TestDivDouble(100000, cFpData::ROUND_NEG_INF);
	cFpDataTest::TestDivDouble(100000, cFpData::ROUND_TOWARDS_ZERO);



	printf("Test done.\n");

	return 0;

}