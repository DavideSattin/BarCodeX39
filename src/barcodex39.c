#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "..\include\barcodex39.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION  
#include "..\include\stb_image_write.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "..\include\stb_truetype.h"

#define BARCODE39_DIGIT  9

typedef struct {
    char character;              // CHAR
    const char *nwpattern;       // Binary Pattern.
    int checkSum;                // Checksum.
} Code39Char;



static const Code39Char code39_table_const[] = {
    {'0', "NNNWWNWNN", 0},  // Char '0'
    {'1', "WNNWNNNNW", 1},  // Char '1'   {'1', "100100001", "WNNWNNNNW", 1}
    {'2', "NNWWNNNNW", 2},  // Char '2'
    {'3', "WNWWNNNNN", 3},  // Char '3'
    {'4', "NNNWWNNNW", 4},  // Char '4'
    {'5', "WNNWWNNNN", 5},  // Char '5'
    {'6', "NNWWWNNNN", 6},  // Char '6'
    {'7', "NNNWNNWNW", 7},  // Char '7'
    {'8', "WNNWNNWNN", 8},  // Char '8'
    {'9', "NNWWNNWNN", 9},  // Char '9'
    {'A', "WNNNNWNNW", 10},  // Char 'A'  {'A', "100001001", "WNNNNWNNW", 10},
    {'B', "NNWNNWNNW", 11},  // Char 'B'
    {'C', "WNWNNWNNN", 12},  // Char 'C'
    {'D', "NNNNWWNNW", 13},  // Char 'D'
    {'E', "WNNNWWNNN", 14},  // Char 'E'
    {'F', "NNWNWWNNN", 15},  // Char 'F'
    {'G', "NNNNNWWNW", 16},  // Char 'G'
    {'H', "WNNNNWWNN", 17},  // Char 'H'
    {'I', "NNWNNWWNN", 18},  // Char 'I'
    {'J', "NNNNWWWNN", 19},  // Char 'J'
    {'K', "WNNNNNNWW", 20},  // Char 'K'
    {'L', "NNWNNNNWW", 21},  // Char 'L'
    {'M', "WNWNNNNWN", 22},  // Char 'M'
    {'N', "NNNNWNNWW", 23},  // Char 'N'
    {'O', "WNNNWNNWN", 24},  // Char 'O'
    {'P', "NNWNWNNWN", 25},  // Char 'P'
    {'Q', "NNNNNNWWW", 26},  // Char 'Q'
    {'R', "WNNNNNWWN", 27},  // Char 'R'
    {'S', "NNWNNNWWN", 28},  // Char 'S'
    {'T', "NNNNWNWWN", 29},  // Char 'T'
    {'U', "WWNNNNNNW", 30},  // Char 'U'
    {'V', "NWWNNNNNW", 31},  // Char 'V'
    {'W', "WWWNNNNNN", 32},  // Char 'W'
    {'X', "NWNNWNNNW", 33},  // Char 'X'
    {'Y', "WWNNWNNNN", 34},  // Char 'Y'
    {'Z', "NWWNWNNNN", 35},  // Char 'Z'
    {'-', "NWNNNNWNW", 36},  // Char '-'
    {'.', "WWNNNNWNN", 37},  // Char '.'
    {' ', "NWWNNNWNN", 38},  // Char ' '
    {'*', "NWNNWNWNN", -1},  // Char '*'
    {'$', "NWNWNWNNN", 39},  // Char '$'
    {'/', "NWNWNNNWN", 40},  // Char '/'
    {'+', "NWNNNWNWN", 41},  // Char '+'
    {'%', "NNNWNWNWN", 42}   // Char '%'
};


static const Code39Char *code39_table = code39_table_const;

static const char code39_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *$/+%";

void drawCharOnBitmap(unsigned char* bitmap, int width, int heigth, int x_offset, int y_offset, stbtt_fontinfo* font, char character, float scale);

/// @brief Load the specified font.
/// @param fontPath 
/// @param fontSize 
/// @return 
unsigned char* loadFont(const char* fontPath, int* fontSize);

barcodex39opt intialize(int width, int heigth)
{
    barcodex39opt opt;
    opt.width  = width;
    opt.height = heigth;
    opt.mode =  Fixed;
    opt.narrowLineWidth = 1;
    opt.showText = true;
   
    return opt;
}

