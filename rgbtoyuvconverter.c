#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main()
{
  int index,index1, width, height,size;
  int R,G,B,Y,U,V;
  width=640;
  height=480;
  int *pixels = malloc((width*height*3)*sizeof(int));
  FILE *RGB;
  RGB=fopen("rgb.raw","rb");
  FILE *YUV;
  YUV = fopen("yuv1.raw","wb");
  if (RGB == NULL || YUV == NULL) 
  {   
   printf("Error! Could not open file\n"); 
   exit(-1); 
  } 
  size=fread(pixels, sizeof(int),640*480*3, RGB); 
  //printf("Reading high score: %d\n",size);
  int yuvArray[640*480*4];
  index=0,index1=0;
 while(index > size)
 {
   for(index; index<size; index=index+3)
   {
      R = pixels[index];    //R Value
      G = pixels[index+1];  //G Value
      B = pixels[index+2];  //B Value
      //formula for RGB to YUV conversion
      Y = 0.299*R + 0.587*G + 0.114*B;
      U = -0.1687*R - 0.3313*G - 0.5*B + 128;
      V = 0.5*R - 0.4187*G - 0.813*B + 128;
      break;
   } 
   for(index1;index1<index+1; index1=index+4)
   {
    yuvArray[index1]=U;
    yuvArray[index1+1]=Y;
    yuvArray[index1+2]=V;
    yuvArray[index1+3]=Y;
    index1=index1+3;
    break;
   } 
 }
  fwrite(yuvArray,sizeof(int),size,YUV);  
  fclose(YUV);
  fclose(RGB);
}
