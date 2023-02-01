/*
@ As duas funcoes abaixo são identicas, exceto pela linha
@ if (vizinho[k] <= limiar) (para Tb)
@ if (vizinho[k] >= limiar) (para dBZ)
@ 
@ Nao foi utilizado um IF para deixar o codigo mais rapido. Como esse IF ficaria
@ dentro de um loop chamado muitas vezes, priorizou-se a velocidade do codigo em
@ detrimento da fluencia do mesmo
*/


/*
@ Funcao para encontrar pixeis adjacentes com refletividade maior que um limiar dado.
@ 
@ Essa funcao chama ela mesma (funcao recursiva)
@ 
*/

#include "trackdef.h"
void numera_cluster_radar(short int *in, unsigned int *out,
                          int i, int j, unsigned int num_cluster,
                          params *parametros)
   {
   short int vizinho[8];
   unsigned short int x[8], y[8];
   int k = 0;
   unsigned int nx = 0, ny = 0;

   nx = parametros->nx;
   ny = parametros->ny;
   

   /*se ja foi marcado como parte de um cluster ou
   se nao eh um valor valido, sai*/
   if ((out[i + nx*j] == num_cluster) ||
       (in[i + nx*j] > UNDEF_FISICO))
      {   
      return;
      }

   out[i + nx*j] = num_cluster;
   
   if ((i + 1) < nx && (j + 1) < ny &&
       (i - 1) >= 0 && (j - 1) >= 0)
      {
      vizinho[0] = in[i + 1 + nx*(j + 1)];
      vizinho[1] = in[i + 1 + nx*(j + 0)];
      vizinho[2] = in[i + 1 + nx*(j - 1)];
      vizinho[3] = in[i + 0 + nx*(j - 1)];
      vizinho[4] = in[i - 1 + nx*(j - 1)];
      vizinho[5] = in[i - 1 + nx*(j + 0)];
      vizinho[6] = in[i - 1 + nx*(j + 1)];
      vizinho[7] = in[i + 0 + nx*(j + 1)];

      x[0] = i + 1;
      x[1] = i + 1;
      x[2] = i + 1;
      x[3] = i + 0;
      x[4] = i - 1;
      x[5] = i - 1;
      x[6] = i - 1;
      x[7] = i + 0;

      y[0] = j + 1;
      y[1] = j + 0;
      y[2] = j - 1;
      y[3] = j - 1;
      y[4] = j - 1;
      y[5] = j + 0;
      y[6] = j + 1;
      y[7] = j + 1;
      
      for (k = 0; k < 8; k++)
         {
         /*se tem um vizinho com Tb abaixo do limiar,
         marca ele com o numero do cluster*/
         if (vizinho[k] >= parametros->limiar)
            {
            numera_cluster_radar(in, out, x[k], y[k], num_cluster, parametros);
            }
         } 
      }
   return;
   }

/*
@ Funcao para encontrar pixeis adjacentes com temperatura menor que um limiar dado.
@ 
@ Essa funcao chama ela mesma (funcao recursiva)
@ 
@ Parâmetros de entrada:
@ 
@ *in = matriz da imagem (Tb)
@ i, j = indices da posicao atual
@ nx, ny = tamanho da imagem
@ num_cluster = numeracao do cluster (sequencial)
@ limiar = Tb para limiar
*/


