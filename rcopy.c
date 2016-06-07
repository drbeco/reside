/*--------------------------------------------------------------------------*
 |                                                                          |
 |          RRRRRR    CCCCCC   OOOOOO   PPPPPP  YY    YY     CCCCCC         |
 |          RR   RR  CC       OO    OO  PP   PP  YY  YY     CC              |
 |          RRRRRR   CC       OO    OO  PPPPPP     YY       CC              |
 |          RR   RR  CC       OO    OO  PP         YY       CC              |
 |          RR   RR   CCCCCC   OOOOOO   PP         YY    ::  CCCCCC         |
 |                                                                          |
 |                       Copiador Residente de Arquivos                     |
 |                Exemplo de utilizacao da Biblioteca MULTITAR.C            |
 |                                                                          |
 *--------------------------------------------------------------------------*/

#include <dos.h>
#include <stdio.h>
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#include "multitar.h"          /* HEADER da biblioteca MULTITAR.C    */

#define TAM_BUFF 1024                  /* Tamanho do buffer de E/S   */

unsigned _heaplen = 1024;              /* Tamanho da HEAP e da PILHA */
unsigned _stklen  = 1024;

Boolean instalado= False;

struct stnomes {                       /* Estrutura do nome dos arquivos */
  unsigned char masc_ent [81];
  unsigned char masc_sai [81];
  unsigned char arq_ent  [81];
  unsigned char arq_sai  [81];
} nomes;

Byte buffer  [TAM_BUFF];               /* Buffer de E/S               */
int erro;                              /* Se = 6  Cancela execucao    */
                                       /* Se = 7  Retira da memoria   */
void interrupt (*velha_int_14h)();     /* Interrupcao de intercambio  */
struct REGPACK regs;

#define MSG_INST "\nPrograma instalado na memoria !"

char *dir_atual (char *path,int drive);
int tam_path (char *masc);
void faz_nome_cheio (char *string);
void faz_nome_sai (char *mascara, char *nome_ent, char *nome_sai);
int copia_arquivo ();
void interrupt int_14h ();
Boolean esta_residente();
Boolean pega_nomes();
Boolean passa_nomes();
void cancela_processo();
void fim_execucao();
void mostra_execucao();
void loop_execucao();

/*----------------------------------------------------------------------------
  Funcao Principal - MAIN
 ---------------------------------------------------------------------------*/

main (argc,argv)
  int argc;
  char *argv[];
{
  int i;

  velha_int_14h = getvect (0x14);
  nomes.masc_ent [0] = 0;
  instalado = esta_residente();

  if (argc > 1) {
    strcpy (nomes.masc_ent,argv[1]);
    strcpy (nomes.masc_sai,argv[2]);
    faz_nome_cheio (nomes.masc_ent);
    faz_nome_cheio (nomes.masc_sai);
    if (instalado) {
      if (argv[1][1] == 0) {
        switch (toupper (argv[1][0])) {
          case 'C' : printf ("Execucao corrente CANCELADA !\n");
                     cancela_processo();
                     break;
          case 'R' : printf ("Programa RETIRADO da Memoria !\n");
                     fim_execucao();
                     break;
        }
      }
      else {
        if (passa_nomes()) {
          mostra_execucao();
          printf ("\nParametros nao podem ser alterados !\n");
        }
        else mostra_execucao();
      }
    }
    else {
      mostra_execucao();
      printf (MSG_INST);
      velha_int_14h = getvect (0x14);
      setvect (0x14,int_14h);
      fica_residente();
      loop_execucao();
    }
  }
  else
    if (instalado) {
      pega_nomes();
      mostra_execucao();
    }
    else {
      printf (MSG_INST);
      printf ("\nParametro : RCOPY entrada saida\n");
      setvect (0x14,int_14h);
      fica_residente();
      loop_execucao();
    }
}

/*----------------------------------------------------------------------------
  DIR_ATUAL - Pega o diretorio corrente na forma D:\PATH\
 ---------------------------------------------------------------------------*/

char *dir_atual (char *path,int drive)
{
  int i;
  path [0] = 'A'- 1 + drive;
  path [1] = ':';
  path [2] = '\\';
  getcurdir (drive, path+3);
  if ((i=strlen (path+3)) > 0) {
    path [i+3] = '\\';
    path [i+4] = 0;
  }
  return (path);
}

