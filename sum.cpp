#include "datapar.hpp"

int main() {
	pasl::pctl::parray<int> xs = { 43, 3222, 11232, 30, 9, -3 };
	int r = pasl::pctl::sum(xs.begin(), xs.end());
	std::cout << "sum(xs) = " << r << std::endl;
	return 0;
}
