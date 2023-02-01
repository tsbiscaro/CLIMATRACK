/*
@
@ Essa funcao remove clusters menores que os clusters de tamanho minimo
@ indicado no arquivo de entrada. Apos isso, calcula parametros como
@ valores medios, maximos/minimos, centro do sistema, tamanho, etc.
@
*/


#include "trackdef.h"
int calcula_parametros (short int *dados, unsigned int *in,
                        info_sistema *clusters, unsigned int *max_cluster,
                        params *parametros)
   {
   int i = 0, j = 0, indice_cluster = 0;
   int indice_contagem = 0;
   unsigned int *out_tmp;
   unsigned int totpix_cluster[MAX_CLUSTER];
   unsigned int pos_x, pos_y;
   double valor = 0;
   double valor_db = 0;
   int max_valor = -3200, min_valor = 35000;
   unsigned short int nx = 0, ny = 0;
   unsigned short int tamanho = 0;
   char msg[MAX_TEXTO];
   int over_t3 = 0;
   
   
   nx = parametros->nx;
   ny = parametros->ny;
   tamanho = parametros->tamanho_minimo;
   
   out_tmp = (unsigned int*) calloc(nx*ny, sizeof(unsigned int));
   if (NULL == out_tmp)
      {
      erro("Memoria insuficiente para execucao do programa", __FILE__, __LINE__);
      return TRACK_ERR;
      }
   
   memset(out_tmp, UNDEF_CLUSTER, sizeof(out_tmp));
   memset(totpix_cluster, 0, sizeof(totpix_cluster));
   memset(clusters, 0, sizeof(info_sistema)*MAX_CLUSTER);

   *max_cluster = 0;
   
   /*Conta qtos clusters existem e o total de pixel por cluster*/
   for(i = 0; i < nx*ny; i++)
      {
      totpix_cluster[in[i]]++;
      
      if (in[i] > *max_cluster)
         {
         *max_cluster = in[i];
         }
      }
   indice_contagem = UNDEF_CLUSTER;
   /*elimina os cluster de tamanho menor que o limiar*/
   for(indice_cluster = UNDEF_CLUSTER + 1; indice_cluster < *max_cluster + 1; indice_cluster++)
      {

      /*
       Se o tamanho for valido, numera a matriz de saida
       Calcula algumas estatisticas (posicao do centro, dBZ/tb medio, etc)
      */
      valor = 0;
      valor_db = 0;
      max_valor = -3200;
      min_valor = 35000;
      over_t3 = 0;
      if (totpix_cluster[indice_cluster] > tamanho)
         {
         indice_contagem++;
         pos_x = 0;
         pos_y = 0;
         for(i = 0; i < nx; i++)
            {
            for(j = 0; j < ny; j++)
               {
               if (in[i + nx*j] == indice_cluster)
                  {
                  if ((i == 91) && (j == 121))
                     {
                     over_t3 = 1;
                     }
                  /*soma as posicoes para calcular o centro depois*/
                  pos_x += i;
                  pos_y += j;
                  /*associa o novo numero para o cluster*/
                  out_tmp[i + nx*j] = indice_contagem;
                  valor += (double) dados[i + nx*j]/100;
                  valor_db += pow(10, ((double) dados[i + nx*j]/100)/10);
                  if (dados[i + nx*j] < min_valor) min_valor = dados[i + nx*j];
                  if (dados[i + nx*j] > max_valor) max_valor = dados[i + nx*j];
                  }
               }
            }
         clusters[indice_contagem].over_t3 = over_t3;
         clusters[indice_contagem].total_pix = totpix_cluster[indice_cluster];
         clusters[indice_contagem].x_centro = pos_x/clusters[indice_contagem].total_pix;
         clusters[indice_contagem].y_centro = pos_y/clusters[indice_contagem].total_pix;
         /*se for img de radar, o valor extremo eh o maior valor*/
         if (IMGRADAR == parametros->tipo_imagem)
            {
            clusters[indice_contagem].valor_extremo = ((float) max_valor)/100;
            clusters[indice_contagem].valor_medio =
               valor / clusters[indice_contagem].total_pix;

            clusters[indice_contagem].valor_medio =
               10*log10f(valor_db / clusters[indice_contagem].total_pix);
            }
         else
            {
            clusters[indice_contagem].valor_extremo = ((float) min_valor)/100;
            clusters[indice_contagem].valor_medio =
               valor / clusters[indice_contagem].total_pix;
            }
         }
      }   
   *max_cluster = indice_contagem;
   /*matriz esta reordenada, passa os valores para o vetor de saida*/
   memcpy(in, out_tmp, nx*ny*sizeof(unsigned int));
   free(out_tmp);
   if (indice_contagem > MAX_CLUSTER_CLEAN)
      {
      erro("Numero maximo de clusters excedido", __FILE__, __LINE__);
      return TRACK_ERR;
      }

   return TRACK_OK;
   }

