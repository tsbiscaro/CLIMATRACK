#include "trackdef.h"

/*
@ chama a funcao pra mostrar na tela ou gravar em arquivo as informacoes
*/
void display_info(info_sistema *clusters, unsigned int max_cluster,
                  params *parametros)
   {
   switch (parametros->display)
      {
      case 1:
         print_info(clusters, max_cluster, parametros);
         break;
      case 2:
         fprint_info(clusters, max_cluster, parametros);
         break;
      case 3:
         print_info(clusters, max_cluster, parametros);
         fprint_info(clusters, max_cluster, parametros);
         break;
      default:
         break;
      }   
   return;
   }

/*
@ Apresenta as informacoes sobre os sistemas
*/
void fprint_info(info_sistema *clusters, unsigned int max_cluster,
                 params *parametros)
   {
   int i = 0, j = 0;
   char ano[5];
   char mes[3];
   char dia[3];
   char hora[3];
   char minuto[3];
   char segundo[3];
   float ang=0;
   int c = 34;
   char tipo[2];
   FILE *fp = NULL;
   char arq_saida[MAX_TEXTO];
   char msg[MAX_TEXTO];
   char unidade[MAX_TEXTO];
   char velocidade[MAX_TEXTO];
   float tamanho = 0, vel = 0;
   
   memset(arq_saida, 0, sizeof(arq_saida));
   sprintf(arq_saida, "%s/%s.TXT", parametros->dir_saida_estatisticas,
           parametros->nome_arquivo_atual);

   fp = fopen(arq_saida, "w");
   if (NULL == fp)
      {
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "Erro abrindo arquivo %s", arq_saida);
      erro(msg, __FILE__, __LINE__);
      return;
      }
   
   memset(ano, 0, sizeof(ano));
   memset(mes, 0, sizeof(mes));
   memset(dia, 0, sizeof(dia));
   memset(hora, 0, sizeof(hora));
   memset(minuto, 0, sizeof(minuto));
   memset(segundo, 0, sizeof(segundo));
   
   memcpy(ano, &parametros->nome_arquivo_atual[0], 4);
   memcpy(mes, &parametros->nome_arquivo_atual[4], 2);
   memcpy(dia, &parametros->nome_arquivo_atual[6], 2);
   memcpy(hora, &parametros->nome_arquivo_atual[8], 2);
   memcpy(minuto, &parametros->nome_arquivo_atual[10], 2);
   memcpy(segundo, &parametros->nome_arquivo_atual[12], 2);
   fprintf(fp, "   ATUAL %s\n", parametros->nome_arquivo_atual);
   fprintf(fp, "ANTERIOR %s\n", parametros->nome_arquivo_anterior);
   fprintf(fp, "Data %s-%s-%s - Hora %s:%s:%s\n", ano, mes, dia,
          hora, minuto, segundo);
   
   if (IMGRADAR == parametros->tipo_imagem)
      {
      fprintf(fp, "Tipo: RADAR\n");
      }
   else
      {
      fprintf(fp, "Tipo: SATELITE\n");
      }
   fprintf(fp, "Sistema | Latitude | Longitude | Tamanho | Media | Extremo | Tipo | Origem |  Vel | Direcao |T3|\n");

   memset(unidade, 0, sizeof(unidade));
   memset(velocidade, 0, sizeof(velocidade));
   if (0 == parametros->unidade_espacamento)
      {
      strcpy(unidade, "  deg2");
      strcpy(velocidade, "d/s");
      }
   else
      {
      strcpy(unidade, "   pix");
      strcpy(velocidade, "m/s");
      }
   
   
   if (IMGRADAR == parametros->tipo_imagem)
      {
      fprintf(fp, "        |     graus|      graus|   %s|    dBZ|      dBZ|      |        |   %s|   graus |  |\n", unidade, velocidade);
      }
   else
      {
      fprintf(fp, "        |     graus|      graus|   %s|      K|        K|      |        |   %s|   graus |\n", unidade, velocidade);
      }
   fprintf(fp, "---------------------------------------------------------------------------------------------\n");
   for (i = 1; i < max_cluster + 1; i++)
      {
      ang = 90 - clusters[i].direcao;
      if (ang < 0) ang+=360;
      switch (clusters[i].estado_separacao)
         {
         case TIPO_SISTEMA_NOVO:
            strcpy(tipo, "N\0");
            break;
         case TIPO_SISTEMA_SPLIT:
            strcpy(tipo, "S\0");
            break;
         case TIPO_SISTEMA_MERGE:
            strcpy(tipo, "F\0");
            break;
         case TIPO_SISTEMA_CONTINUACAO:
            strcpy(tipo, "C\0");
            break;
         default:
            strcpy(tipo, "E\0");
            break;
         }


      /*calcula o tamanho em km2 ou em grau*/
      if (0 == parametros->unidade_espacamento)
         {
         tamanho = clusters[i].total_pix*parametros->dx*parametros->dy/(1000*1000);
         vel = clusters[i].velocidade;
         }
      else
         {
         tamanho = 111*111*clusters[i].total_pix*parametros->dx*parametros->dy;
         vel = clusters[i].velocidade*111000;
         }

      tamanho = clusters[i].total_pix;
      
      switch (clusters[i].estado_separacao)
         {
         case TIPO_SISTEMA_MERGE:
            {
            /*se for um sistema que tem mais de 1 "pai",
            ou seja, um sistema que se originou
            a partir da fusao de outros sistemas,
            escreve a origem dele tambem*/
            j = 0;
            fprintf(fp, "%7d |%10.5f| %10.5f|%9d|%7.3f|%9.3f|     %s|%8d|%6.2f|%9.3f| %01d|\n",
                    i, (float) clusters[i].y_centro,
                    (float) clusters[i].x_centro,
                    (int) tamanho, clusters[i].valor_medio,
                    clusters[i].valor_extremo, tipo, clusters[i].ind_img_ant[j++],
                    vel*((parametros->dx +parametros->dy)/2)/60, ang, clusters[i].over_t3);
            while ((clusters[i].ind_img_ant[j] != 0) && (j < MAX_MERGE))
               {
               fprintf(fp, "        |          |           |         |       |         |      |%8d|      |         |\n", clusters[i].ind_img_ant[j++]);
               }
            break;
            }
         case TIPO_SISTEMA_NOVO:
            {
            fprintf(fp, "%7d |%10.5f| %10.5f|%9d|%7.3f|%9.3f|     %s|        |      |         | %01d|\n",
                    i, (float) clusters[i].y_centro,
                    (float) clusters[i].x_centro,
                    (int) tamanho, clusters[i].valor_medio,
                    clusters[i].valor_extremo, tipo, clusters[i].over_t3);
            break;
            }
         case TIPO_SISTEMA_CONTINUACAO:
         case TIPO_SISTEMA_SPLIT:
         default:
            {
            fprintf(fp, "%7d |%10.5f| %10.5f|%9d|%7.3f|%9.3f|     %s|%8d|%6.2f|%9.3f| %01d|\n",
                    i, (float) clusters[i].y_centro,
                    (float) clusters[i].x_centro,
                    (int) tamanho, clusters[i].valor_medio,
                    clusters[i].valor_extremo, tipo, clusters[i].ind_img_ant[0],
                    vel*((parametros->dx +parametros->dy)/2)/60, ang, clusters[i].over_t3);
            break;
            }            
         }
      }

   fclose(fp);
   
   return;
   }


