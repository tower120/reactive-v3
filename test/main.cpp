#include <iostream>

#include "TestObservableProperty.h"
#include "Benchmark.h"

int main() {
    std::cout << sizeof(reactive::nonblocking::ObservableProperty<int, false>) << std::endl;

    //TestObservableProperty().test_all();
    Benchmark().benchmark_all();


	char ch;
	std::cin >> ch;

    return 0;
}