/*----------------------------------------------------------------------*
 | Programa  : C L I C K . C                                            |
 |                                                                      |
 | Funcao    : Emite um sinal sonoro quando no pressionamento  de  uma  |
 |             tecla, exemplificando a utilizacao da INT 9h (Teclado).  |
 |                                                                      |
 | Linguagem : Turbo C, versao 1.5 ou superior                          |
 |                                                                      |
 *----------------------------------------------------------------------*/
#include <dos.h>
#include <conio.h>

unsigned _heaplen = 1;              /* Define a area utilizada pelo C   */
unsigned _stklen  = 128;

void interrupt (*velha_int_09h)();  /* Interrupcao de Teclado           */
int  i;                             /* Variavel auxiliar                */

/*---------------------------------------------------------------------------
  INT 09h - Interrupcao de Teclado
 ---------------------------------------------------------------------------*/
void interrupt int_09h()
{
   if ((inportb (0x60) & 0x80) == 0) {  /* Tecla pressionada ?  */
     sound (5000);                      /* Entao emite o CLICK  */
     for (i=0; i < 300; i++);
     nosound();
   }
   (*velha_int_09h)();                  /* Chama int 9 anterior */
}

/*---------------------------------------------------------------------------
  MAIN - Funcao Principal
 ---------------------------------------------------------------------------*/
main()
{
  velha_int_09h = getvect (0x09);       /* Desvia INT 0x9       */
  setvect (0x09,int_09h);
  keep (0,_SS + (_SP / 16) - _psp + 10);      /* Fica residente */
}
