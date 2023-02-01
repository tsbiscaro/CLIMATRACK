#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dirent.h>
#include <unistd.h>

#define TRACK_ERR 1
#define TRACK_OK 0
#define TRACK_TRUE 1
#define TRACK_FALSE 0
#define UNDEF_CLUSTER 0
#define MAX_CLUSTER 999999
#define MAX_CLUSTER_CLEAN 10000
#define MAX_MERGE 20
#define MAX_IMAGENS 10000
#define IMGSATELITE 0
#define IMGRADAR 1

#define MAX_DIRETORIO 256
#define MAX_TEXTO 256

#define TIPO_SISTEMA_NOVO 0
#define TIPO_SISTEMA_MERGE TIPO_SISTEMA_NOVO + 2
#define TIPO_SISTEMA_SPLIT TIPO_SISTEMA_NOVO + 3
#define TIPO_SISTEMA_CONTINUACAO TIPO_SISTEMA_NOVO + 4

/*Um limite de 160 dBZ ou 160 K para indicar dado invalido, ja que
podem aparecer valores -32767 ou 32768 nos dados*/
#define UNDEF_FISICO 16000

/*estrutura de informacoes de cada sistema identificado*/
typedef struct INFO_SISTEMA
   {
   /*posicao do sistema*/
   unsigned short int x_centro;
   unsigned short int y_centro;
   /*
   indice desse sistema na imagem anterior (0 para novos sistemas)
   eh um array porque se o sistema detectado eh um merge, entao ele pode
   ser originado de diversos sistemas. Preencher com 0 os indices nao
   utilizados
   */
   unsigned int ind_img_ant[MAX_MERGE];
   /*numero de pixeis desse cluster*/
   unsigned int total_pix;
   /*
   temp ou refl media, desvio padrao,
   maior dBZ ou menor Tb, area,
   desclocamento
   */
   float valor_medio;
   float valor_extremo;
   float velocidade;
   float direcao;
   /*0 = mesmo sistema, 1 = merge, 2 = split, 3 = continuacao */
   unsigned char estado_separacao;
   /*numero de pixeis que o sistema esta aumentando (ou diminuindo)*/
   int expansao;
   /*passou ou nao sobre o t3*/
   int over_t3;
   } info_sistema;


typedef struct PARAMS
   {
   /*caminho dos arquivos*/
   char dir_entrada_imagens[MAX_DIRETORIO];
   char dir_saida_previsao[MAX_DIRETORIO];
   char dir_saida_estatisticas[MAX_DIRETORIO];
   char nome_arquivo_atual[MAX_TEXTO];
   char nome_arquivo_anterior[MAX_TEXTO];
   /*tipo da imagem IMGRADAR ou IMGSATELITE */
   char tipo_imagem;
   /*tamanho minimo em pixeis para deteccao*/
   unsigned short int tamanho_minimo;
   /*limite de pixeis invalidos (porcentagem), limite de linhas consecutivas invalidas*/
   unsigned short int pct_pixeis_invalidos;
   unsigned short int num_linhas_invalidas_consecutiva;
   /*0 = graus lat/lon, 1 = metros*/
   unsigned short int unidade_espacamento;
   /*numero de colunas e linhas da imagem, espacamento da imagem, canto da imagem*/
   unsigned short int nx;
   unsigned short int ny;
   float dx;
   float dy;
   float canto_nw_x;
   float canto_nw_y;
   /*limiar de deteccao (Tb max ou dBZ min)*/
   float limiar;
   /*passo de tempo para previsao*/
   unsigned long deltat;
   /*numero de previsoes a gerar*/
   unsigned short int num_prev;
   /*tempo maximo aceitavel entre 2 img*/
   unsigned short int t_maximo;
   /*intervalo de tempo entre as imagens atuais (em minutos)*/
   unsigned short int intervalo_atual;
   /*valor de sobreposicao para considerar o mesmo cluster em 2 imagens (%)*/
   unsigned short int porcentagem_sobreposicao;
   /*valor para dados invalidos*/
   short int UNDEF_DATA;
   /*tipo de display de informacao dos clusters
   0 = nenhuma
   1 = info na tela
   2 = info no arquivo
   3 = info na tela e no arquivo*/
   short int display;
   } params;



void numera_cluster_radar(short int *in, unsigned int *out,
                          int i, int j, unsigned int num_cluster,
                          params *parametros);
void numera_cluster_satelite(short int *in, unsigned int *out,
                             int i, int j, unsigned int num_cluster,
                             params *parametros);

int numera_cluster(short int *in, unsigned int *out, params *parametros);

int calcula_parametros (short int *dados, unsigned int *in,
                        info_sistema *clusters, unsigned int *max_cluster,
                        params *parametros);

int faz_previsao(unsigned int *img_cluster1, unsigned int *img_cluster2,
                 info_sistema *cluster1, info_sistema *cluster2,
                 short int *imagem, params *parametros,
                 unsigned int numcluster1, unsigned int numcluster2,
                 char gera_img_previsao);


int le_parametros(params *parametros, char *arquivo);

int le_arquivo_imagem(short int *img, params *parametros, char *nome);


void erro(char *texto, char *arquivo, int linha);

void limpa_memoria(void);

int verifica_intervalo(char *nome_img1, char *nome_img2,
                       unsigned short int intervalo_max,
                       float *intervalo);

void print_info(info_sistema *clusters,
                unsigned int max_cluster, params *parametros);

void fprint_info(info_sistema *clusters,
                 unsigned int max_cluster, params *parametros);

void display_info(info_sistema *clusters, unsigned int max_cluster,
                  params *parametros);

void calcula_deslocamento(info_sistema *cluster1, info_sistema *cluster2,
                          params *parametros, unsigned int numcluster);

void preenche_imagem_sintetica(short int *imagem_sintetica, short int *imagem,
                               unsigned int *img_cluster2, info_sistema *cluster2,
                               int total_cluster, params *parametros, int deltat);
   
void preenche_borda(short int *imagem_sintetica, int x, int y,
                    params *parametros, int add, float valor);

void novo_nome(char *nome_entrada, char *nome_saida, int deltat);

void grava_arquivo_clusters(unsigned int *img_cluster, params *parametros);

int verifica_arquivo_imagem(short int *imagem, params *parametros);

void direcao_velocidade(info_sistema *cluster2, int cl,
                        params *parametros, float dx, float dy);

