#include "main.h"
#include <math.h>

sbmp_image image;

int32_t main(int32_t argc, char* argv[]){
  uint16_t **kernel = calloc (K_SIZE, sizeof (int *));
  char filename[TAM];
  uint32_t centerX,centerY,nThreads,radius;
  float l,k;

  if(argc<5){
    printf("complete los parametros en orden : r l k nThreads \n");
    exit(1);
  }

  radius=atoi(argv[1]);
  l=atof(argv[2]);
  k=atof(argv[3]);
  nThreads=atoi(argv[4]);
  
  for (int k = 0; k < K_SIZE; k++)
  kernel[k] = calloc (K_SIZE, sizeof (uint16_t));
  kernel_setup (kernel, K_SIZE);

  strcpy(filename,"simple_bmp/base.bmp");
  sbmp_load_bmp(filename,&image);



  centerX=image.info.image_height/2;
  centerY=image.info.image_width/2;
  int32_t kernelSum=getKernelSum(kernel);
  omp_set_num_threads(nThreads);
  double time1=omp_get_wtime( );
  
  #pragma omp parallel 
  {
    #pragma omp for schedule(runtime)
    for(int32_t i=0;i<image.info.image_height-1;i++){
        for(int32_t j=0;j<image.info.image_width-1;j++){           
            if(radius>=getModule(centerX-i,centerY-j)){
              image.data[i][j].red=linearFiltering(image.data[i][j].red,k,l);
              image.data[i][j].green=linearFiltering(image.data[i][j].green,k,l);
              image.data[i][j].blue=linearFiltering(image.data[i][j].blue,k,l);
            }else{
              convFiltering(i,j,kernel,kernelSum);
            }
        }
    }

  }
  double time2=omp_get_wtime( );

  printf("%d %f \n",nThreads,time2-time1);
  
  strcpy(filename,"simple_bmp/prueba.bmp");

  sbmp_save_bmp (filename, &image);


  return 0;    
}


uint8_t linearFiltering(uint8_t  pixelColor,float k,float l){
    int32_t result=(pixelColor*k)+l;
    if(result>255){
        result=255;
    }
    return (uint8_t) result;
}

int32_t getModule(int32_t x,int32_t y){
    return sqrt(pow(x,2)+pow(y,2));
}


void convFiltering(int32_t x,int32_t y,uint16_t** kernel,int32_t kernelSum){
    int32_t limit=(K_SIZE-  1)/2;
    int32_t resultR,resultG,resultB,s,t;
    resultR=0;
    resultG=0;
    resultB=0;
    

    if((x-limit)<0 || (x+limit)>(image.info.image_height-1) || (y-limit)<0 || (y+limit)>(image.info.image_width-1)   ){
        return;
    }
    s=0;
    for(int i=x-limit;i<=x+limit;i++){
        t=0;
        for(int j=y-limit;j<=y+limit;j++){
            
            // printf(" s%d t%d %d\n",s,t,kernel[s][t]);
            resultR+=image.data[i][j].red*kernel[s][t];
            resultG+=image.data[i][j].green*kernel[s][t];
            resultB+=image.data[i][j].blue*kernel[s][t];
            t++;
        }
        s++;
    }
    // printf("llega a salir %d\n",s);

    image.data[x][y].red=resultR/kernelSum;
    image.data[x][y].green=resultG/kernelSum;
    image.data[x][y].blue=resultB/kernelSum;
}

void kernel_setup (uint16_t **kern, int16_t ksize){
  uint16_t st_val = 1;
  for (int j = 0; j < ksize; j++)
          kern[0][j] = st_val;

  for (int i = 1; i < ksize / 2 + 1; i++)
    {
      for (int j = 0; j < ksize; j++)
        {
          if (j >= i && j < (ksize - i))
            kern[i][j] = (uint16_t) (kern[i - 1][j] + (uint16_t) 1);
          else
            kern[i][j] = kern[i - 1][j];
        }

    }

  for (int i = 1; i < (ksize / 2); i++)
    {
      for (int j = 0; j < ksize; j++)
        {
          kern[i + ksize / 2][j] = kern[ksize / 2 - i][j];
        }
    }

  for (int j = 0; j < ksize; j++)
    kern[ksize-1][j] = st_val;

  // for (int i = 0; i < ksize; i++)
  //   {
  //     for (int j = 0; j < ksize; j++)
  //       {
  //         printf ("%3hu ", kern[i][j]);
  //       }
  //     printf ("\n");
  //   }
}

int32_t getKernelSum(uint16_t ** kernel){
  int32_t result=0;
  for(int32_t i=0;i<K_SIZE;i++){
    for(int32_t j=0;j<K_SIZE;j++){
      result+=kernel[i][j];
    }
  }
  return result;
}
