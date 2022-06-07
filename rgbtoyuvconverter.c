#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main()
{
  FILE *RGB;
  int index, width, height,size;
  unsigned char R,G,B,Y,U,V;
  width=640;
  height=480;
  int *pixels = malloc((width*height*3)*sizeof(int));
  RGB=fopen("rgb.raw","rb");
  if(RGB == NULL)
  {
   printf("RGB FILE OPENED SUCCESSFULLY!!!");
  }
  else 
  {
    printf("RGB FILE OPENING IS FAILED...\n");
  }

  size=fread(pixels, sizeof(int),640*480*3, RGB); 
  printf("Reading high score: %d\n",size);
  FILE *YUV;
  YUV = fopen("yuv.raw","w");
  if(YUV == NULL)
  {
   printf("YUV FILE CREATED SUCCESSFULLY!!!");
  }
  else 
  {
    printf("YUV FILE OPENING IS FAILED...\n");
  }

 for(index=0; index<size; index=index+3)
  {
    R = pixels[index];    //R Value
    G = pixels[index+1];  //G Value
    B = pixels[index+2];  //B Value
    //formula for RGB to YUV conversion
    Y=0.299*R + 0.587*G + 0.114*B;
    U= -0.1687*R - 0.3313*G - 0.5*B + 128;
    V= 0.5*R - 0.4187*G - 0.813*B + 128;
  }
  fwrite(pixels,size,1,YUV);
  
  fclose(YUV);
  fclose(RGB);
}
