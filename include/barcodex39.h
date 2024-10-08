#include <stdio.h>
#include <stdbool.h>

enum barCodeMode
{
    Fixed = 1,
    Auto  = 2
};

enum barCodeResult
{
    None = 0,
    WidthNotCompliant = 1,
    Done = 2,
};

typedef struct barcodex39opt
{
    int width;              /* width  */
    int height;             /* height */
    int narrowLineWidth;    
    enum barCodeMode mode;  
    bool showText;
} barcodex39opt;

typedef struct barcodeImageStr
{
    int width;        
    int heigth; 
    char* image;
    enum barCodeResult result;
} barcode39Data;


barcodex39opt intialize(int width, int heigth);

barcode39Data* generate(barcodex39opt opt, char *value);

void savepng(barcode39Data *barCodeImage, char *fileName);


