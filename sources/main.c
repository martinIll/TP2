#include "../simple_bmp/simple_bmp.h"
#include <omp.h>
#include <math.h>

#define TAM 256
#define K_SIZE 41
#define SRC_PATH "simple_bmp/base.bmp"
#define NEW_PATH "prueba.bmp"


int32_t getModule(int32_t x,int32_t y);

uint8_t linearFiltering(uint8_t,float,float);

void   convFiltering(int32_t x,int32_t y,uint16_t**,int32_t);

void kernel_setup (uint16_t **, int16_t);

int32_t getKernelSum(uint16_t **);

void doProccess(int32_t radius,int32_t nThreads, float k, float l);


sbmp_image inputImage,outputImage;



/**
 * @brief en la funcion main se toman los parametros de argv  se abren la imagen y se llama a la funcion doProcess
 *una vez procesada la imagen se guardan los resultados en el path de la imagen de salida y termina la ejecucion
 *@param int32_t argc cantidad de argumentos de argv
 *@param char *[] argv  argumentos de entrada   

 *@returns int32_t

*/

int32_t main(int32_t argc, char* argv[]){
  char filename[TAM];
  int32_t nThreads,radius;
  float l,k;
  // si no estan todos los parametros se termina la ejecucion
  if(argc<5){
    printf("complete los parametros en orden : r l k nThreads \n");
    exit(1);
  }
  //recupero y parseo los parametros
  radius= atoi(argv[1]);
  l=(float)atof(argv[2]);
  k=(float)atof(argv[3]);
  nThreads= atoi(argv[4]);

  //abro la imagen de entrada y de salida
  strcpy(filename,SRC_PATH);
  sbmp_load_bmp(filename,&inputImage);
  sbmp_load_bmp(filename,&outputImage);
  
  //inicia el procesamiento tomando los tiempos antes y despues
  double time1=omp_get_wtime( );
  doProccess(radius,nThreads,k,l);
  double time2=omp_get_wtime( );
 
 //imprimo el tiempo total de la region paralela
  printf("%d %f \n",nThreads,time2-time1);
  
  //guardo el archivo
  strcpy(filename,NEW_PATH);
  sbmp_save_bmp (filename, &outputImage);


  return 0;    
}

/**
 * @brief funcion para el procesamiento de la imagen, busca el centro, inicializa el kernel ,establece el paralelismo,
 * y recorre la imagen luego despues de determinar si el pixel esta dentro del radio indicado lo manda a un filtro o a otro
 *@param int32_t radius radio separador de los filtros ingresado por parametro por el usuario
 *@param int32_t nThreads numero de threads indicado por el usuario se usa para indicarle este valor a openmp
 *@param float k constante k ingresada por el usuario por parametro representa el contraste del filtro lineal
 *@param float l constante ingresada por el usuario por parametro representa el brillo del filtro lineal

 *@returns void

*/

void doProccess(int32_t radius,int32_t nThreads, float k, float l){
  uint16_t **kernel = calloc (K_SIZE, sizeof (int *));
  int32_t centerX,centerY;
  //busco el centro
  centerX=inputImage.info.image_height/2;
  centerY=inputImage.info.image_width/2;

  //inicializacion del kernel y obtencion de la sumatoria de sus valores
  for (int k = 0; k < K_SIZE; k++)
  kernel[k] = calloc (K_SIZE, sizeof (uint16_t));
  kernel_setup (kernel, K_SIZE);
  int32_t kernelSum=getKernelSum(kernel);
  
  omp_set_num_threads(nThreads);
  //se recorre la imagen para clasificarla dentro o fuera del circulo y se procede en consecuencia 
  #pragma omp parallel 
  {
    #pragma omp for schedule(dynamic)
    for(int32_t i=0;i<inputImage.info.image_height-1;i++){
        for(int32_t j=0;j<inputImage.info.image_width-1;j++){           
            if(radius>=getModule(centerX-i,centerY-j)){
              outputImage.data[i][j].red=linearFiltering(inputImage.data[i][j].red,k,l);
              outputImage.data[i][j].green=linearFiltering(inputImage.data[i][j].green,k,l);
              outputImage.data[i][j].blue=linearFiltering(inputImage.data[i][j].blue,k,l);
            }else{
              convFiltering(i,j,kernel,kernelSum);
            }
        }
    }
  }
}


