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
    char character;              // Carattere del Code39
    const char *nwpattern;       // Pattern binario
    int checkSum;               // Checksum.
} Code39Char;



static const Code39Char code39_table_const[] = {
    {'0', "NNNWWNWNN", 0},  // Carattere '0': Pattern e posizioni di barre/spazi larghi
    {'1', "WNNWNNNNW", 1},  // Carattere '1'   {'1', "100100001", "WNNWNNNNW", 1}
    {'2', "NNWWNNNNW", 2},  // Carattere '2'
    {'3', "WNWWNNNNN", 3},  // Carattere '3'
    {'4', "NNNWWNNNW", 4},  // Carattere '4'
    {'5', "WNNWWNNNN", 5},  // Carattere '5'
    {'6', "NNWWWNNNN", 6},  // Carattere '6'
    {'7', "NNNWNNWNW", 7},  // Carattere '7'
    {'8', "WNNWNNWNN", 8},  // Carattere '8'
    {'9', "NNWWNNWNN", 9},  // Carattere '9'
    {'A', "WNNNNWNNW", 10},  // Carattere 'A'  {'A', "100001001", "WNNNNWNNW", 10},
    {'B', "NNWNNWNNW", 11},  // Carattere 'B'
    {'C', "WNWNNWNNN", 12},  // Carattere 'C'
    {'D', "NNNNWWNNW", 13},  // Carattere 'D'
    {'E', "WNNNWWNNN", 14},  // Carattere 'E'
    {'F', "NNWNWWNNN", 15},  // Carattere 'F'
    {'G', "NNNNNWWNW", 16},  // Carattere 'G'
    {'H', "WNNNNWWNN", 17},  // Carattere 'H'
    {'I', "NNWNNWWNN", 18},  // Carattere 'I'
    {'J', "NNNNWWWNN", 19},  // Carattere 'J'
    {'K', "WNNNNNNWW", 20},  // Carattere 'K'
    {'L', "NNWNNNNWW", 21},  // Carattere 'L'
    {'M', "WNWNNNNWN", 22},  // Carattere 'M'
    {'N', "NNNNWNNWW", 23},  // Carattere 'N'
    {'O', "WNNNWNNWN", 24},  // Carattere 'O'
    {'P', "NNWNWNNWN", 25},  // Carattere 'P'
    {'Q', "NNNNNNWWW", 26},  // Carattere 'Q'
    {'R', "WNNNNNWWN", 27},  // Carattere 'R'
    {'S', "NNWNNNWWN", 28},  // Carattere 'S'
    {'T', "NNNNWNWWN", 29},  // Carattere 'T'
    {'U', "WWNNNNNNW", 30},  // Carattere 'U'
    {'V', "NWWNNNNNW", 31},  // Carattere 'V'
    {'W', "WWWNNNNNN", 32},  // Carattere 'W'
    {'X', "NWNNWNNNW", 33},  // Carattere 'X'
    {'Y', "WWNNWNNNN", 34},  // Carattere 'Y'
    {'Z', "NWWNWNNNN", 35},  // Carattere 'Z'
    {'-', "NWNNNNWNW", 36},  // Carattere '-'
    {'.', "WWNNNNWNN", 37},  // Carattere '.'
    {' ', "NWWNNNWNN", 38},  // Carattere ' '
    {'*', "NWNNWNWNN", -1},  // Carattere '*'
    {'$', "NWNWNWNNN", 39},  // Carattere '$'
    {'/', "NWNWNNNWN", 40},  // Carattere '/'
    {'+', "NWNNNWNWN", 41},  // Carattere '+'
    {'%', "NNNWNWNWN", 42}   // Carattere '%'
};





static const Code39Char *code39_table = code39_table_const;

static const char code39_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *$/+%";

void drawCharOnBitmap(unsigned char* bitmap, int width, int heigth, int x_offset, int y_offset, stbtt_fontinfo* font, char character, float scale);

unsigned char* loadFont(const char* fontPath, int* fontSize);