/*----------------------------------------------------------------------------
  TAM_PATH - Retorna o tamanho do PATH do nome do arquivo
 ---------------------------------------------------------------------------*/

int tam_path (char *masc)
{
  int len_p, i;

  len_p = strlen (masc);
  for (i=len_p; i >=0; i--)
    if (masc [i] == '\\')
      return (i+1);
  return (0);
}

/*----------------------------------------------------------------------------
  FAZ_NOME_CHEIO - Coloca drive, caminho e mascara no nome do arquivo
 ---------------------------------------------------------------------------*/

void faz_nome_cheio (char *string)
{
  char drive [3], nome[13], ext [5];
  char linha [81];
  int  i;

  if ((string [0] == '.') && (string[1] == 0))
    string [0] = 0;

  strcpy (linha, strupr (string));
  fnsplit (linha, drive , string+2, nome, ext);
  if (drive [0] == 0)
    string [0] = getdisk() + 'A';
  else string [0] = drive [0];
  string [1] = ':';
  if (string [2] == 0)
    dir_atual (string, string [0] - 'A' + 1);
  else
    if (string [2] != '\\') {
      strcpy (linha, string+2);
      dir_atual (string, string [0] - 'A' + 1);
      strcat (string, linha);
    }

  if ((nome [0] == 0) && (ext [0] == 0))
    strcpy (nome,"*.*");
  else
    strcat (nome,ext);
  strcat (string, nome);
}

/*----------------------------------------------------------------------------
  FAZ_NOME_SAI - Faz nome do arquivo de saida em funcao da mascara e entrada.
 ---------------------------------------------------------------------------*/

void faz_nome_sai (char *mascara, char *nome_ent, char *nome_sai)
{
  int i,j;
  char atual,linha[12];
  static struct fcb fcb_ent;

  parsfnm (nome_ent+tam_path(nome_ent),&fcb_ent,0);
  memmove (linha,fcb_ent.fcb_name,11);
  linha [11] = 0;

  parsfnm (mascara+tam_path(mascara),&fcb_ent,0);
  for (i=j=0; i < 11; i++) {
    atual = fcb_ent.fcb_name [i];
    if ((atual == '?') && (linha [i] != ' '))
      nome_sai [j++] = linha [i];
    else
      if ((atual != '?') && (atual != ' '))
        nome_sai [j++] = atual;
    if (i==7)
      nome_sai [j++] = '.';
  }
  nome_sai [j] = 0;
}

/*----------------------------------------------------------------------------
  COPIA_ARQUIVO - Copia o arquivo dado em arq_ent para arq_sai
 ---------------------------------------------------------------------------*/

int copia_arquivo ()
{
  int handle_ent, handle_sai, leu;
  static struct ftime dharq;

  erro = 0;
  if ((handle_ent = _open (nomes.arq_ent,O_RDONLY)) != -1) {
    if ((handle_sai = _creat (nomes.arq_sai,0)) != -1) {
      do {
        delay (60);
        if ((leu = _read (handle_ent,buffer,TAM_BUFF)) == -1)
          erro = 3;
        else {
          delay (60);
          if (_write (handle_sai,buffer,leu) == -1)
            erro = 4;
        }
      } while ((!erro) && (leu != 0));
      if (!erro) {
        getftime (handle_ent,&dharq);
        setftime (handle_sai,&dharq);
      }
      _close (handle_sai);
    }
    else erro = 2;
    _close (handle_ent);
  }
  else erro = 1;
  return (erro);
}

/*----------------------------------------------------------------------------
  INT_14H - Interrupcao para intercambio de informacoes entre processos
 ---------------------------------------------------------------------------*/

static void interrupt int_14h (bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flgs)
{
  switch ((ax >> 8) & 0xff) {
    case 0x90 : ax |= 0xff00;              /* Residente Instalado */
                break;
    case 0x91 : erro = 6;                  /* Manda cancelar */
                break;
    case 0x92 : if (nomes.masc_ent[0] != 0) { /* Retorna endereco de NOMES */
                  es = _DS;
                  bx = (unsigned int)&nomes;
                  ax = 0;
                }
                else ax = 1;
                break;
    case 0x93 : if (nomes.masc_ent[0] == 0) { /* Inicializa NOMES       */
                  movedata (ds,dx,_DS,&nomes,sizeof(struct stnomes));
                  ax = 0;
                }
                else ax = 1;
                break;
    case 0x94 : erro = 7;                  /* Retira programa da memoria */
                break;
    default   : _AX = ax; _DX = dx;        /* Se funcao nao reconhecida */
                (*velha_int_14h)();        /* chama a antiga INT 14h    */
                ax = _AX;                  /* Retorne em AX o valor dado*/
  }                                        /* pela int 14h original     */
}

