#include "trackdef.h"

FILE *uncompress_pipe (FILE *fp)
   {
   /* Pass the file pointed to by 'fp' through the gzip pipe. */
   
   FILE *fpipe;
   int save_fd;
   
   save_fd = dup(0);
   close(0); /* Redirect stdin for gzip. */
   dup(fileno(fp));
   
   fpipe = popen("gzip -q -d -f --stdout", "r");
   if (fpipe == NULL) perror("uncompress_pipe");
   close(0);
   dup(save_fd);
   fclose(fp);
   return fpipe;
   }

/*
@ Emite uma msg de erro na tela com a linha e o arquivo fonte que
@ gerou o erro
*/
void erro(char *texto, char *arquivo, int linha)
   {
   printf("%s (fonte: %s - linha %d)\n", texto, arquivo, linha);
   }


/*
@
@ Cria o nome do arquivo previsto com base na data atual
@
*/
void novo_nome(char *nome_entrada, char *nome_saida, int deltat)
   {
   struct tm cal_time_img;
   struct tm *tempo_futuro;
   char ano[5];
   char mes[3];
   char dia[3];
   char hora[3];
   char minuto[3];
   char segundo[3];
   time_t tempo;
   memset(&cal_time_img, 0, sizeof(cal_time_img));

   memset(ano, 0, sizeof(ano));
   memset(mes, 0, sizeof(mes));
   memset(dia, 0, sizeof(dia));
   memset(hora, 0, sizeof(hora));
   memset(minuto, 0, sizeof(minuto));
   memset(segundo, 0, sizeof(segundo));

   memcpy(ano, &nome_entrada[0], 4);
   memcpy(mes, &nome_entrada[4], 2);
   memcpy(dia, &nome_entrada[6], 2);
   memcpy(hora, &nome_entrada[8], 2);
   memcpy(minuto, &nome_entrada[10], 2);
   memcpy(segundo, &nome_entrada[12], 2);
   
   cal_time_img.tm_year = atoi(ano) - 1900;
   cal_time_img.tm_mon  = atoi(mes) - 1;
   cal_time_img.tm_mday = atoi(dia);
   cal_time_img.tm_hour = atoi(hora);
   cal_time_img.tm_min  = atoi(minuto);
   cal_time_img.tm_sec  = atoi(segundo);
   /*corrigindo para nao haver diferenca entre horario normal
   e horario de verao. Se esse valor for 0 ou positivo a funcao
   mktime convertera o horario para horario de verao no caso do mesmo
   ser valido pra epoca do ano determinada pela data no arquivo*/
   cal_time_img.tm_isdst = -1;
   
   tempo = mktime(&cal_time_img) + deltat*60;
   tempo_futuro = localtime(&tempo);
   sprintf(nome_saida, "%04d%02d%02d%02d%02d00.PREV.bin",
           tempo_futuro->tm_year+1900, tempo_futuro->tm_mon + 1,
           tempo_futuro->tm_mday, tempo_futuro->tm_hour,
           tempo_futuro->tm_min);
   
   return;
   }


