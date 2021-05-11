#include <iostream>
#include <string>
#include <omp.h>
#include <cassert>

using namespace std;

int main(){
	
	char a = 'a';
	char d = 'd';

	assert(a == 'a' || d == 'd');
	assert(d == 'a' || d == 'd');
	std::cout << a << ", " << d << std::endl;

	return 0;
}