/// @brief generate the bar code. 
/// @param opt 
/// @param value 
/// @return 
barcode39Data* generate(barcodex39opt opt, char *value)
{
    if (value == NULL)
    {
        perror("Cannot set a char null pointer.");
        return NULL;
    }

    const size_t value_len = strlen(value);
    char* barcodeNewValue = malloc(sizeof(char) * (value_len + 2));
    if (barcodeNewValue==NULL)
    {
         fprintf(stderr, "Error on string memory allocation.\n");
         return NULL;
    }

    strcpy(barcodeNewValue, "*");    // Start character (*)    
    strcat(barcodeNewValue, value);
    strcat(barcodeNewValue, "*");    // end character (*)  

    //Barcode line width.
    int narrow = opt.narrowLineWidth;
    int wide = narrow * 3;
    
    int fontHeight = 28;                        //The font heigth.

    int bcodeWidth = 0;                         //The bar code width calculated.
    int bar_height = opt.height - fontHeight;   //The bar code lines height.

    char *barcodeValueStr = barcodeNewValue;     

    //Cycle the string
    while (*barcodeValueStr)
    {
        //Return a pointer of first char (*inputString) found in code39_chars.
        const char *pos = strchr(code39_chars, *barcodeValueStr);
        if (pos==NULL)
        {
            fprintf(stderr, "Invalid BarCode Char:'%c'\n", *barcodeValueStr);
             return NULL;
        }

        int index = pos - code39_chars;
        Code39Char code = code39_table[index];
        
        for (int i = 0; i<BARCODE39_DIGIT; i++)
        {
            bcodeWidth += code.nwpattern[i] == 'W' ? wide:narrow;
        }

        //Add the space.
        bcodeWidth +=narrow;

        barcodeValueStr++;
    }//End while.

    //Add the quite space narrow * 10 
    bcodeWidth = bcodeWidth + (narrow * 10  * 2);
    
    //Create response.
    barcode39Data* imageResult = malloc(sizeof(barcode39Data));
    imageResult->result = 0;
    if (imageResult==NULL)
    {
         fprintf(stderr, "Error on image memory allocation.\n");
         return NULL;
    }

    //Check
    if (opt.mode == Fixed && bcodeWidth > opt.width)
    {
        imageResult->image  = NULL;
        imageResult->width  = 0;
        imageResult->heigth = 0;
        imageResult->result = WidthNotCompliant;
        return imageResult;
    }

    //Draw barcode
    int memoryLength =opt.width * opt.height;
    imageResult->image  = (unsigned char*)malloc(memoryLength);
    memset(imageResult->image, 255, memoryLength);  // WHITE BACKGROUND. -> CREATE DEFINE.

    imageResult->heigth = opt.height;
    imageResult->width  = opt.width;
    imageResult->result = Done;

    //Calculate the half.
    int x = (imageResult->width - bcodeWidth) /2;
    
    barcodeValueStr = barcodeNewValue;

    while (*barcodeValueStr)
    {
         //Return a pointer of first char (*inputString) found in code39_chars.
        const char *pos = strchr(code39_chars, *barcodeValueStr);
        if (pos==NULL)
        {
            fprintf(stderr, "Invalid BarCode Char:'%c'\n", *barcodeValueStr);
            return NULL;
        }

        int index = pos - code39_chars;
        Code39Char code = code39_table[index];
        
        for (int i = 0; i<9; i++)
        {
            int bar_width = code.nwpattern[i] == 'W' ? wide:narrow;

             // x = 0;
            for (int w = 0; w < bar_width; w++) 
            {
                int pxValue  = (i + 1) % 2 ==0 ? 255: 0;

                for (int y = 0; y < bar_height; y++) 
                {
                     imageResult->image[y *  opt.width + x] = pxValue;  
                }

                x++;
            }

            //Draw space betwen char but not last....
            for (int ws = 0; ws < 1; ws++) 
            {
                for (int y = 0; y < bar_height; y++) 
                {
                     imageResult->image[y *  opt.width + x] = 255;  
                }
            }
            x++;

        }

        

        barcodeValueStr++;
    }

    //Load the Font.
    int fontSize;
    unsigned char* fontBuffer = loadFont("../res/RobotoMono-Regular.ttf", &fontSize);
    if (!fontBuffer) {
        printf("Error on loading font.\n");
        return NULL;
    }

    //Get the font offset.
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0))) {
        printf("Error on font initialization.\n");
        free(fontBuffer);
        exit(1);
    }

    //Set the font scale
  
    int fontWidth  = fontHeight /2;
    float scale = stbtt_ScaleForPixelHeight(&font, fontHeight);

    //Reset the pointer.
    barcodeValueStr = barcodeNewValue;
    
    //Find the center.
    int startx = (imageResult->width - (strlen(barcodeValueStr) * fontWidth)) / 2;

    int offsetY = bar_height + fontHeight -5;
    int counter  = 0;

    //Draw the text.
    for (int counter=0; *barcodeValueStr!='\0'; counter++)
    {
        int offsetX = startx + (counter * fontWidth);
        drawCharOnBitmap(imageResult->image, imageResult->width, imageResult->heigth, offsetX, offsetY, &font, *barcodeValueStr, scale);
        barcodeValueStr++;
    }

  
    return imageResult;

}


