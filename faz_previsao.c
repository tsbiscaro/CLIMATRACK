#include "trackdef.h"

int faz_previsao(unsigned int *img_cluster1, unsigned int *img_cluster2,
                 info_sistema *cluster1, info_sistema *cluster2,
                 short int *imagem, params *parametros,
                 unsigned int numcluster1, unsigned int numcluster2,
                 char gera_img_previsao)
   {
   int i = 0, j = 0, cl=0, cl1 = 0, cl2 = 0;
   short int *imagem_sintetica;   
   unsigned int coincidentes1em2[MAX_CLUSTER_CLEAN][MAX_CLUSTER_CLEAN];
   unsigned int coincidentes2em1[MAX_CLUSTER_CLEAN][MAX_CLUSTER_CLEAN];
   unsigned int total_coincidentes1em2[MAX_CLUSTER_CLEAN];
   unsigned int total_coincidentes2em1[MAX_CLUSTER_CLEAN];
   unsigned int clusters_coincidentes = 0;
   unsigned int primeiro_indice = 0;
   unsigned int ind_img_posterior[MAX_CLUSTER_CLEAN][MAX_MERGE];
   float taxa_expansao = 0;
   FILE *fp = NULL;
   char nome_saida[MAX_TEXTO];
   char nome_arquivo[MAX_TEXTO];
   char msg[MAX_TEXTO];


   gera_img_previsao = TRACK_FALSE;
   
   imagem_sintetica = (short int*) calloc(parametros->nx*parametros->ny,
                                          sizeof(short int));
   if (NULL == imagem_sintetica)
      {
      erro("Memoria insuficiente para execucao do programa",
           __FILE__, __LINE__);
      return TRACK_ERR;
      }
   
   /*
   @ Procura os clusters na imagem mais nova (img 2) e verifica se ha
   @ sistemas que ja existiam na imagem mais velha (img1).
   @ Essa parte da conta dos processos de continuidade, sistemas novos
   @ e merge
   */
   memset(coincidentes2em1, 0, sizeof(coincidentes2em1));
   for (cl = UNDEF_CLUSTER + 1; cl < numcluster2 + 1; cl++)
      {
      for(i = 0; i < parametros->nx*parametros->ny; i++)
         {
         if ((img_cluster2[i] == cl) && (img_cluster1[i] > UNDEF_CLUSTER))
            {
            /*
            @ encontrou sobreposicao de pixeis. Incrementa o numero de pixeis
            @ no vetor de pixeis coincidentes. Usamos um vetor para lidar com
            @ o processo de merge
            */
            coincidentes2em1[cl][img_cluster1[i]]++;
            }
         }
      }

   /*
   @ procura os clusters na imagem mais velha (img 1) e verifica se ha
   @ sistemas que ainda persistem na imagem mais nova (img2).
   @ Essa parte da conta do processo de split
   */
   memset(coincidentes1em2, 0, sizeof(coincidentes1em2));
   for (cl = UNDEF_CLUSTER + 1; cl < numcluster1 + 1; cl++)
      {
      for(i = 0; i < parametros->nx*parametros->ny; i++)
         {
         if ((img_cluster1[i] == cl) && (img_cluster2[i] > UNDEF_CLUSTER))
            {
            /*
            @ encontrou sobreposicao de pixeis. Incrementa o numero de pixeis
            @ no vetor de pixeis coincidentes. Usamos um vetor para lidar com
            @ o processo de split
            */
            coincidentes1em2[cl][img_cluster2[i]]++;
            }
         }
      }
   
   /*verificacao dos tipos de sistema*/
   for (cl2 = UNDEF_CLUSTER + 1; cl2 < numcluster2 + 1; cl2++)
      {
      cl = 0;
      for (cl1 = UNDEF_CLUSTER + 1; cl1 < numcluster1 + 1; cl1++)
         {
         if ((100*coincidentes2em1[cl2][cl1] / cluster2[cl2].total_pix)
             > parametros->porcentagem_sobreposicao)
            {
            /*sistema eh continuidade de um anterior, adiciona esse
            indice na lista de clusters na imagem anterior*/
            cluster2[cl2].ind_img_ant[cl++] = cl1;
            }
         }
      }

   /*Para tratarmos o processo de split, invertemos a logica,
   usando a imagem 2 como 1 e tratando o split como um merge ao
   contrario*/
   memset(ind_img_posterior, 0, sizeof(ind_img_posterior));
   for (cl1 = UNDEF_CLUSTER + 1; cl1 < numcluster1 + 1; cl1++)
      {
      cl = 0;
      for (cl2 = UNDEF_CLUSTER + 1; cl2 < numcluster2 + 1; cl2++)
         {
         if ((100*coincidentes1em2[cl1][cl2] / cluster2[cl2].total_pix)
             > parametros->porcentagem_sobreposicao)
            {
            /*sistema eh continuidade de um anterior, adiciona esse
            indice na lista de clusters na imagem anterior*/
            ind_img_posterior[cl1][cl++] = cl2;
            }
         }
      }

   /*Ha uma hierarquia na classificacao do sistema, por isso que os
   IFs nao sao exclusivos*/
   
   for (cl2 = UNDEF_CLUSTER + 1; cl2 < numcluster2 + 1; cl2++)
      {
      if (0 == cluster2[cl2].ind_img_ant[0])
         {
         /*caso 1 - sistema novo*/
         cluster2[cl2].estado_separacao = TIPO_SISTEMA_NOVO;
         }
      else
         {
         /*caso 2 - continuacao - calcula num de pixeis de expansao*/
         cluster2[cl2].estado_separacao = TIPO_SISTEMA_CONTINUACAO;
         taxa_expansao = ((float) cluster2[cl2].total_pix) /
            ((float) cluster1[cluster2[cl2].ind_img_ant[0]].total_pix);
         cluster2[cl2].expansao =
            (taxa_expansao - 1)*cluster2[cl2].total_pix;
         }
      
      if (0 != cluster2[cl2].ind_img_ant[1])
         {
         /*caso 3 - merge/fusao - ha pelo menos 2 sistemas que deram
         origem a esse (indices 0 e 1 no vetor sao diferentes de zero)*/
         cluster2[cl2].estado_separacao = TIPO_SISTEMA_MERGE;
         }
      }


   /*caso 4 - split - ha pelo menos 2 sistemas sao originarios de
   outro sistema*/
   for (cl1 = UNDEF_CLUSTER + 1; cl1 < numcluster1 + 1; cl1++)
      {
      if (0 != ind_img_posterior[cl1][1])
         {
         cl = 0;
         while (0 != ind_img_posterior[cl1][cl])
            {
            cluster2[ind_img_posterior[cl1][cl]].estado_separacao =
               TIPO_SISTEMA_SPLIT;
            cluster2[ind_img_posterior[cl1][cl]].ind_img_ant[0] = cl1;
            cluster2[ind_img_posterior[cl1][cl]].ind_img_ant[1] = 0;
            cl++;
            }
         }
      }

   

#if 0
   
   /*caso 4 - split - ha pelo menos 2 sistemas sao originarios de
   outro sistema*/
   for (cl1 = UNDEF_CLUSTER + 1; cl1 < numcluster1 + 1; cl1++)
      {
      if (0 != ind_img_posterior[cl1][1])
         {
         cl = 0;
         while (0 != ind_img_posterior[cl1][cl])
            {
            if (TIPO_SISTEMA_MERGE !=
                cluster2[ind_img_posterior[cl1][cl]].estado_separacao)
               {
               /*existem casos especiais onde um sistema pode ser classificado
               ambiguamente como split ou merge, esses casos serao considerados
               como split*/
               cluster2[ind_img_posterior[cl1][cl]].estado_separacao =
                  TIPO_SISTEMA_SPLIT;
               for (i=1; i < MAX_MERGE; i++)
                  {
                  cluster2[ind_img_posterior[cl1][cl]].ind_img_ant[i] = 0;
                  }
               }
            cl++;
            }
         }
      }
 #endif  
   /*calcula a velocidade dos clusters entre a img1 e img2*/
   calcula_deslocamento(cluster1, cluster2, parametros, numcluster2);
   /*se nao for pra gerar a imagem sintetica, retorna*/
   if (TRACK_FALSE == gera_img_previsao)
      {
      free(imagem_sintetica);
      return TRACK_OK;
      }
   
   for (i = 1; i < parametros->num_prev + 1; i++)
      {
      /*gera a imagem sintetica da previsao*/      
      memset(nome_saida, 0, sizeof(nome_saida));
      memset(nome_arquivo, 0, sizeof(nome_arquivo));
      memset(msg, 0, sizeof(msg));

      preenche_imagem_sintetica(imagem_sintetica, imagem, img_cluster2,
                                cluster2, numcluster2, parametros,
                                parametros->deltat*i);
      
      novo_nome(parametros->nome_arquivo_atual, nome_arquivo, parametros->deltat*i);
      sprintf(nome_saida, "%s/%s", parametros->dir_saida_previsao, nome_arquivo);
      fp = fopen(nome_saida, "w");
      if (NULL == fp)
         {
         sprintf(msg, "Erro criando arquivo de previsao %s", nome_arquivo);
         erro(msg, __FILE__, __LINE__);
         }
      else
         {
         if (parametros->nx*parametros->ny !=
             fwrite((void *) imagem_sintetica, sizeof(short int),
                    parametros->nx*parametros->ny, fp))
            {
            sprintf(msg, "Erro criando arquivo de previsao %s", nome_arquivo);
            erro(msg, __FILE__, __LINE__);
            }
         fclose(fp);
         }
      }
   
   free(imagem_sintetica);
   return TRACK_OK;
   }

