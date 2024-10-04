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

void draw_text(unsigned char* image, int width, int height, const char* text);

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

    draw_text(imageResult->image,imageResult->width, imageResult->heigth, "ciaone");

    printf("|%s|\n", newString);

    return imageResult;

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

// void draw_barcode(unsigned char* image, int width, int height, char* value)
// {
//     if (image==NULL)
//     {
//         perror("Invalid BarCode char.");
//         exit(1);
//     }


//     int x = 30;
//     int bar_height = height - 20;       //Space for the text.
//     while (*value)
//     {
      
//          //Return a pointer of first char (*inputString) found in code39_chars.
//         const char *pos = strchr(code39_chars, *value);
//         if (pos==NULL)
//         {
//              perror("Invalid BarCode char.");
//              exit(1);
//         }
//         int index = pos - code39_chars;
//         Code39Char code = code39_table[index];

//        for (int i = 0; i<9; i++)
//        {
//             int bar_width = code.nwpattern[i] == 'W' ? 3:1;
            
//             // x = 0;
//             for (int w = 0; w < bar_width; w++) 
//             {

//                 //int pxValue = (code.pattern[i] == '1') ? 0 : 255;       // 0 per nero, 255 per bianco
//                 //char b = (code.pattern[i] == '1') ? 'B' : 'W';          
//                 //printf("%c",b);

//                 int pxValue  = (i + 1) % 2 ==0 ? 255: 0;

//                 for (int y = 0; y < bar_height; y++) 
//                 {
//                     image[y * width + x] = pxValue;  
//                 }

//                 x++;
//             }

//             //Draw space betweh char but not last....
//             for (int ws = 0; ws < 1; ws++) 
//             {
//                 for (int y = 0; y < bar_height; y++) 
//                 {
//                     image[y * width + x] = 255;  
//                 }
//             }
//             x++;
            
//        }
//        te++;

//     }
    
//}





// Funzione per disegnare il testo sotto il barcode
void draw_text(unsigned char* image, int width, int height, const char* text) {
    unsigned char ttf_buffer[1<<20];
    stbtt_fontinfo font;
    
    FILE* font_file = fopen("../res/arial.ttf", "rb");
    if (font_file == NULL) {
        perror("Font not found.");
        exit(1);
    }
    fread(ttf_buffer, 1, 1<<20, font_file);
    fclose(font_file);

    if (!stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0))) {
        printf("Failed to initialize font\n");
        exit(1);
    }

    float scale = stbtt_ScaleForPixelHeight(&font, 64);  // Aumenta la scala per caratteri più grandi
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    printf("Ascent:%d Descent:%d LineGap%d\n", ascent, descent,lineGap);
    ascent *= scale;
    int x = 10, y = height - 30;  // Posizionamento del testo

    for (int i = 0; text[i]; i++) {
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font, text[i], &advance, &lsb);
        int c_x1, c_y1, c_x2, c_y2;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, text[i], &c_x1, &c_y1, &c_x2, &c_y2);

        printf("Char: %c, c_x1: %d, c_y1: %d, c_x2: %d, c_y2: %d\n", text[i], c_x1, c_y1, c_x2, c_y2);
        
        if (bitmap == NULL || c_x2 <= c_x1 || c_y2 <= c_y1) {
            printf("Bitmap generation failed or bounding box invalid for character: %c\n", text[i]);
            continue;
        }

        for (int cy = 0; cy < (c_y2 - c_y1); cy++) {
            for (int cx = 0; cx < (c_x2 - c_x1); cx++) {
                int px = x + cx;
                int py = y + cy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    image[py * width + px] = bitmap[cy * (c_x2 - c_x1) + cx] < 128 ? 255 : 0;
                }
            }
        }
        x += advance * scale;
        stbtt_FreeBitmap(bitmap, NULL);
    }
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

    //char decodeValue[100] = "";

    //Decode(value, decodeValue);

    //draw_barcode(image, width, height, value);

    // Aggiungi il testo in chiaro sotto il barcode
    //draw_text(image, width, height, originalString);
    //Salva come PNG
    
    //stbi_write_png("barcode.png", width, height, 1, image, width);
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