/**
 * @brief filtro lineal recibe uno de los colores de un pixel y le aplica el filtro lineal de la forma p(x,y)*k+l
 *@param uint_8 pixelColor codigo de un color de la imagen
 *@param float l representa el brillo
 *@param float k representa el contraste   
 *@returns uint8_t

*/
uint8_t linearFiltering(uint8_t  pixelColor,float k,float l){
    int32_t result=(int32_t)((pixelColor*k)+l);
    //el resultado debe ser >0 y <255 para evitar saturacion de colores
    if(result>255){
        result=255;
    }else if(result<0){
      result=0;
    }
    return (uint8_t) result;
}

/**
 * @brief obtiene el modulo de un determinado punto
 *@param int32_t x cordenada x del punto 
 *@param int32_t y cordenada y del punto
 *@returns int32_t
*/
int32_t getModule(int32_t x,int32_t y){
    return (int32_t)sqrt(pow(x,2)+pow(y,2));
}

/**
 * @brief realiza el filtrado convolucional de un determinado pixel de la imagen toma como parametro el punto y dependiendo de la dimension del kernel
 * toma valores en ambas direcciones para construir la matriz a convolucionarse si este intervalo sale de los limites de la imagen no se hace el procesamiento
 * una vez hecha la convolucion en los 3 colores se normaliza haciendo el promedio ponderado sobre la sumatoria de los valores del kernel
 *@param int32_t x(fila) cordenada x del pixel
 *@param int32_t y(columna) cordenada y del pixel
 *@param uint16_t** puntero a puntero donde estan guardados los valores del kernel   
 *@param int32_t kernelSum sumatoria de todos los valores del kernel

 *@returns void

*/
void convFiltering(int32_t x,int32_t y,uint16_t** kernel,int32_t kernelSum){
  int32_t resultR,resultG,resultB,s;
  int32_t limit=(K_SIZE-  1)/2;
  resultR=0;
  resultG=0;
  resultB=0;
    
  //si me salgo de los limites de la imagen este pixel no se procesa
  if((x-limit)<0 || (x+limit)>(outputImage.info.image_height-1) || (y-limit)<0 || (y+limit)>(outputImage.info.image_width-1)   ){
      return;
  }
  //realizo la convolucion
  s=0;
  for(int i=x-limit;i<=x+limit;i++){
      int t=0;
      for(int j=y-limit;j<=y+limit;j++){
          resultR+=inputImage.data[i][j].red*kernel[s][t];
          resultG+=inputImage.data[i][j].green*kernel[s][t];
          resultB+=inputImage.data[i][j].blue*kernel[s][t];
          t++;
      }
      s++;
  }
  //guardo los resultados en la imgen de salida
  outputImage.data[x][y].red=(uint8_t)(resultR/kernelSum);
  outputImage.data[x][y].green=(uint8_t)(resultG/kernelSum);
  outputImage.data[x][y].blue=(uint8_t)(resultB/kernelSum);
}

/**
 * @brief inicializacion del kernel construyendo una "matriz" simetrica
 *@param uint16_t **kern puntero a puntero a entero donde se guardaran los valores generados
 *@param int16_t tamaño de la matriz del kernel

 *@returns void

*/

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

}

/**
 * @brief  recorre el kernel proporcionado y suma sus valores
 *@param uint16_t** kernel del que se sumaran los valores   

 *@returns int32_t

*/

int32_t getKernelSum(uint16_t ** kernel){
  int32_t result=0;
  for(int32_t i=0;i<K_SIZE;i++){
    for(int32_t j=0;j<K_SIZE;j++){
      result+=kernel[i][j];
    }
  }
  return result;
}