void preenche_imagem_sintetica(short int *imagem_sintetica, short int *imagem,
                               unsigned int *img_cluster2,
                               info_sistema *cluster2,
                               int total_cluster, params *parametros,
                               int deltat)
   {
   int cl = 0, i = 0, j = 0;
   int numpix_add = 0;
   int x_centro = 0, y_centro = 0;
   int x_ini = 0, y_ini = 0;
   float angulo = 0, deslocamento = 0;
   int dx[MAX_CLUSTER_CLEAN];
   int dy[MAX_CLUSTER_CLEAN];
   
   memset(dx, 0, sizeof(dx));
   memset(dy, 0, sizeof(dy));

   /*inicializacao da imagem*/
   for (i = 0; i < parametros->nx*parametros->ny; i++)
      {
      imagem_sintetica[i] = parametros->UNDEF_DATA;
      }
   
   for (cl = UNDEF_CLUSTER + 1; cl < total_cluster + 1; cl++)
      {
      if (TIPO_SISTEMA_NOVO != cluster2[cl].estado_separacao)
         {
         /*so calcula uma posicao nova pra sistemas que ja existiam,
         para sistemas novos dx e dy sao mantidos com velocidade zero*/
         deslocamento = cluster2[cl].velocidade * parametros->deltat;
         angulo = M_PI*cluster2[cl].direcao/180;
         dx[cl] = round(cos(angulo)*deslocamento);
         dy[cl] = round(sin(angulo)*deslocamento);
         }
      }

   for (i = 0; i < parametros->nx; i++)
      {
      for (j = 0; j < parametros->ny; j++)
         {
         /*verifica de qual sistema eh o pixel em questao e se
         o sistema vai passar das bordas da img no tempo futuro*/
         cl = img_cluster2[i + parametros->nx*j];
         if ((cl != UNDEF_CLUSTER) &&
             ((dx[cl] + i) < parametros->nx) &&
             ((dy[cl] + j) < parametros->nx) &&
             ((dx[cl] + i) >= 0) &&
             ((dy[cl] + j) >= 0))
            {
            imagem_sintetica[(dx[cl] + i) +
                             parametros->nx*(j + dy[cl])] =
               imagem[i + parametros->nx*j];
            }
         }
      }
   /*imagem nova estah preenchida, verifica os
   clusters que devem ser aumentados ou diminuidos*/
   for (cl = UNDEF_CLUSTER + 1; cl < total_cluster + 1; cl++)
      {
      if (TIPO_SISTEMA_CONTINUACAO == cluster2[cl].estado_separacao)
         {
         x_centro = dx[cl] + cluster2[cl].x_centro;
         y_centro = dy[cl] + cluster2[cl].y_centro;
         if ((x_centro >= 0) && (x_centro < parametros->nx) &&
             (y_centro >= 0) && (y_centro < parametros->nx))
            {
            preenche_borda(imagem_sintetica, x_centro, y_centro, parametros,
                           cluster2[cl].expansao, cluster2[cl].valor_medio);
            }
         }
      }
   }