/*----------------------------------------------------------------------------
  ESTA_RESIDENTE - Informa se o programa ja esta instalado
 ---------------------------------------------------------------------------*/
Boolean esta_residente()
{
  regs.r_ax = 0x9000;
  intr (0x14,&regs);
  return (((regs.r_ax & 0xff00) == 0xff00) ? True : False);
}

/*----------------------------------------------------------------------------
  PEGA_NOMES - Pega estrutura NOMES do processo residente
 ---------------------------------------------------------------------------*/
Boolean pega_nomes()
{
  regs.r_ax = 0x9200;
  intr (0x14,&regs);
  if (regs.r_ax == 0)
    movedata (regs.r_es,regs.r_bx,_DS,&nomes,sizeof(struct stnomes));
  return (regs.r_ax & 0x1);
}

/*----------------------------------------------------------------------------
  PASSA_NOMES - Passa estrutura NOMES para o processo residente
 ---------------------------------------------------------------------------*/

Boolean passa_nomes()
{
  regs.r_ax = 0x9300;
  regs.r_ds = _DS;
  regs.r_dx = (unsigned int) &nomes;
  intr (0x14,&regs);
  return (regs.r_ax);
}

/*----------------------------------------------------------------------------
  CALCELA_PROCESSO - Pede ao processo residente p/ cancelar a copia corrente
 ---------------------------------------------------------------------------*/
void cancela_processo()
{
  regs.r_ax = 0x9100;
  intr (0x14,&regs);
}

/*----------------------------------------------------------------------------
  FIM_EXECUCAO - Termina processo que esta na memoria
 ---------------------------------------------------------------------------*/
void fim_execucao()
{
  regs.r_ax = 0x9400;  /* Termina processo residente   */
  intr (0x14,&regs);
  exit (0);            /* e finaliza processo corrente */
}

/*----------------------------------------------------------------------------
  MOSTRA_EXECUCAO - Mostra na tela a copia corrente que esta sendo executada
 ---------------------------------------------------------------------------*/
void mostra_execucao()
{
  if (nomes.masc_ent [0] == 0)
    printf ("Nenhuma copia esta sendo executada !\n");
  else {
    printf ("Copia sendo executada:\n\n");
    printf ("Mascara de Entrada : %s\n",nomes.masc_ent);
    printf ("Mascara de Saida   : %s\n",nomes.masc_sai);
    printf ("Arquivo de Entrada : %s\n",nomes.arq_ent);
    printf ("Arquivo de Saida   : %s\n",nomes.arq_sai);
  }
}

/*----------------------------------------------------------------------------
  LOOP_EXECUCAO - Loop principal de execucao
 ---------------------------------------------------------------------------*/
void loop_execucao()
{
  static struct ffblk fcb_pesq;
  int retorno;

  for (;;) {
    while (nomes.masc_ent[0] == 0) {    /* Enquanto nao ha nada a fazer */
      if (erro == 7) {                  /* Pediu para retirar ?         */
        setvect (0x14,velha_int_14h);   /* entao retira e volta         */
        retira_residente();
      }                                 /* Volta ao transiente, porque    */
      else volta_transiente();          /* nao ha nada a fazer no momento */
    }
    erro = 0;
    retorno = findfirst (nomes.masc_ent,&fcb_pesq,0);
    while (retorno == 0) {
      memcpy (nomes.arq_ent,nomes.masc_ent,tam_path(nomes.masc_ent));
      strcpy (nomes.arq_ent+tam_path(nomes.masc_ent),fcb_pesq.ff_name);
      memcpy (nomes.arq_sai,nomes.masc_sai,tam_path(nomes.masc_sai));
      faz_nome_sai (nomes.masc_sai,nomes.arq_ent,
                    nomes.arq_sai+tam_path(nomes.masc_sai));
      if (strcmp(nomes.arq_ent,nomes.arq_sai))
        copia_arquivo ();
      retorno = (erro >= 6) ? 1 : findnext (&fcb_pesq);
    }
    sound (800); delay (50); nosound();  /* Avisa que acabou */
    nomes.masc_ent [0] = 0;
  }
}

