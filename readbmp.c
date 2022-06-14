#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include<inttypes.h>
#define HEADER_FIELD_SIZE 2

//STRUCTURE FOR BITMAP FILE HEADER
typedef struct bitmapFileHeader { 
  uint32_t fileSize;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t offset;
} BMP_FILE_HEADER;

//STRUCTURE FOR BITMAP INFORMATION HEADER
typedef struct bitmapInfoHeader {
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

//PRINTING THE FEATURES OF BITMAP FILE HEADER
void printFileHeader(BMP_FILE_HEADER fileHeader){
   printf("FILE SIZE  : %lu\n", fileHeader.fileSize);
   printf("RESERVED 1 : %lu\n",fileHeader.reserved1);
   printf("RESERVED 2 : %lu\n",fileHeader.reserved2);
   printf("OFFSET     : %lu\n", fileHeader.offset);
};

//PRINTING THE FEATURES OF BITMAP INFORMATION HEADER
void printInfoHeader(DIB_HEADER infoHeader) {
   printf("\nSIZE OF DIB HEADER          : %lu\n", infoHeader.bmpSize);
   printf("WIDTH                       : %lu\n", infoHeader.bmpWidth);
   printf("HEIGHT                      : %lu\n", infoHeader.bmpHeight);
   printf("COLOR PLANES                : %lu\n", infoHeader.colorPlanes);
   printf("BITS PER PIXELS             : %lu\n", infoHeader.bitsPerPixels);
   printf("COMPRESSION                 : %lu\n", infoHeader.compression);
   printf("IMAGE SIZE                  : %lu\n", infoHeader.imageSize);
   printf("PIXEL RESOLUTIONS           : %lu*%lu\n", infoHeader.hResolution, infoHeader.vResolution);
   printf("NUMBER OF COLORS IN PALETTE : %lu\n", infoHeader.noOfColors);
   printf("NUMBER OF IMPORTANT COLORS  : %lu\n", infoHeader.noOfImpColors);
}
//FUNCTION FOR LOADING BMP FILE
void loadBmpFile()
{
  FILE *filePtr = NULL;
  filePtr = fopen("inputimg.bmp","rb");
  if(filePtr == NULL) 
  {
     printf("Could not open the file");
     fclose(filePtr);
  }
 char headerField[HEADER_FIELD_SIZE]; 
 uint32_t fileSize;
 fread(headerField,sizeof(char),2,filePtr);

 //PRINTING BMP HEADER DATA
 printf("----------FILE HEADER DATA----------\n");
 printf("FILE TYPE  : %c%c\n",headerField[0],headerField[1]);
 //VALIDATING THE FILE TYPE
 if(headerField[0] != 'B' || headerField[1] != 'M')
 {
    printf("The file is not a valid BMP");
 }
 
 BMP_FILE_HEADER fileHeader;
 DIB_HEADER infoHeader;
 fread(&fileHeader,1,sizeof(BMP_FILE_HEADER),filePtr);
 printFileHeader(fileHeader);
 fread(&infoHeader, sizeof(DIB_HEADER), 1, filePtr);

 //PRINTING DIB HEADER DATA 
 printf("----------DIB HEADER DATA-----------");
 printInfoHeader(infoHeader);
 fclose(filePtr);
}

//DRIVER FUNCTION
int main()
{
   loadBmpFile();
   return 0;
}




