#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "..\include\barcodex39.h"


int main()
{
    printf("BarCode\n");
    char *originalString = "DAVIDE";
  
    //Create options.
    barcodex39opt opt = intialize(400,100);
    // opt.width  = 400;
    // opt.height = 100;
    // opt.mode = Fixed;
    // opt.showText = false;
    // opt.narrowLineWidth = 1;
  

  
    barcode39Data* result = generate(opt, originalString);
    
    printf("Result: %d", result->result);
    
    savepng(result,"barcode.png");
    

    
 

    return 0;
}
