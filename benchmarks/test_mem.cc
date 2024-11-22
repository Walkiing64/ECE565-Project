#include <iostream>

int main() {
    int i = 10;
    int* y = new int;
    *y = i;

    std::cout << *y;

    delete y;

    return 0;
}