barcode39Data* calculate(barcodex39opt opt, char *value)
{
    if (value == NULL)
    {
        perror("Cannot set a char null pointer.");
        exit(1);
    }

    const size_t value_len = strlen(value);
    char* newString = malloc(sizeof(char) * (value_len + 2));
    if (newString==NULL)
    {
         fprintf(stderr, "Error on string memory allocation.\n");
         return NULL;
    }

    strcpy(newString, "*");    // Start character (*)    
    strcat(newString, value);
    strcat(newString, "*");    // end character (*)  

    //Barcode line width.
    int narrow = opt.narrowLineWidth;
    int wide = narrow * 3;
    

    int bcodeWidth = 0;
    int bar_height = opt.height - 20;

    char *char1= newString;     //Rename char1! 

    //Cycle the string
    while (*char1)
    {
        //Return a pointer of first char (*inputString) found in code39_chars.
        const char *pos = strchr(code39_chars, *char1);
        if (pos==NULL)
        {
            fprintf(stderr, "Invalid BarCode Char:'%c'\n", *char1);
            exit(1);
        }

        int index = pos - code39_chars;
        Code39Char code = code39_table[index];
        
        for (int i = 0; i<BARCODE39_DIGIT; i++)
        {
            bcodeWidth += code.nwpattern[i] == 'W' ? wide:narrow;
        }

        //Add the space.
        bcodeWidth +=narrow;

        char1++;
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
    
    char1 = newString;

    while (*char1)
    {
         //Return a pointer of first char (*inputString) found in code39_chars.
        const char *pos = strchr(code39_chars, *char1);
        if (pos==NULL)
        {
            fprintf(stderr, "Invalid BarCode Char:'%c'\n", *char1);
            exit(1);
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

        

        char1++;
    }

    //Load the Font.
    int fontSize;
    unsigned char* fontBuffer = loadFont("../res/RobotoMono-Regular.ttf", &fontSize);
    if (!fontBuffer) {
        exit(1);
    }

    //Get the font offset.
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0))) {
        printf("Errore nell'inizializzazione del font\n");
        free(fontBuffer);
        exit(1);
    }

    //Set the font sca.e
    float scale = stbtt_ScaleForPixelHeight(&font, 28);

    //Rset the pointer.
    char1 = newString;
    int startx = 10;
    int offsetX = 0;
    int counter  = 0;
    while(*char1)
    {

        int value = startx + (counter * 14);
        drawCharOnBitmap(imageResult->image, imageResult->width, imageResult->heigth, value, 70, &font, *char1, scale);
        char1++;
        counter++;
    }
  

    //draw_text(imageResult->image,imageResult->width, imageResult->heigth, "ciaone");

    printf("|%s|\n", newString);

    return imageResult;

}




bool IsValidString(char * barCodeValue)
{
    if (barCodeValue == NULL)
    {
        perror("Cannot set a char null pointer.");
        exit(1);
    }

    int length = strlen(barCodeValue);
    if (length>43)
    {
        return false;
    }

    printf("Valore in Ingresso: %s - Len: %d\n", barCodeValue, length);

    for (int si=0; si<length;si++)
    {
        char currChar = barCodeValue[si];

        int isAspace  = isspace(currChar) && currChar==' ';
        int isSpecial = currChar == '-' || currChar =='.' || currChar =='$' || currChar=='/' || currChar =='+' || currChar=='%';
        

        if (!isalnum(currChar) &&  islower(currChar)  && !isAspace & !isSpecial)
        {
            return false;
        }

        
    }
    return true;
}

void Decode(char *inputString, char *outputString)
{
    //BarCode39 init char *
    if (inputString == NULL)
    {
        perror("CREPA");
        exit(1);
    }

    strcpy(outputString, "010010100"); // Start character (*)

    while(*inputString)
    {
        //Return a pointer of first char (*inputString) found in code39_chars.
        const char *pos = strchr(code39_chars, *inputString);
        
        if (pos!=NULL)
        {
            int index = pos - code39_chars;
            strcat(outputString, &code39_table[index].character);
        }

        inputString++;
    }

    strcat(outputString, "010010100"); // Stop character (*)

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

    memset(image, 255, width * height);  // Fondo bianco

    return image;

}

void WriteBarCode(unsigned char *image, char *fileName, int width, int height)
{
     int res = stbi_write_png(fileName, width, height, 1, image, width);
     if (res!=1)
     {
        perror("Error writing image.");
        exit(1);
     }
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

