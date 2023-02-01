#include "trackdef.h"

int le_parametros(params *parametros, char *arquivo)
   {
   char linha[MAX_TEXTO];
   char msg[MAX_TEXTO];
   char fim_arquivo = TRACK_FALSE;
   int linhanum = 0;
   FILE *file;
   file = fopen(arquivo, "r");
   if (NULL == file)
      {
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "Arquivo de parametros %s nao encontrado", arquivo);
      erro(msg, __FILE__, __LINE__);      
      return TRACK_ERR;
      }
   
   while(fgets(linha, MAX_TEXTO, file) != NULL)
      {
      if(linha[0] == '#') continue;

      switch (linhanum)
         {
         case 0:
            /*copia o nome sem incluir o \n (caracter de fim de linha)*/
            strncpy(parametros->dir_entrada_imagens, linha, strlen(linha)-1);
            break;
         case 1:
            strncpy(parametros->dir_saida_previsao, linha, strlen(linha)-1);
            break;
         case 2:
            strncpy(parametros->dir_saida_estatisticas, linha, strlen(linha)-1);
            break;
         case 3:
            parametros->tipo_imagem = (char) atoi(linha);
            break;
         case 4:
            parametros->tamanho_minimo = (unsigned short int) atoi(linha);
            break;
         case 5:
            parametros->pct_pixeis_invalidos = (unsigned short int) atoi(linha);
            break;
         case 6:
            parametros->num_linhas_invalidas_consecutiva =
               (unsigned short int) atoi(linha);
            break;
         case 7:
            parametros->unidade_espacamento = (unsigned short int) atoi(linha);
            break;
         case 8:
            parametros->nx = (unsigned short int) atoi(linha);
            break;
         case 9:
            parametros->ny = (unsigned short int) atoi(linha);
            break;
         case 10:
            parametros->dx = (float) atof(linha);
            break;
         case 11:
            parametros->dy = (float) atof(linha);
            break;
         case 12:
            parametros->canto_nw_x = (float) atof(linha);
            break;
         case 13:
            parametros->canto_nw_y = (float) atof(linha);
            break;
         case 14:
            parametros->limiar = 100 * ((float) atof(linha));
            break;
         case 15:
            parametros->deltat = atoi(linha);
            break;
         case 16:
            parametros->num_prev = atoi(linha);
            break;
         case 17:
            parametros->t_maximo = atoi(linha);
            break;
         case 18:
            parametros->porcentagem_sobreposicao = atoi(linha);
            break;
         case 19:
            parametros->UNDEF_DATA = atoi(linha);
            break;
         case 20:
            parametros->display = atoi(linha);
            /*leu a ultima linha*/
            fim_arquivo = TRACK_TRUE;
            break;
         default:
            break;            
         }
      linhanum++;
      }
   fclose(file);

   if (TRACK_FALSE == fim_arquivo)
      {
      /*nao leu a ultima linha - nao leu o arquivo inteiro*/
      erro("Arquivo de parametros parametros.txt incompleto", __FILE__, __LINE__);
      return TRACK_ERR;
      }
   

   if (1 == parametros->unidade_espacamento)
      {
      parametros->dx = parametros->dx / 111111;
      parametros->dy = parametros->dy / 111111;
      }
   
   
   return TRACK_OK;
   }