/*
@ Verifica o intervalo de tempo entre 2 imagens com base
@ no nome das mesmas
*/
int verifica_intervalo(char *nome_img1, char *nome_img2,
                       unsigned short int intervalo_maximo,
                       float *intervalo)
   {
   struct tm cal_time_img1, cal_time_img2;
   char ano[5];
   char mes[3];
   char dia[3];
   char hora[3];
   char minuto[3];
   char segundo[3];
   float tempo1, tempo2;
   char msg[MAX_TEXTO];
   
   
   memset(&cal_time_img1, 0, sizeof(cal_time_img1));
   memset(&cal_time_img2, 0, sizeof(cal_time_img2));

   memset(ano, 0, sizeof(ano));
   memset(mes, 0, sizeof(mes));
   memset(dia, 0, sizeof(dia));
   memset(hora, 0, sizeof(hora));
   memset(minuto, 0, sizeof(minuto));
   memset(segundo, 0, sizeof(segundo));

   memcpy(ano, &nome_img1[0], 4);
   memcpy(mes, &nome_img1[4], 2);
   memcpy(dia, &nome_img1[6], 2);
   memcpy(hora, &nome_img1[8], 2);
   memcpy(minuto, &nome_img1[10], 2);
   memcpy(segundo, &nome_img1[12], 2);
   
   cal_time_img1.tm_year = atoi(ano) - 1900;
   cal_time_img1.tm_mon  = atoi(mes) - 1;
   cal_time_img1.tm_mday = atoi(dia);
   cal_time_img1.tm_hour = atoi(hora);
   cal_time_img1.tm_min  = atoi(minuto);
   cal_time_img1.tm_sec  = atoi(segundo);

   memset(ano, 0, sizeof(ano));
   memset(mes, 0, sizeof(mes));
   memset(dia, 0, sizeof(dia));
   memset(hora, 0, sizeof(hora));
   memset(minuto, 0, sizeof(minuto));
   memset(segundo, 0, sizeof(segundo));

   memcpy(ano, &nome_img2[0], 4);
   memcpy(mes, &nome_img2[4], 2);
   memcpy(dia, &nome_img2[6], 2);
   memcpy(hora, &nome_img2[8], 2);
   memcpy(minuto, &nome_img2[10], 2);
   memcpy(segundo, &nome_img2[12], 2);
   
   cal_time_img2.tm_year = atoi(ano) - 1900;
   cal_time_img2.tm_mon  = atoi(mes) - 1;
   cal_time_img2.tm_mday = atoi(dia);
   cal_time_img2.tm_hour = atoi(hora);
   cal_time_img2.tm_min  = atoi(minuto);
   cal_time_img2.tm_sec  = atoi(segundo);

   tempo1 = (float) mktime(&cal_time_img1);
   tempo2 = (float) mktime(&cal_time_img2);
   /*retorna o intervalo em minutos*/
   *intervalo = roundf((tempo2 - tempo1) / 60);
   
   if ((*intervalo <= 0) || (*intervalo > intervalo_maximo))
      {
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "Intervalo de tempo invalido entre as imagens %s e %s",
              nome_img1, nome_img2);
      erro(msg, __FILE__, __LINE__);
      return TRACK_ERR;
      }
   
   
   return TRACK_OK;
   }

/*
@ Le um arquivo de imagem de satelite ou radar
*/
int le_arquivo_imagem(short int *img, params *parametros, char *nome)
   {
   FILE *fp = NULL;
   char msg[MAX_TEXTO];
   char arquivo[MAX_TEXTO];
   int i;
   
   memset(arquivo, 0, sizeof(arquivo));
   strcpy(arquivo, parametros->dir_entrada_imagens);
   strcat(arquivo, "/");
   strcat(arquivo, nome);
  
   fp = fopen(arquivo, "rb");
   if (NULL == fp)
      {
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "Erro abrindo arquivo %s", nome);
      erro(msg, __FILE__, __LINE__);
      return TRACK_ERR;
      }

   
   // fp = uncompress_pipe(fp);
   
   memset(img, 0, sizeof(short int)*parametros->nx*parametros->ny);
   
   if (parametros->nx*parametros->ny !=
       fread((void *) img, sizeof(short int), parametros->nx*parametros->ny, fp))
      {
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "Tamanho do arquivo %s incompativel com numero de linhas e colunas", nome);
      erro(msg, __FILE__, __LINE__);
      fclose(fp);
      return TRACK_ERR;
      }
   fclose(fp);
#if 0
   for (i=0; i < parametros->nx*parametros->ny; i ++)
      {
      if (img[i] > 0)
         printf("%d ", i);
      }
   printf("\n");
#endif   
   memset(parametros->nome_arquivo_atual, 0, sizeof(parametros->nome_arquivo_atual));
   strncpy(parametros->nome_arquivo_atual, nome, MAX_TEXTO);
   
   /*arquivo lido com sucesso, verifica qtd de linhas e pixeis invalidos*/
   return verifica_arquivo_imagem(img, parametros);
   }

/*
@ Verifica a quantidade de pixeis invalidos e a qt de linhas invalidas
@
*/