void numera_cluster_satelite(short int *in, unsigned int *out,
                             int i, int j, unsigned int num_cluster,
                             params *parametros)
   {
   short int vizinho[8];
   unsigned short int x[8], y[8];
   int k = 0;
   unsigned int nx = 0, ny = 0;

   nx = parametros->nx;
   ny = parametros->ny;

   /*se ja foi marcado como parte de um cluster ou
   se nao eh um valor valido, sai*/
   if ((out[i + nx*j] == num_cluster) ||
       (in[i + nx*j] < UNDEF_FISICO))
      {   
      return;
      }
   
   out[i + nx*j] = num_cluster;
   
   if ((i + 1) < nx && (j + 1) < ny &&
       (i - 1) >= 0 && (j - 1) >= 0)
      {
      vizinho[0] = in[i + 1 + nx*(j + 1)];
      vizinho[1] = in[i + 1 + nx*(j + 0)];
      vizinho[2] = in[i + 1 + nx*(j - 1)];
      vizinho[3] = in[i + 0 + nx*(j - 1)];
      vizinho[4] = in[i - 1 + nx*(j - 1)];
      vizinho[5] = in[i - 1 + nx*(j + 0)];
      vizinho[6] = in[i - 1 + nx*(j + 1)];
      vizinho[7] = in[i + 0 + nx*(j + 1)];

      x[0] = i + 1;
      x[1] = i + 1;
      x[2] = i + 1;
      x[3] = i + 0;
      x[4] = i - 1;
      x[5] = i - 1;
      x[6] = i - 1;
      x[7] = i + 0;

      y[0] = j + 1;
      y[1] = j + 0;
      y[2] = j - 1;
      y[3] = j - 1;
      y[4] = j - 1;
      y[5] = j + 0;
      y[6] = j + 1;
      y[7] = j + 1;
      
      for (k = 0; k < 8; k++)
         {
         /*se tem um vizinho com Tb abaixo do limiar,
         marca ele com o numero do cluster*/
         if (vizinho[k] <= parametros->limiar)
            {
            numera_cluster_satelite(in, out, x[k], y[k], num_cluster, parametros);
            }
         } 
      }
   return;
   }


