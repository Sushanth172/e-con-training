#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main()
{
  int index,index1, width, height,size;
  long int R,G,B,Y,U,V,R1,G1,B1,Y1,U1,V1,TU,TV;
  width=640;
  height=480;
  unsigned char *pixels = (unsigned char*)malloc((width*height*3));
  FILE *RGB;
  RGB=fopen("rgb.raw","rb");
  if (RGB == NULL) 
  {   
   printf("Error! RGB file could not open...\n"); 
   exit(-1); 
  } 
  size=fread(pixels,1,640*480*3, RGB); 
  unsigned char *yuvArray = (unsigned char*)malloc((640*480*2));
 //1.Iterating 2 pixels at a time and calculating corfileSizeponding YUV values

 /*for(index=0, index1=0 ; index<640*3; index=index+6)
  {
   for(index=0, index1=0 ; index<480*3; index1=index1+4)
   { 
       R = pixels[index][index1];
       G = pixels[index][index1+1];
       B = pixels[index][index1+2];
       R1 = pixels[index][index1+3];   
       G1 = pixels[index][index1+4];
       B1 = pixels[index][index1+5];
       //formula for RGB to YUV conversion
       Y =  0.257 * R + 0.504 * G + 0.098 * B +  16;
       U = -0.148 * R - 0.291 * G + 0.439 * B + 128;
       V =  0.439 * R - 0.368 * G - 0.071 * B + 128;

       Y1 =  0.257 * R1 + 0.504 * G1 + 0.098 * B1 +  16;
       U1 = -0.148 * R1 - 0.291 * G1 + 0.439 * B1 + 128;
       V1 =  0.439 * R1 - 0.368 * G1 - 0.071 * B1 + 128;

       //2.Taking the average of U & V of both pixels fileSizepectively
       TU = (U + U1)/2;
       TV = (V + V1)/2;
       //3.Assigning in UYVY format
       yuvArray[index][index1]=TU;    
       yuvArray[index][index1+1]=Y;  
       yuvArray[index][index1+2]=TV;  
       yuvArray[index1+3]=Y1;  */
 for(index=0, index1=0 ; index<640*480*3; index=index+6,index1=index1+4)
   {
      //printf("%d\n",index);
      R = pixels[index];    //R Value
      G = pixels[index+1];  //G Value
      B = pixels[index+2];  //B Value
      R1 = pixels[index+3];   
      G1 = pixels[index+4]; 
      B1 = pixels[index+5];  
      //formula for RGB to YUV conversion
      Y =  0.257 * R + 0.504 * G + 0.098 * B +  16;
      U = -0.148 * R - 0.291 * G + 0.439 * B + 128;
      V =  0.439 * R - 0.368 * G - 0.071 * B + 128;

      Y1 =  0.257 * R1 + 0.504 * G1 + 0.098 * B1 +  16;
      U1 = -0.148 * R1 - 0.291 * G1 + 0.439 * B1 + 128;
      V1 =  0.439 * R1 - 0.368 * G1 - 0.071 * B1 + 128;
   
      //2.Taking the average of U & V of both pixels fileSizepectively
      TU = (U + U1)/2;
      TV = (V + V1)/2;
      //3.Assigning in UYVY format
      yuvArray[index1]=TU;    
      yuvArray[index1+1]=Y;  
      yuvArray[index1+2]=TV;  
      yuvArray[index1+3]=Y1;  
   }
  FILE *YUV;
  YUV = fopen("yuv.raw","wb"); 
  if (YUV == NULL) 
  {   
   printf("Error! YUV file could not open\n"); 
   exit(-1); 
  } 
  //writing the values in the array in YUV file
  fwrite(yuvArray,1,640*480*2,YUV);
  fclose(RGB);
  fclose(YUV);
  
  if(pixels == NULL)
  {
   free(pixels);
  }
  
  if(yuvArray ==NULL)
  {
  free(yuvArray);
  }

  return 0;
  
}
