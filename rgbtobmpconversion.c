#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include<inttypes.h>
#include <string.h>

//DEFINING MACROS FOR BITMAP HEADER
#define RESERVED_1 0
#define RESERVED_2 0
#define OFFSET_VALUE 54

//DEFINING MACROS FOR DIB HEADER
#define BMP_SIZE 40
#define BMP_WIDTH 640
#define BMP_HEIGHT -480
#define COLOR_PLANES 1
#define BITS_PER_PIXELS 24
#define COMPRESSION_METHOD 0
#define H_RESOLUTION 0
#define V_RESOLUTION 0
#define NO_OF_COLORS 0
#define NO_OF_IMP_COLORS 0
#define HEADER_FIELD_SIZE 2
#define BUFFER_SIZE 921600

//GLOBAL DECLARATION FOR HEADER FIELD
char headerField[HEADER_FIELD_SIZE];

//STRUCTURE FOR BITMAP HEADER
typedef struct bitmap_Header { 
  uint32_t fileSize;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t offset;
} BITMAP_HEADER;

//STRUCTURE FOR DIB HEADER
typedef struct dib_Header {
  uint32_t bmpSize;
  uint32_t bmpWidth;
  uint32_t bmpHeight;
  uint16_t colorPlanes;
  uint16_t bitsPerPixels;
  uint32_t compression;
  uint32_t imageSize;
  uint32_t hResolution;
  uint32_t vResolution;
  uint32_t noOfColors;
  uint32_t noOfImpColors;
} DIB_HEADER;

//ASSIGNING THE DATA FOR BITMAP HEADER
void getBmpHeader(BITMAP_HEADER *bitmapHeader, int filesize)
{
 bitmapHeader->fileSize  = filesize+OFFSET_VALUE;
 bitmapHeader->reserved1 = RESERVED_1;
 bitmapHeader->reserved2 = RESERVED_2;
 bitmapHeader->offset    = OFFSET_VALUE;
}

//ASSIGNING THE DATA FOR DIB HEADER
void getDibHeader(DIB_HEADER *dibHeader, int filesize)
{ 
 dibHeader->bmpSize       = BMP_SIZE;
 dibHeader->bmpWidth      = BMP_WIDTH;
 dibHeader->bmpHeight     = BMP_HEIGHT;
 dibHeader->colorPlanes   = COLOR_PLANES;
 dibHeader->bitsPerPixels = BITS_PER_PIXELS;
 dibHeader->compression   = COMPRESSION_METHOD;
 dibHeader->imageSize     = filesize;
 dibHeader->hResolution   = H_RESOLUTION;
 dibHeader->vResolution   = V_RESOLUTION;
 dibHeader->noOfColors    = NO_OF_COLORS;
 dibHeader->noOfImpColors = NO_OF_IMP_COLORS; 
}

//PRINTING THE DATA OF BITMAP HEADER
void printBmpHeader(BITMAP_HEADER bitmapHeader)
{
   printf("\nFILE SIZE   : %lu\n",bitmapHeader.fileSize);
   printf("RESERVED 1  : %lu\n",bitmapHeader.reserved1);
   printf("RESERVED 2  : %lu\n",bitmapHeader.reserved2);
   printf("OFFSET      : %lu\n",bitmapHeader.offset);
}

//PRINTING THE DATA OF DIB HEADER
void printDibHeader(DIB_HEADER dibHeader) {
   printf("\nSIZE OF DIB HEADER          : %lu\n", dibHeader.bmpSize);
   printf("WIDTH                       : %lu\n", dibHeader.bmpWidth);
   printf("HEIGHT                      : %lu\n", dibHeader.bmpHeight);
   printf("COLOR PLANES                : %lu\n", dibHeader.colorPlanes);
   printf("BITS PER PIXELS             : %lu\n", dibHeader.bitsPerPixels);
   printf("COMPRESSION                 : %lu\n", dibHeader.compression);
   printf("IMAGE SIZE                  : %lu\n", dibHeader.imageSize);
   printf("PIXEL RESOLUTIONS           : %lu-%lu\n",dibHeader.hResolution, dibHeader.vResolution);
   printf("NUMBER OF COLORS IN PALETTE : %lu\n", dibHeader.noOfColors);
   printf("NUMBER OF IMPORTANT COLORS  : %lu\n", dibHeader.noOfImpColors);
}

void function()
{
 FILE *filePtr1 = NULL;
 int filesize;
 unsigned char *buffer = (unsigned int*)malloc(BUFFER_SIZE);
 //OPEN WITH READ BINARY MODE AND VALIDATING THE FILE
 filePtr1 = fopen("rgb.raw","rb");
 if(filePtr1 == NULL)
 {
  printf("This %s file could not open....","rgb.raw");
 }

 //FINDING THE SIZE OF THE FILE USING FSEEK FUNCTION
 fseek(filePtr1,0,SEEK_END);
 filesize = ftell(filePtr1);

 //ASSIGNING THE POINTER BACK TO THE BEGINING 
 fseek(filePtr1,0,SEEK_SET);

 //VALIDATING FREAD FUNCTION
 int total= fread(buffer,filesize,1,filePtr1);
 printf("fread size: %d",total);
 BITMAP_HEADER bitmapHeader;
 DIB_HEADER dibHeader;

 //CALLING THE FUNCTIONS
 getBmpHeader(&bitmapHeader,filesize);
 getDibHeader(&dibHeader,filesize);
 FILE *filePtr2 = NULL;
 //OPEN WITH APPEND MODE AND VALIDATING THE FILE
 filePtr2 = fopen("bmpFile.bmp","a");
 if(filePtr2 == NULL)
 {
  printf("This %s file could not open....","bmpFile.bmp");
 }

 //ASSIGNING HEX DEC VALUES TO RESPECTIVE HEADER FIELDS
 headerField[0]=0x42;
 headerField[1]=0x4D;

 //PRINTING THE HEADER VALUES FOR OUR VERIFICATION        
 printf("\nHEADER FIELD  : %c%c",headerField[0],headerField[1]);
 printBmpHeader(bitmapHeader);
 printDibHeader(dibHeader);

 //WRITING THE BITMAP AND DIB HEADER IN A FILE
 fwrite(headerField,HEADER_FIELD_SIZE,1,filePtr2);
 fwrite(&bitmapHeader,sizeof(BITMAP_HEADER),1,filePtr2);
 fwrite(&dibHeader,sizeof(DIB_HEADER),1,filePtr2);

 //APPENDING THE CONTENT OF RAW FILE TO THE HEADER DATA
 fwrite(buffer,filesize,1,filePtr2);

 //CLOSING THE FILE
 fclose(filePtr1);
 fclose(filePtr2);
 
 //FREE THE MEMORY FOR BUFFER
 if(buffer == NULL)
 {
  free(buffer);
 }
}

//DRIVER FUNCTION
int main()
{
 function();
 return 0;
}

