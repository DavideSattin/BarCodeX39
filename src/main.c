#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "..\include\barcodex39.h"


int main()
{
    printf("BarCode\n");
    char *originalString = "DAVIDE";
  
    //Initialize options.
    barcodex39opt opt = intialize(400,100);
    
    //Generate barcode.
    barcode39Data* result = generate(opt, originalString);
    
    //Show the result.
    printf("Result: %d", result->result);
    
    //Save the barcode.
    savepng(result,"barcode.png");

    //Release the memory.
    destroy(&result);

    
 

    return 0;
}
