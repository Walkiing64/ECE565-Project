#include <iostream>
#include <stdlib.h>

int main() {
    //Initialize a seed
    srand(27);

    //Create a heap allocated array
    size_t size = 16;
    int* array = (int*) malloc(sizeof(*array) * size);

    //Check that memory was created
    if(array == nullptr) {
        return 1;
    }

    //Initialize the array to be random constant values between 0 and 99
    for(int i = 0; i < size; i++) {
        array[i] = rand() % 100;
    }

    //Now loop through the array 8 times, adding up the values
    int sum = 0;
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < size; j++) {
            sum += array[j];
        }
    }

    //Print sum
    std::cout << sum;

    return 0;
}