/*
@
@ Verifica todos os pixeis menores que o limiar e marca a matriz de cluster
@ com UNDEF_CLUSTER + 1 nos pixeis onde o valor eh valido.
@ Apos isso, numera os clusters de acordo com o agrupamento.
@
*/
int numera_cluster(short int *in, unsigned int *out, params *parametros)
   {
   int i = 0, j = 0;
   unsigned int num_cluster = UNDEF_CLUSTER + 1;
   unsigned short int total_vizinho = 0;
   FILE *fp = NULL;
   
   memset(out, UNDEF_CLUSTER, sizeof(unsigned int)*parametros->nx*parametros->ny);

   if (IMGRADAR == parametros->tipo_imagem)
      {
      /*limpa pixeis isolados ou invalidos da imagem de radar*/
      for (i = 0; i < parametros->nx ; i++)
         {
         for (j = 0; j < parametros->ny ; j++)
            {            
            if ((in[i + parametros->nx*j] < parametros->limiar) ||
                (in[i + parametros->nx*j] > UNDEF_FISICO))
               {
               continue;
               }
            total_vizinho = 0;
            if ((in[i + 1 + parametros->nx*(j + 1)] >= parametros->limiar) &&
                (in[i + 1 + parametros->nx*(j + 1)] < UNDEF_FISICO)) total_vizinho++;
            if ((in[i + 1 + parametros->nx*(j + 0)] >= parametros->limiar) &&
                (in[i + 1 + parametros->nx*(j + 0)] < UNDEF_FISICO)) total_vizinho++;
            if ((in[i + 1 + parametros->nx*(j - 1)] >= parametros->limiar) &&
                (in[i + 1 + parametros->nx*(j - 1)] < UNDEF_FISICO)) total_vizinho++;
            if ((in[i + 0 + parametros->nx*(j - 1)] >= parametros->limiar) &&
                (in[i + 0 + parametros->nx*(j - 1)] < UNDEF_FISICO)) total_vizinho++;
            if ((in[i - 1 + parametros->nx*(j - 1)] >= parametros->limiar) &&
                (in[i - 1 + parametros->nx*(j - 1)] < UNDEF_FISICO)) total_vizinho++;
            if ((in[i - 1 + parametros->nx*(j + 0)] >= parametros->limiar) &&
                (in[i - 1 + parametros->nx*(j + 0)] < UNDEF_FISICO)) total_vizinho++;
            if ((in[i - 1 + parametros->nx*(j + 1)] >= parametros->limiar) &&
                (in[i - 1 + parametros->nx*(j + 1)] < UNDEF_FISICO)) total_vizinho++;
            if ((in[i + 0 + parametros->nx*(j + 1)] >= parametros->limiar) &&
                (in[i + 0 + parametros->nx*(j + 1)] < UNDEF_FISICO)) total_vizinho++;
            /*se for um pixel isolado, marca que nao eh um sistema*/
            if (0 == total_vizinho)
               in[i + parametros->nx*j] = parametros->limiar - 1;
            }
         }
      /*para todos os pixeis com dBZ acima do limiar e nao marcados ainda
      com um numero valido de cluster, chama a funcao de cluster*/
      for (i = 1; i < parametros->nx -1; i++)
         {
         for (j = 1; j < parametros->ny -1; j++)
            {
            if ((in[i + parametros->nx*j] >= parametros->limiar) &&
                (out[i + parametros->nx*j] == UNDEF_CLUSTER) &&
                (in[i + parametros->nx*j] < UNDEF_FISICO))
               {
               numera_cluster_radar(in, out, i, j, num_cluster++, parametros);
               if (num_cluster > (MAX_CLUSTER - 1))
                  {
                  erro("Numero maximo de clusters excedido", __FILE__, __LINE__);
                  return TRACK_ERR;
                  }
               }
            }
         }
      }
   else
      {
      /*limpa pixeis isolados da imagem de satelite*/
      for (i = 0; i < parametros->nx ; i++)
         {
         for (j = 0; j < parametros->ny ; j++)
            {
            if ((in[i + parametros->nx*j] > parametros->limiar) ||
                (in[i + parametros->nx*j] < UNDEF_FISICO))
               {
               continue;
               }
            total_vizinho = 0;
            if ((in[i + 1 + parametros->nx*(j + 1)] <= parametros->limiar) &&
                (in[i + 1 + parametros->nx*(j + 1)] > UNDEF_FISICO)) total_vizinho++;
            if ((in[i + 1 + parametros->nx*(j + 0)] <= parametros->limiar) &&
                (in[i + 1 + parametros->nx*(j + 0)] > UNDEF_FISICO)) total_vizinho++;
            if ((in[i + 1 + parametros->nx*(j - 1)] <= parametros->limiar) &&
                (in[i + 1 + parametros->nx*(j - 1)] > UNDEF_FISICO)) total_vizinho++;
            if ((in[i + 0 + parametros->nx*(j - 1)] <= parametros->limiar) &&
                (in[i + 0 + parametros->nx*(j - 1)] > UNDEF_FISICO)) total_vizinho++;
            if ((in[i - 1 + parametros->nx*(j - 1)] <= parametros->limiar) &&
                (in[i - 1 + parametros->nx*(j - 1)] > UNDEF_FISICO)) total_vizinho++;
            if ((in[i - 1 + parametros->nx*(j + 0)] <= parametros->limiar) &&
                (in[i - 1 + parametros->nx*(j + 0)] > UNDEF_FISICO)) total_vizinho++;
            if ((in[i - 1 + parametros->nx*(j + 1)] <= parametros->limiar) &&
                (in[i - 1 + parametros->nx*(j + 1)] > UNDEF_FISICO)) total_vizinho++;
            if ((in[i + 0 + parametros->nx*(j + 1)] <= parametros->limiar) &&
                (in[i + 0 + parametros->nx*(j + 1)] > UNDEF_FISICO)) total_vizinho++;
            /*se for um pixel isolado, marca que nao eh um sistema*/
            if (0 == total_vizinho)
               in[i + parametros->nx*j] = parametros->limiar + 1;
            }
         }
      /*para todos os pixeis com Tb abaixo do limiar e nao marcados ainda
      com um numero valido de cluster, chama a funcao de cluster*/
      for (i = 1; i < parametros->nx -1; i++)
         {
         for (j = 1; j < parametros->ny -1; j++)
            {
            if ((in[i + parametros->nx*j] <= parametros->limiar) &&
                (out[i + parametros->nx*j] == UNDEF_CLUSTER) &&
                (in[i + parametros->nx*j] > UNDEF_FISICO))
               {
               numera_cluster_satelite(in, out, i, j, num_cluster++, parametros);
               if (num_cluster > (MAX_CLUSTER - 1))
                  {
                  erro("Numero maximo de clusters excedido", __FILE__, __LINE__);
                  return TRACK_ERR;
                  }
               }
            }
         }
      }
   return TRACK_OK;
   }
