#include "trackdef.h"

short int *img1, *img2, *saida;
unsigned int *img_cluster1, *img_cluster2;
info_sistema *cluster1, *cluster2;

int main(int argc, char *argv[])
   {
   struct rlimit rlim;
   params parametros;
   DIR *diretorio;
   struct dirent *arquivo;
   struct stat infofile;
   char msg[MAX_TEXTO];
   char primeira_imagem = TRACK_TRUE;
   unsigned int numcluster_img1 = 0, numcluster_img2 = 0;
   char nome_anterior[MAX_TEXTO];
   char lista_arquivos[MAX_IMAGENS][MAX_TEXTO];
   int i = 0;
   char tmpStr[MAX_TEXTO];
   char inverteu = TRACK_TRUE;
   short int total_arquivos = 0, arquivos_ordenar = 0;
   float intervalo = 0;
   FILE *fp = NULL;

   if (2 != argc)
      {
      printf("Uso: %s <arquivo de parametros>\n", argv[0]);
      return TRACK_ERR;
      }
   
   /*le arquivos de parametros de entrada*/
   memset(&parametros, 0, sizeof(params));
   if (TRACK_ERR == le_parametros(&parametros, argv[1]))
      {
      return TRACK_ERR;
      }

   /*testa se os caminhos existem*/
   diretorio = opendir(parametros.dir_saida_previsao);
   if (NULL == diretorio)
      {
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "Diretorio %s nao encontrado", parametros.dir_saida_previsao);
      erro(msg, __FILE__, __LINE__);
      return TRACK_ERR;
      }
   closedir(diretorio);
   
   diretorio = opendir(parametros.dir_saida_estatisticas);
   if (NULL == diretorio)
      {
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "Diretorio %s nao encontrado", parametros.dir_saida_estatisticas);
      erro(msg, __FILE__, __LINE__);
      return TRACK_ERR;
      }
   closedir(diretorio);
   
   diretorio = opendir(parametros.dir_entrada_imagens);
   if (NULL == diretorio)
      {
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "Diretorio %s nao encontrado", parametros.dir_entrada_imagens);
      erro(msg, __FILE__, __LINE__);
      return TRACK_ERR;
      }

   /*
   getrlimit(RLIMIT_STACK, &rlim);
   rlim.rlim_cur = rlim.rlim_max;
   setrlimit(RLIMIT_STACK, &rlim);
   */
   
   /*aloca as matrizes necessarias*/
   img1 = (short int*) calloc(parametros.nx*parametros.ny, sizeof(short int));
   img2 = (short int*) calloc(parametros.nx*parametros.ny, sizeof(short int));
   img_cluster1 = (unsigned int*) calloc(parametros.nx*parametros.ny,
                                               sizeof(unsigned int));
   img_cluster2 = (unsigned int*) calloc(parametros.nx*parametros.ny,
                                               sizeof(unsigned int));
   saida = (short int*) calloc(parametros.nx*parametros.ny, sizeof(short int));
   cluster1 = (info_sistema*) calloc(MAX_CLUSTER, sizeof(info_sistema));
   cluster2 = (info_sistema*) calloc(MAX_CLUSTER, sizeof(info_sistema));
   if ((NULL == img1) || (NULL == img2) || (NULL == img_cluster1) || (NULL == img_cluster2) ||
       (NULL == saida) || (NULL == cluster1) || (NULL == cluster2))
      {
      erro("Memoria insuficiente para execucao do programa", __FILE__, __LINE__);
      limpa_memoria();      
      return TRACK_ERR;
      }   
   /*
   monta a lista de arquivos de entrada - verifica o conteudo do diretorio
   das imagens de entrada e ignora diretorios e lins simbolicos
   */
   memset(lista_arquivos, 0, sizeof(lista_arquivos));
   while (NULL != (arquivo = readdir(diretorio)))
      {
      /*monta o nome completo do arquivo*/
      memset(tmpStr, 0, sizeof(tmpStr));
      strcpy(tmpStr, parametros.dir_entrada_imagens);
      strcat(tmpStr, "/");
      strcat(tmpStr, arquivo->d_name);
      /*verifica se eh um arquivo comum*/
      stat(tmpStr, &infofile);
      if (!S_ISREG(infofile.st_mode))
         {
         /*
         Se nao for um arquivo comum, pula. O comando readdir nao lista apenas
         arquivos, mas lista diretorios e block devices, o que nao nos interessa.
         ver "man readdir" e "man -a stat" pra informacoes completas
         */
         continue;
         }
      /*eh um arquivo de imagem, coloca na lista*/
      strcpy(lista_arquivos[i++], arquivo->d_name);
      if (i > MAX_IMAGENS)
         {
         /*passou o limite de arquivos, sai*/
         erro("Numero maximo de imagens excedido", __FILE__, __LINE__);
         break;
         }
      }
   closedir (diretorio);
   total_arquivos = i;
   arquivos_ordenar = i;
   /*coloca a lista de arquivos em ordem alfabetica*/
   while (inverteu)
      {
      inverteu = TRACK_FALSE;
      for (i = 0; i < arquivos_ordenar - 1; i++)
         {
         if (strcmp (lista_arquivos[i], lista_arquivos[i+1]) > 0)
            {
            memset(tmpStr, 0, sizeof(tmpStr));
            memcpy(tmpStr, lista_arquivos[i], sizeof(tmpStr));
            memcpy(lista_arquivos[i], lista_arquivos[i+1], sizeof(tmpStr));
            memcpy(lista_arquivos[i+1], tmpStr, sizeof(tmpStr));
            inverteu = TRACK_TRUE;
            }
         }
      arquivos_ordenar--;
    }
   
   /*comeca a leitura dos arquivos e o tracking/previsao*/
   for (i = 0; i < total_arquivos; i++)
      {
      if (TRACK_TRUE == primeira_imagem)
         {
         /*leitura da primeira imagem*/
         if (TRACK_OK == le_arquivo_imagem(img1, &parametros,
                                           lista_arquivos[i]))
            {
            /*imagem lida com sucesso - limpa pixeis isolados e identifica os clusters*/
            if (TRACK_ERR == numera_cluster(img1, img_cluster1, &parametros))
               {
               limpa_memoria();
               return TRACK_ERR;
               }            
            /*remove clusters pequenos e calcula estatisticas*/
            if (TRACK_ERR == calcula_parametros(img1, img_cluster1, cluster1,
                                                &numcluster_img1, &parametros))
               {
               limpa_memoria();
               return TRACK_ERR;
               }
            /*mostra na tela as informacoes dos sistemas*/
            display_info(cluster1, numcluster_img1, &parametros);

            /*grava o arquivo com os clusters identificados e zipa*/
            grava_arquivo_clusters(img_cluster1, &parametros);

            /*guarda o nome da primeira imagem*/
            memset(nome_anterior, 0, sizeof(nome_anterior));
            sprintf(nome_anterior, "%s", lista_arquivos[i]);
            primeira_imagem = TRACK_FALSE;
            }
         }
      else
         {
         /*le a sequencia de imagem*/
         if (TRACK_OK == le_arquivo_imagem(img2, &parametros,
                                           lista_arquivos[i]))
            {
            /*verifica se a diferenca de tempo entre essa imagem
            e a anterior eh aceitavel*/
            strcpy(parametros.nome_arquivo_anterior, nome_anterior);
            if (TRACK_ERR == verifica_intervalo(nome_anterior,
                                                lista_arquivos[i],
                                                parametros.t_maximo,
                                                &intervalo))
               {
               /*intervalo eh maior que o intervalo informado
               transforma essa imagem na primeira imagem, caso nao seja a
               ultima imagem. Se for, nao vai gerar a previsao.*/
               if (i != (total_arquivos - 1))
                  {
                  i--;
                  primeira_imagem = TRACK_TRUE;
                  continue;
                  }
               else
                  {
                  limpa_memoria();
                  return TRACK_ERR;
                  }
               }
            parametros.intervalo_atual = intervalo;
            /*imagem lida com sucesso - limpa pixeis isolados
            e identifica os clusters*/
            if (TRACK_ERR == numera_cluster(img2, img_cluster2, &parametros))
               {
               limpa_memoria();
               return TRACK_ERR;
               }
            /*remove clusters pequenos e calcula estatisticas*/
            if (TRACK_ERR == calcula_parametros(img2, img_cluster2, cluster2,
                                                &numcluster_img2, &parametros))
               {
               limpa_memoria();
               return TRACK_ERR;
               }            
            /*faz a o calculo dos deslocamentos e tipo de sistema
            e gera img sintetica da previsao caso seja a ultima imagem*/
            if (i == (total_arquivos - 1))
               {
               if (TRACK_ERR == faz_previsao(img_cluster1, img_cluster2,
                                             cluster1, cluster2, img2,
                                             &parametros, numcluster_img1,
                                             numcluster_img2, TRACK_TRUE))
                  {
                  limpa_memoria();
                  return TRACK_ERR;
                  }
               }
            else
               {
               if (TRACK_ERR == faz_previsao(img_cluster1, img_cluster2,
                                             cluster1, cluster2, img2,
                                             &parametros, numcluster_img1,
                                             numcluster_img2, TRACK_FALSE))
                  {
                  limpa_memoria();
                  return TRACK_ERR;
                  }
               }
            /*mostra na tela ou no arquivo ou nos 2 as
            informacoes dos sistemas*/
            display_info(cluster2, numcluster_img2, &parametros);
            /*copia os dados da imagem 2 pra imagem 1*/
            numcluster_img1 = numcluster_img2;            
            memcpy(img1, img2, parametros.nx*parametros.ny*sizeof(short int));
            memcpy(img_cluster1, img_cluster2,
                   parametros.nx*parametros.ny*sizeof(unsigned int));
            memcpy(cluster1, cluster2, MAX_CLUSTER*sizeof(info_sistema));

            /*grava o arquivo com os clusters identificados e zipa*/
            grava_arquivo_clusters(img_cluster2, &parametros);

            /*limpa as variaveis para a proxima rodada*/
            memset(img2, 0, parametros.nx*parametros.ny*sizeof(short int));
            memset(img_cluster2, 0,
                   parametros.nx*parametros.ny*sizeof(unsigned int));
            memset(cluster2, 0, MAX_CLUSTER*sizeof(info_sistema));
            /*guarda o nome dessa imagem*/
            memset(nome_anterior, 0, sizeof(nome_anterior));
            sprintf(nome_anterior, "%s", lista_arquivos[i]);
            }
         }
      }
   limpa_memoria();      
   return TRACK_OK;
   
   }

void limpa_memoria(void)
   {
   free(img1);
   free(img2);
   free(img_cluster1);
   free(img_cluster2);
   free(cluster1);
   free(cluster2);
   free(saida);
   }