unsigned char* CreateBarCode (int width, int height, char *value)
{
    if (width<=0 || height<=0)
    {
        fprintf(stderr, "Error: Barcode size not valid. \n");
        return NULL;
    }

    // int width = 200, height = 100;
    unsigned char* image = (unsigned char*)malloc(width * height);
    if (image==NULL)
    {
         fprintf(stderr, "Error on image memory allocation.\n");
         return NULL;
    }

    memset(image, 255, width * height);  //White background.

    return image;

}



void savepng(barcode39Data *barCodeImage, char *fileName)
{
     int res = stbi_write_png(fileName, barCodeImage->width, barCodeImage->heigth, 1, barCodeImage->image, barCodeImage->width);
     if (res!=1)
     {
        perror("Error writing image.");
        exit(1);
     }
}

unsigned char* loadFont(const char* fontPath, int* fontSize) {
    FILE* f = fopen(fontPath, "rb");
    if (!f) {
        printf("Impossibile aprire il font: %s\n", fontPath);
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char* fontBuffer = (unsigned char*)malloc(size);
    fread(fontBuffer, 1, size, f);
    fclose(f);
    
    *fontSize = size;
    return fontBuffer;
}

void drawCharOnBitmap(unsigned char* bitmap, int imageWidth, int imageHeigth,  int x_offset, int y_offset, stbtt_fontinfo* font, char character, float scale) {
    int c_x1, c_y1, c_x2, c_y2;
    int width, height, xoff, yoff;
    unsigned char* charBitmap;

    // Ottieni le coordinate del bounding box per il carattere
    stbtt_GetCodepointBitmapBox(font, character, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

    // Stampa di debug per il bounding box
    printf("Character bounds for '%c': (%d, %d) to (%d, %d)\n", character, c_x1, c_y1, c_x2, c_y2);
    printf("Char width: %d, Char height: %d\n", c_x2 - c_x1, c_y2 - c_y1);

    // Ottieni la bitmap per il carattere
    charBitmap = stbtt_GetCodepointBitmap(font, scale, scale, character, &width, &height, &xoff, &yoff);

    // Stampa di debug per la bitmap
    printf("Bitmap width: %d, height: %d, xoff: %d, yoff: %d\n", width, height, xoff, yoff);

    // Ciclo per disegnare il carattere sulla bitmap
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int pixelValue = charBitmap[j * width + i];

            // Verifica che il valore del pixel sia > 0 per disegnare
            if (pixelValue > 0) {
                int dstX = x_offset + i + xoff;  // Usa l'offset X corretto
                int dstY = y_offset + j + yoff;  // Usa l'offset Y corretto

                // Controlla se il pixel � dentro i limiti della bitmap
                if (dstX >= 0 && dstX < imageWidth && dstY >= 0 && dstY < imageWidth) {
                    bitmap[dstY * imageWidth + dstX] = 0;  // Imposta il colore del testo (bianco)
                }
            }
        }
    }

    // Libera la bitmap del carattere
    stbtt_FreeBitmap(charBitmap, NULL);
}

