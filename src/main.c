#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "..\include\barcodex39.h"


/*Questa è la versione funzionante*/
int main()
{
    printf("BarCode\n");

    // //char value[100] ="-.$/+%";          // Allocazione di memoria per la stringa    
    // //scanf("%99s", value);             // Legge una stringa di massimo 99 caratteri

   
    char *originalString = "DAVIDE";
  
    //Create options.
    barcodex39opt opt;
    opt.width  = 400;
    opt.height = 100;
    opt.mode = Fixed;
    opt.showText = false;
    opt.narrowLineWidth = 1;
  
    barcode39Data* result = calculate(opt, originalString);
    printf("Result: %d", result->result);
    
    savepng(result,"barcode.png");
    

    // if (IsValidString(originalString))
    // {
    //     printf("Valida Valore %s", originalString);
    // }
    // else
    // {
    //     printf("Non Valida Valore %s", originalString);      
    //     exit(1);
    // }



    // Decode(originalString, destination);
    // printf("Result: %s\n" ,destination);

    

    // char originalString[] = "DAVIDE";
    // int width = 400, height = 100;
    // unsigned char* image = CreateBarCode(width, height, originalString);
    // if (image==NULL)
    // {
    //     exit(1);
    // }

    // WriteBarCode(image, "barcode.png", width, height);

    // free(image);
    // image = NULL;  

    // printf("BarCode Ciao\n");
 

    return 0;
}
