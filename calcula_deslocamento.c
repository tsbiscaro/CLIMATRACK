#include "trackdef.h"

void calcula_deslocamento(info_sistema *cluster1, info_sistema *cluster2,
                          params *parametros, unsigned int numcluster)
   {
   int cl = 0, i = 0;
   float dt = 0, dx = 0, dy = 0;
   float pos_x = 0, pos_y = 0;
   float pos_x_split[MAX_CLUSTER_CLEAN];
   float pos_y_split[MAX_CLUSTER_CLEAN];
   float dx_pond[MAX_CLUSTER_CLEAN];
   float dy_pond[MAX_CLUSTER_CLEAN];
   unsigned int total_split[MAX_CLUSTER_CLEAN];
   long int tamanho_total[MAX_CLUSTER_CLEAN];
   long int tamanho_origem = 0;


   /*zera as matrizes*/
   for (cl = 0; cl < MAX_CLUSTER_CLEAN; cl++)
      {
      pos_x_split[cl] = 0;
      pos_y_split[cl] = 0;
      dx_pond[cl] = 0;
      dy_pond[cl] = 0;
      }
   
   memset(total_split, 0, sizeof(total_split));
   memset(tamanho_total, 0, sizeof(tamanho_total));
   
   for (cl = UNDEF_CLUSTER + 1; cl < numcluster + 1; cl++)
      {
      switch (cluster2[cl].estado_separacao)
         {
         case TIPO_SISTEMA_NOVO:
            {
            cluster2[cl].velocidade = 0;
            cluster2[cl].direcao = 0;
            break;
            }
         case TIPO_SISTEMA_CONTINUACAO:
         case TIPO_SISTEMA_SPLIT:
         case TIPO_SISTEMA_MERGE:
            {
            /*no caso de merge temos que fazer uma media das posicoes
            dos centros dos outros sistemas que deram origem a esse*/
            i = 0;
            dx = 0;
            dy = 0;
            pos_x = 0;
            pos_y = 0;
            tamanho_origem = 0;
            
            while ((0 != cluster2[cl].ind_img_ant[i]) && (i < MAX_MERGE))
               {
               pos_x += cluster1[cluster2[cl].ind_img_ant[i]].x_centro *
                  cluster1[cluster2[cl].ind_img_ant[i]].total_pix;
               pos_y += cluster1[cluster2[cl].ind_img_ant[i]].y_centro *
                  cluster1[cluster2[cl].ind_img_ant[i]].total_pix;   
               tamanho_origem +=
                  cluster1[cluster2[cl].ind_img_ant[i]].total_pix;
               
               if (TIPO_SISTEMA_SPLIT == cluster2[cl].estado_separacao)
                  {
                  /*soma os deslocamentos para fazer a media em caso de split, pondera
                  pelo tamanho do cluster*/
                  pos_x_split[cluster2[cl].ind_img_ant[0]] +=
                     cluster2[cl].x_centro*cluster2[cl].total_pix;
                  pos_y_split[cluster2[cl].ind_img_ant[0]] +=
                     cluster2[cl].y_centro*cluster2[cl].total_pix;
                  tamanho_total[cluster2[cl].ind_img_ant[0]] +=
                     cluster2[cl].total_pix;
                  /*fim do tratamento do split*/
                  }
               /*check if this cluster was over t3 in the past image*/

#if 0               
               if (0 == cluster2[cl].over_t3)
                  {
                  cluster2[cl].over_t3 = cluster1[cluster2[cl].ind_img_ant[i]].over_t3;
                  }
#endif
               
               i++;
               }

            dx = cluster2[cl].x_centro - (pos_x/tamanho_origem);
            dy = cluster2[cl].y_centro - (pos_y/tamanho_origem);
            
            direcao_velocidade(cluster2, cl, parametros, dx, dy);
            break;
            }
         default:
            break;
         }
      }


   /*no caso de split temos que varrer o loop de novo para calcular as
   medias de deslocamento*/

   for (cl = UNDEF_CLUSTER + 1; cl < numcluster + 1; cl++)
      {
      if (TIPO_SISTEMA_SPLIT != cluster2[cl].estado_separacao)
         {
         /*so prossegue se for split*/
         continue;
         }
      
      dx_pond[cluster2[cl].ind_img_ant[0]] =
         pos_x_split[cluster2[cl].ind_img_ant[0]] /
         tamanho_total[cluster2[cl].ind_img_ant[0]];
      
      dy_pond[cluster2[cl].ind_img_ant[0]] =
         pos_y_split[cluster2[cl].ind_img_ant[0]] /
         tamanho_total[cluster2[cl].ind_img_ant[0]];

//      printf("%d %d %f %f\n", cl, cluster2[cl].ind_img_ant[0],dy_pond[cluster2[cl].ind_img_ant[0]], dx_pond[cluster2[cl].ind_img_ant[0]]);
      
      
      dx = cluster1[cluster2[cl].ind_img_ant[0]].x_centro -
         dx_pond[cluster2[cl].ind_img_ant[0]];

      dy = cluster1[cluster2[cl].ind_img_ant[0]].y_centro -
         dy_pond[cluster2[cl].ind_img_ant[0]];
      
      direcao_velocidade(cluster2, cl, parametros, dx, dy);
      }
   
   return;
   }


/*
@
@ calcula a direcao e velocidade dos sistemas
@
*/
void direcao_velocidade(info_sistema *cluster2, int cl,
                        params *parametros, float dx, float dy)
   {
   float ang = 0;

   
   if ((0 == dx) && (0 == dy))
      {
      /*sistema esta parado*/
      cluster2[cl].velocidade = 0;
      cluster2[cl].direcao = 0;
      return;
      }
   if (0 == dx)
      {
      /*sistema esta se movimentando na vertical*/
      cluster2[cl].velocidade = dy/parametros->intervalo_atual;
      if (cluster2[cl].velocidade < 0)
         {
         dy = -dy;
         cluster2[cl].velocidade = -cluster2[cl].velocidade;
         }
      if (dy > 0)
         {
         cluster2[cl].direcao = 0;
         }
      else
         {
         cluster2[cl].direcao = 180;
         }
      }
   else
      {
      if (0 == dy)
         {
         /*sistema esta se movimentando na horizontal*/
         cluster2[cl].velocidade = dx/parametros->intervalo_atual;
         if (cluster2[cl].velocidade < 0)
            {
            dx = -dx;
            cluster2[cl].velocidade = -cluster2[cl].velocidade;
            }
         if (dx > 0)
            {
            cluster2[cl].direcao = 90;
            }
         else
            {
            cluster2[cl].direcao = 270;
            }
         }
      else
         {
         /*
         calcula o angulo e o quadrante correto, ja que a funcao
         atan retorna valores entre [-pi/2, +pi/2]
         
         O angulo esta orientado da seguinte maneira:
         
                         |90
                         |
                         |
                         |
                180---------------0
                         |
                         |
                         |
                         |270
         
         Para visualizacao, colocarmos na saida o valor
         orientado N-S-E-W
         */
         ang = (180*atanf(dy/dx)/M_PI);
         if ((dx < 0) && (dy > 0))
            {
            ang = ang + 180;
            }
         if ((dx < 0) && (dy < 0))
            {
            ang = ang + 180;
            }
         if ((dx > 0) && (dy < 0))
            {
            ang = ang + 360;
            }
         cluster2[cl].direcao = ang;
         cluster2[cl].velocidade = sqrt(dx*dx + dy*dy)/parametros->intervalo_atual;
         }
      }
   return;
   }