void preenche_borda(short int *imagem_sintetica, int x, int y,
                    params *parametros, int add, float valor)
   {
   int incremento = 1, i = 0, j = 0;
   int max_add = parametros->nx*parametros->ny;
   int cont = 1, sinal = 1;
   int nx = parametros->nx;
   int xorig = 0, yorig = 0;
   
   if (add > 0) incremento = -1;
   
   while ((add*incremento < 0) || (0 != max_add--))
      {
      /*preenche a imagem, procura pelas bordas no sentido E - W - N - S*/
      for (i = x; i < parametros->nx - 1; i++)
         {
         if ((imagem_sintetica[(i + 1) + nx*y]) == parametros->UNDEF_DATA)
            {
            imagem_sintetica[(i + 1) + nx*y] = valor;
            add+=incremento;
            if (add*incremento < 0) return;
            break;
            }
         }
      for (i = x; i > 0; i--)
         {
         if ((imagem_sintetica[(i - 1) + nx*y]) == parametros->UNDEF_DATA)
            {
            imagem_sintetica[(i - 1) + nx*y] = valor;
            add+=incremento;
            if (add*incremento < 0) return;
            break;
            }
         }
      for (j = y; j < parametros->ny; j++)
         {
         if ((imagem_sintetica[x + nx*(j + 1)]) == parametros->UNDEF_DATA)
            {
            imagem_sintetica[x + nx*(j + 1)] = valor;
            add+=incremento;
            if (add*incremento < 0) return;
            break;
            }
         }
      for (j = y; j > 0; j--)
         {
         if ((imagem_sintetica[(x + 1) + nx*j]) == parametros->UNDEF_DATA)
            {
            imagem_sintetica[x + nx*(j - 1)] = valor;
            add+=incremento;
            if (add*incremento < 0) return;
            break;
            }
         }

      /*calcula as novas posicoes iniciais x e y para busca*/
      x = x + sinal*cont++;
      y = x + sinal*cont++;
      sinal = -sinal;
      if ((x < 0) || (y < 0) ||
          (x >= parametros->nx) || (y >= parametros->ny))
         {
         return;
         }
      }
   }
