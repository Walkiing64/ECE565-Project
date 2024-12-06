#include <stdlib.h>
#include <stdio.h>

int main() {
    //Initialize a seed
    //srand(27);

    //Create a heap allocated array
    size_t size = 8;
    int* array = (int*) malloc(sizeof(*array) * size);

    //Check that memory was created
    if(array == NULL) {
        return 1;
    }

    //Initialize the array to be random constant values between 0 and 99
    for(int i = 0; i < size; i++) {
        array[i] = i;
    }

    //Now loop through the array 8 times, adding up the values
    int sum = 0;
    for(int i = 0; i < 32; i++) {
        sum += array[0];
        sum += array[1];
        sum += array[2];
        sum += array[3];
        sum += array[4];
        sum += array[5];
        sum += array[6];
        sum += array[7];
    }

    //Print sum
    printf("sum = %d", sum); 

    return 0;
}