int verifica_arquivo_imagem(short int *imagem, params *parametros)
   {
   unsigned int i=0, j=0, invalidos=0;
   unsigned int total_linhas_consecutivas = 0;
   float total_pixeis = 0;
   
   /*indice -2 porque comparamos o indice da linha com o indice da
   linha anterior. Se usarmos -1 teremos 2 linhas invalidas ja na primeira
   linha caso a linha 0 seja invalida*/
   int linha_anterior = -2;
   
   int encontrou_linha_invalida = TRACK_FALSE;
   char msg[MAX_TEXTO];

   /*conta a qtd de pixeis invalidos*/
   for (i = 0; i < parametros->nx*parametros->ny; i++)
      {
      if (imagem[i + parametros->nx*j] == parametros->UNDEF_DATA)
         {
         invalidos++;
         }
      }

   total_pixeis = parametros->nx*parametros->ny;
   
   
   if (100*(invalidos / total_pixeis) >= parametros->pct_pixeis_invalidos)
      {
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "Arquivo %s possui mais de %d%% de pixeis invalidos",
              parametros->nome_arquivo_atual,
              parametros->pct_pixeis_invalidos);
      erro(msg, __FILE__, __LINE__);
      return TRACK_ERR;
      }

   /*conta a qtd de linhas invalidas*/   
   for (j = 0; j < parametros->ny; j++)
      {
      invalidos = 0;
      for (i = 0; i < parametros->nx; i++)
         {
         if (imagem[i + parametros->nx*j] == parametros->UNDEF_DATA)
            {
            invalidos++;
            }
         }
      if (invalidos == parametros->nx)
         {
         /*toda essa linha eh invalida*/
         if ((j - 1) == linha_anterior)
            {
            total_linhas_consecutivas++;
            }
         else
            {
            total_linhas_consecutivas = 1;
            }
         linha_anterior = j;
         if (total_linhas_consecutivas >=
             parametros->num_linhas_invalidas_consecutiva)
            {
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "Arquivo %s possui mais de %d linhas invalidas consecutivas",
                    parametros->nome_arquivo_atual,
                    parametros->num_linhas_invalidas_consecutiva);
            erro(msg, __FILE__, __LINE__);
            return TRACK_ERR;
            }
         }
      }

   if (total_linhas_consecutivas > 0)
      {
      /*tem linhas invalidas, mas nao acima do limite informado.
      Duplica a linha acima para nao haver descontinuidade nos sistemas*/

      for (j = 1; j < parametros->ny; j++)
         {
         invalidos = 0;
         for (i = 0; i < parametros->nx; i++)
            {
            if (imagem[i + parametros->nx*j] == parametros->UNDEF_DATA)
               {
               invalidos++;
               }
            }
         if (invalidos == parametros->nx)
            {
            /*essa linha toda eh invalida, preenche com a linha anterior*/
            for (i = 0; i < parametros->nx; i++)
               {
               imagem[i + parametros->nx*j] = imagem[i + parametros->nx*(j-1)];
               }
            }
         }
      }
   
   return TRACK_OK;
   
   }

/*
@
@ Grava o arquivo de saida dos clusters
@
*/
void grava_arquivo_clusters(unsigned int *img_cluster, params *parametros)
   {
   char tmpStr[MAX_TEXTO];
   char msg[MAX_TEXTO];
   FILE *fp = NULL;
   
   memset(tmpStr, 0, sizeof(tmpStr));
   sprintf(tmpStr, "%s/%s.clusters",
           parametros->dir_saida_estatisticas,
           parametros->nome_arquivo_atual);
   fp = fopen(tmpStr, "w");
   if (NULL != fp)
      {
      fwrite(img_cluster, sizeof(unsigned int),
             parametros->nx*parametros->ny, fp);
      fclose(fp);
      memset(tmpStr, 0, sizeof(tmpStr));
      sprintf(tmpStr, "gzip -f %s/%s.clusters",
              parametros->dir_saida_estatisticas,
              parametros->nome_arquivo_atual);
      (void) system(tmpStr);
      }
   else
      {
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "Erro criando arquivo de saida %s.clusters",
              parametros->nome_arquivo_atual);
      erro(msg, __FILE__, __LINE__);
      }
   return;
   }

