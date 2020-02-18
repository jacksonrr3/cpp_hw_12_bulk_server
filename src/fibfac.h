#pragma once
#include <cstdint>

std::int64_t fib(int a) {
	if (a == 0 || a == 1) { return 0; }
	else {
		return fib(a - 1) + fib(a - 2);
	}
}

std::int64_t fac(int a) {
	if (a == 0) { return 1; }
	else {
		std::int64_t res = 1;
		for (int i = 1; i <= a; i++) {
			res *= i;
		}
		return res;
	}
}
