#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main()
{
  int index,index1, width, height,size;
  int R,G,B,Y,U,V,R1,G1,B1,Y1,U1,V1,TU,TV;
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
  unsigned char *yuvArray = (unsigned char*)malloc((640*480*(3/2)));
 /*1.Iterating 2 pixels at a time and calculating corresponding YUV values
   2.Taking the average of U & V of both pixels respectively
   3.Assigning in UYVY format*/
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
      Y = 0.299*R + 0.587*G + 0.114*B;
      U = -0.1687*R - 0.3313*G - 0.5*B + 128;
      V = 0.5*R - 0.4187*G - 0.813*B + 128;
 
      Y1 = 0.299*R1 + 0.587*G1 + 0.114*B1;
      U1 = -0.1687*R1 - 0.3313*G1 - 0.5*B1 + 128;
      V1 = 0.5*R1 - 0.4187*G1 - 0.813*B1 + 128;
 
      TU = (U + U1)/2;
      TV = (V + V1)/2;
 
      yuvArray[index1]=TU;    
      yuvArray[index1+1]=Y;  
      yuvArray[index1+2]=TV;  
      yuvArray[index1+3]=Y1;   
    } 
     /*if((index%2) != 0)
     { 
      if(count==0)
      {
       yuvArray[index+1]=Y;
       count++;
      }
      else
      {
       yuvArray[index+1]=Y1;
       count--;
      }
     } 
     else if(index==0 && index%4 == 0)
     {
      yuvArray[index]=U;
     } 
     else
     {
      yuvArray[index]=V;
     }*/ 
  FILE *YUV;
  YUV = fopen("yuv2.raw","wb"); 
  if (YUV == NULL) 
  {   
   printf("Error! YUV file could not open\n"); 
   exit(-1); 
  } 
  //writing the values in the array in YUV file
  fwrite(yuvArray,1,640*480*(3/2),YUV);  
  fclose(YUV);
  fclose(RGB);
}
