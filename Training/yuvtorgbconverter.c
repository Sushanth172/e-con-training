#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//function for restricting the values below 0 & Above 255
int clamp(int value)
{
   if(value < 0 )
   {
   return 0;
   }
   else if(value > 255 )
   {
   return 255;
   } 
   return value;
}

int main()
{
  int index,width,height,index1,value;
  long int R,G,B,Y,U,V,Y1,R1,G1,B1;
  width=640;
  height=480;
  unsigned char *yuvArray = (unsigned char*)malloc((width*height*2));
  FILE *YUV;
  YUV=fopen("uyvy.raw","rb");
  if (YUV == NULL) 
  {   
   printf("Error! YUV file could not open...\n"); 
   exit(-1); 
  } 
  fread(yuvArray,1,640*480*2,YUV); 
  unsigned char *rgbArray = (unsigned char*)malloc((640*480*3));

 //1.Iterating 2 pixels at a time and calculating corresponding YUV values
  for(index=0, index1=0 ; index<640*480*2; index=index+4,index1=index1+6)
   {
      U = yuvArray[index]-128; 
      Y = yuvArray[index+1];             //U Y V Y U Y V Y U Y V  Y  U  Y  V  Y
      V = yuvArray[index+2]-128;         //0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 
      Y1= yuvArray[index+3];

      //formula for YUV to RGB conversion
      R = 1.164*(Y - 16) + 1.596*(V - 128);
      clamp(R);
      G = 1.164*(Y - 16) - 0.813*(V - 128) - 0.391*(U - 128);
      clamp(G);
      B  = 1.164*(Y - 16)+ 2.018*(U - 128);
      clamp(B);
      R1 = 1.164*(Y1 - 16) + 1.596*(V - 128);
      clamp(R1);
      G1 = 1.164*(Y1 - 16) - 0.813*(V - 128) - 0.391*(U - 128);
      clamp(G1);
      B1  = 1.164*(Y1 - 16)+ 2.018*(U - 128);
      clamp(B1);

      /* R = ((298*Y)+(409*V))>>8;
       clamp(R);
       G = ((298*Y)-(100*U)-(208*V))>>8;
       clamp(G);
       B = ((298*Y)+(516*U))>>8;
       clamp(B);
       R1= ((298*Y1)+(409*V))>>8;
       clamp(R1);
       G1 = ((298*Y1)-(100*U)-(208*V))>>8;
       clamp(G1);
       B1 = ((298*Y1)+(516*U))>>8;
       clamp(B1);*/
     
      //Assigning in RGB format
      rgbArray[index1]   = R;    
      rgbArray[index1+1] = G;  
      rgbArray[index1+2] = B; 
      rgbArray[index1+3] = R1;    
      rgbArray[index1+4] = G1;  
      rgbArray[index1+5] = B1; 
   }

  FILE *RGB;
  RGB = fopen("rgbnew.raw","wb"); 
  if (RGB == NULL) 
  {   
   printf("Error! RGB file could not open\n"); 
   exit(-1); 
  } 
  //writing the values in the array in YUV file
  fwrite(rgbArray,1,640*480*3,RGB);
  fclose(YUV);
  fclose(RGB); 
  if(rgbArray == NULL)
  {
   free(rgbArray);
  }
  
  if(yuvArray ==NULL)
  {
  free(yuvArray);
  }
  return 0;
  
}

