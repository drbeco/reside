/*----------------------------------------------------------------------*
 | Programa  : P R I N T E R . C                                        |
 |                                                                      |
 | Funcao    : Filtra os codigos especiais do PC para impressora,       |
 |             exemplificando os programas residente ativados por       |
 |             interrupcao de Software, que neste caso e' a 17h.        |
 |                                                                      |
 | Linguagem : Turbo C, versao 1.5 ou superior                          |
 |                                                                      |
 *----------------------------------------------------------------------*/
#include <dos.h>

unsigned _heaplen = 1;              /* Define a area utilizada pelo C   */
unsigned _stklen  = 128;

void interrupt (*velha_int_17h)();  /* Interrupcao de Impressora        */

/* Novos Codigos especiais do PC de 128 a 218 ASCII */

char tab_esp_pc [91] =  "CueaaaaceeeiiiAAE  ooouuyOU     aiounNao?"
                        "          |++++++|+++++++++-++++++++-+-++"
                        "+++++++++";

unsigned char atual;    /* Variavel auxiliar */

/*---------------------------------------------------------------------------
  INT 17h - Interrupcao do BIOS para impressora
 ---------------------------------------------------------------------------*/
void interrupt int_17h (bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flgs)
{
  if (((ax >> 8) & 0xff) == 0) {       /* Funcao 0 chamada  ?    */
    atual = ax & 0xff;                 /* Pega caractere a ser   */
    if (atual > 127) {                 /* impresso. Se maior que */
      if (atual <= 218)                /* 127, entao converte    */
        atual = tab_esp_pc [atual - 128];
      else
        atual = ' ';
      ax = (ax & 0xff00) | atual;
    }
  }
  _AX = ax;                             /* Carrega registradores passados */
  _DX = dx;
  (*velha_int_17h)();                   /* Chama int original */
  ax = _AX;                             /* Restaura AX retornado pela 17h */
}

/*---------------------------------------------------------------------------
  MAIN - Funcao Principal
 ---------------------------------------------------------------------------*/
main()
{
  velha_int_17h = getvect (0x17);    /* Desvia INT 0x17      */
  setvect (0x17,int_17h);
  keep (0,_SS + (_SP / 16) - _psp + 10);   /* Fica residente */
}
