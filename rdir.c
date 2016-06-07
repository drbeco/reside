/*--------------------------------------------------------------------------*
 |                                                                          |
 |                 RRRRRR   DDDDDD   II  RRRRRR       CCCCCC                |
 |                 RR   RR  DD   DD  II  RR   RR     CC                     |
 |                 RRRRRR   DD   DD  II  RRRRRR      CC                     |
 |                 RR   RR  DD   DD  II  RR   RR     CC                     |
 |                 RR   RR  DDDDDD   II  RR   RR  ::  CCCCCC                |
 |                                                                          |
 |                                                                          |
 |     Diretorio Residente - Exemplo de utilizacao da Biblioteca RESIDE.C   |
 |                                                                          |
 *--------------------------------------------------------------------------*/

#include <conio.h>            /* As rotinas da Console devem ser da CONIO.H */
#include <dos.h>
#include <dir.h>
#include <string.h>
#include "reside.h"           /* HEADER da biblioteca RESIDE.C              */

/* Teclas interpretadas pela rotina de leitura do teclado (edita_linha)     */

#define T_ENTER   13
#define T_ESC     27
#define T_BKS     8
#define T_SESQ    (256+75)
#define T_SDIR    (256+77)
#define T_SCIMA   (256+72)
#define T_SBAIXO  (256+80)
#define T_CTREND  (256+117)
#define T_INS     (256+82)
#define T_DEL     (256+83)
#define T_END     (256+79)
#define T_HOME    (256+71)
#define T_CTREND  (256+117)

unsigned _heaplen = 1;           /* Nao esta usando alocacao dinamica     */
unsigned _stklen = 2048;         /* Tamanho da pilha (default=4k)         */

struct ffblk fcb;                /* FCB para pesquisa de arquivos         */
char mascara[80]="";             /* Mascara dos arquivos a serem mostrados*/

void escreve (char *linha);
int le_tecla();
int edita_linha (int x,int y,char *linha, int max);

/*--------------------------------------------------------------------------
  Funcao Principal - MAIN
 -------------------------------------------------------------------------*/
main ()
{
  if (ja_instalado()) {
    escreve ("Programa ja instalado na memoria !");
    exit (1);
  }

  escreve ("Ficou Residente...Tecle ALT-ESC");
  fica_residente();
  clrscr();

  for (;;) {
    gotoxy (1,wherey());
    cprintf ("\n\nTecle ALT-ESC para retornar ou"
             "\n\rDigite FIM para retirar o residente da memoria.\n\r"
             "\n\rDiretorio : ");
    edita_linha (wherex(),wherey(),mascara,80);
    if ((strcmp (mascara,"fim") == 0) ||
        (strcmp (mascara,"FIM") == 0))     /* Se mascara == FIM, entao   */
      retira_residente();                  /* retira programa da memoria */
    cprintf ("\n\n");
    gotoxy (1,wherey());
    if (!findfirst (mascara,&fcb,0))
      do
        cprintf ("%-16s",fcb.ff_name);
      while (!findnext (&fcb));
  }
}
/*--------------------------------------------------------------------------
  ESCREVE - Escreve uma linha (string) no video
 -------------------------------------------------------------------------*/

void escreve (char *linha)
{
  char *pont;

  for (pont=linha-1;*++pont;)
    putch (*pont);
}

/*--------------------------------------------------------------------------
  LE_TECLA - Le uma tecla da console, sem chamar o DOS
 -------------------------------------------------------------------------*/

int le_tecla()
{
  int tecla;

  tecla = bioskey (0);             /* Le tecla atraves do BIOS */
  if ((tecla & 0xff) == 0)         /* Se tecla especial do PC  */
    tecla = (tecla >> 8) + 256;    /* soma 256 ao seu valor    */
  else
    tecla &= 0xff;
  return (tecla);                  /* Retorna codigo da tecla  */
}

/*--------------------------------------------------------------------------
  EDITA_LINHA - Edita uma linha na console, sem chamar o DOS
                X, Y - Coordenadas do video (X - Coluna, Y - Linha).
                linha- String a ser editada.
                max  - Tamanho maximo da linha.
 -------------------------------------------------------------------------*/
int edita_linha (int x,int y,char *linha, int max)
{
  int tam_atual,i;
  int tecla;
  static unsigned char insere = 0;
  unsigned char fim = 0;
  int index = 0;

  tam_atual = strlen (linha);
  gotoxy (x,y);
  escreve (linha);
  gotoxy (x,y);

  do {
    tecla = le_tecla();
    if ((tecla >= 32) && (tecla < 127)) {
      if ((insere) && (tam_atual < max)) {
        for (i=tam_atual; i > index; i--)
          linha [i] = linha [i-1];
        linha[++tam_atual] = 0;
        gotoxy (x,y);
        escreve (linha);
        gotoxy (x+index,y);
      }
      if (index < max) {
        linha [index++] = tecla;
        if (index > tam_atual) {
          linha [index] = 0;
          tam_atual++;
        }
        putch (tecla);
      }
    }
    else
      switch (tecla) {
        case T_DEL:                     /* Deleta caractere a direita      */
          if (index < tam_atual) {
            strcpy (linha+index,linha+index+1);
            tam_atual--;
            escreve (linha+index);
            putch (' ');
            gotoxy (x+index,y);
          }
          break;
        case T_BKS:                     /* Deleta a esquerda e volta cursor */
          if (index) {
            strcpy (linha+index-1,linha+index);
            tam_atual--;
            index--;
            gotoxy (x+index,y);
            escreve (linha+index);
            putch (' ');
            gotoxy (x+index,y);
          }
          break;
        case T_SESQ:                    /* Volta cursor uma posicao  */
          if (index > 0) {
            index--;
            gotoxy (x+index,y);
          }
          break;
        case T_SDIR:                    /* Avanca cursor uma posicao */
          if (index < tam_atual) {
            index++;
            gotoxy (x+index,y);
          }
          break;
        case T_INS:                     /* Ativa modo de insercao    */
          insere ^= 1;
          break;
        case T_END:                     /* Vai para o final da linha */
          index = tam_atual;
          gotoxy (x+index,y);
          break;
        case T_HOME:                    /* Vai para o inicio da linha */
          index = 0;
          gotoxy (x+index,y);
          break;
        case T_CTREND:                  /* Apaga ate o final da linha */
          linha [tam_atual=index] = 0;
          gotoxy (x,y);
          escreve (linha);
          for (i=0; i < max-tam_atual; i++)
            putch (' ');
          gotoxy (x+index,y);
          break;
        case T_ENTER:                   /* ENTER ou ESC finaliza      */
        case T_ESC  :
          fim = 1;
      }
   } while (!fim);
   return (index);
}