void print_info(info_sistema *clusters, unsigned int max_cluster,
                params *parametros)
   {
   int i = 0, j = 0;
   char ano[5];
   char mes[3];
   char dia[3];
   char hora[3];
   char minuto[3];
   char segundo[3];
   float ang=0;
   int c = 34;
   char tipo[2];
   char unidade[MAX_TEXTO];
   char velocidade[MAX_TEXTO];
   float tamanho = 0, vel = 0;
   
   memset(ano, 0, sizeof(ano));
   memset(mes, 0, sizeof(mes));
   memset(dia, 0, sizeof(dia));
   memset(hora, 0, sizeof(hora));
   memset(minuto, 0, sizeof(minuto));
   memset(segundo, 0, sizeof(segundo));
   
   memcpy(ano, &parametros->nome_arquivo_atual[0], 4);
   memcpy(mes, &parametros->nome_arquivo_atual[4], 2);
   memcpy(dia, &parametros->nome_arquivo_atual[6], 2);
   memcpy(hora, &parametros->nome_arquivo_atual[8], 2);
   memcpy(minuto, &parametros->nome_arquivo_atual[10], 2);
   memcpy(segundo, &parametros->nome_arquivo_atual[12], 2);
   printf("\n   ATUAL %s\n", parametros->nome_arquivo_atual);
   printf("ANTERIOR %s\n", parametros->nome_arquivo_anterior);
   printf("Data %s-%s-%s - Hora %s:%s:%s\n", ano, mes, dia,
          hora, minuto, segundo);
   if (IMGRADAR == parametros->tipo_imagem)
      {
      printf("Tipo: RADAR\n");
      }
   else
      {
      printf("Tipo: SATELITE\n");
      }


   memset(unidade, 0, sizeof(unidade));
   memset(velocidade, 0, sizeof(velocidade));
   if (0 == parametros->unidade_espacamento)
      {
      strcpy(unidade, "  deg2");
      strcpy(velocidade, "d/s");
      }
   else
      {
      strcpy(unidade, "   pix");
      strcpy(velocidade, "m/s");
      }
   
   
   printf("Sistema | Latitude | Longitude | Tamanho | Media | Extremo | Tipo | Origem |  Vel | Direcao |T3|\n");
   if (IMGRADAR == parametros->tipo_imagem)
      {
      printf("        |     graus|      graus|   %s|    dBZ|      dBZ|      |        |   %s|   graus |\n", unidade, velocidade);
      }
   else
      {
      printf("        |     graus|      graus|   %s|      K|        K|      |        |   %s|   graus |\n", unidade, velocidade);
      }   
   printf("---------------------------------------------------------------------------------------------\n");
   for (i = 1; i < max_cluster + 1; i++)
      {
      ang = 90 - clusters[i].direcao;
      if (ang < 0) ang+=360;
      switch (clusters[i].estado_separacao)
         {
         case TIPO_SISTEMA_NOVO:
            strcpy(tipo, "N\0");
            break;
         case TIPO_SISTEMA_SPLIT:
            strcpy(tipo, "S\0");
            break;
         case TIPO_SISTEMA_MERGE:
            strcpy(tipo, "F\0");
            break;
         case TIPO_SISTEMA_CONTINUACAO:
            strcpy(tipo, "C\0");
            break;
         default:
            strcpy(tipo, "E\0");
            break;
         }

      /*calcula o tamanho em km2 ou em grau*/
      if (0 == parametros->unidade_espacamento)
         {
         tamanho = clusters[i].total_pix*parametros->dx*parametros->dy/(1000*1000);
         vel = clusters[i].velocidade;
         }
      else
         {
         tamanho = 111*111*clusters[i].total_pix*parametros->dx*parametros->dy;
         vel = clusters[i].velocidade*111000;
         }
      tamanho = clusters[i].total_pix;

      switch (clusters[i].estado_separacao)
         {
         case TIPO_SISTEMA_MERGE:
            {
            /*se for um sistema que tem mais de 1 "pai",
            ou seja, um sistema que se originou
            a partir da fusao de outros sistemas,
            escreve a origem dele tambem*/
            j = 0;

            printf("%7d |%10.5f| %10.5f|%9d|%7.3f|%9.3f|     %s|%8d|%6.2f|%9.3f| %01d|\n",
                   i, (float) clusters[i].y_centro,
                   (float) clusters[i].x_centro,
                   (int) tamanho, clusters[i].valor_medio,
                   clusters[i].valor_extremo, tipo, clusters[i].ind_img_ant[j++],
                   vel*((parametros->dx +parametros->dy)/2)/60, ang,clusters[i].over_t3);
            while ((clusters[i].ind_img_ant[j] != 0) && (j < MAX_MERGE))
               {
               printf("        |          |           |         |       |         |      |%8d|      |         |\n", clusters[i].ind_img_ant[j++]);
               }
            break;
            }
         case TIPO_SISTEMA_NOVO:
            {
            printf("%7d |%10.5f| %10.5f|%9d|%7.3f|%9.3f|     %s|        |      |         | %01d|\n",
                   i, (float) clusters[i].y_centro,
                   (float) clusters[i].x_centro,
                   (int) tamanho, clusters[i].valor_medio,
                   clusters[i].valor_extremo, tipo, clusters[i].over_t3);
            break;
            }
         case TIPO_SISTEMA_CONTINUACAO:
         case TIPO_SISTEMA_SPLIT:
         default:
            {
            printf("%7d |%10.5f| %10.5f|%9d|%7.3f|%9.3f|     %s|%8d|%6.2f|%9.3f| %01d|\n",
                   i, (float) clusters[i].y_centro,
                   (float) clusters[i].x_centro,
                   (int) tamanho, clusters[i].valor_medio,
                   clusters[i].valor_extremo, tipo, clusters[i].ind_img_ant[0],
                   vel*((parametros->dx + parametros->dy)/2)/60, ang,clusters[i].over_t3);
            break;
            }            
         }
      }

   return;
   }

