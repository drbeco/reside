/*----------------------------------------------------------------------*
 | Programa  : S C A N . C                                              |
 |                                                                      |
 | Funcao    : Desvia a interrupcao de teclado, e mostra os codigos     |
 |             (SCAN CODE) de cada tecla pressiona, bem como as te-     |
 |             clas de SHIFT. O programa termina com  o  pressiona-     |
 |             mento da tecla ESC.                                      |
 |                                                                      |
 | Linguagem : Turbo C, versao 1.5 ou superior                          |
 |                                                                      |
 *----------------------------------------------------------------------*/
#include <dos.h>                      /* Biblioteca do DOS     */
#include <conio.h>                    /* Biblioteca da Console */

unsigned char finalizou = 0;
void interrupt (*velha_int_09h)();    /* Guarda endereco da antiga INT 9h   */
unsigned char scan;                   /* Guarda SCAN CODE da tecal pression.*/
unsigned char aux;                    /* Variavel Auxiliar                  */
unsigned int far *SHIFTS;             /* Ponteiro para area do BIOS         */

/*---------------------------------------------------------------------------
  INT_09H - Interrupcao de teclado
 ---------------------------------------------------------------------------*/
void interrupt int_09h()
{
  if ((inportb(0x60) & 0x80) == 0)  /* Se pressionamento de uma Tecla,     */
    scan = inportb (0x60);          /* entao guarda codigo em scan         */

  if (scan == 1)                    /* SCAN CODE da Tecla ESC ?            */
    finalizou = 1;                  /* sim, avisa finalizacao              */

  aux = inportb (0x61);             /* Liga Bit 7 de reconhecimento        */
  outportb (0x61,aux | 0x80);       /* e torna a desligar, avisando ao con-*/
  outportb (0x61,aux);              /* trolador de teclado o reconhecimento*/
  outportb (0x20,0x20);             /* Fim de Interrupcao de Hardware(8259)*/
}

/*---------------------------------------------------------------------------
  MAIN - Funcao Principal
 ---------------------------------------------------------------------------*/
main()
{
  textattr (7);
  clrscr();
  cprintf ("Digite ESC para finalizar");

  velha_int_09h = getvect (9);       /* Pega endereco da Int 9 original     */
  setvect (9,int_09h);               /* Seta novo endereco da interrupcao   */

  SHIFTS = (unsigned int far *) MK_FP (0x0000,0x0417);

  do {
    textattr (7);
    gotoxy (1,3);
    cprintf ("SCAN = %4u",scan);
    gotoxy (1,5);
    cprintf ("ShiftDir  ShiftEsq  CTRL  ALT");
    textattr (112);

    if ((*SHIFTS & 1) != 0) {       /* Mostra Teclas de Shift */
      gotoxy (1,5);
      cprintf ("ShiftDir");
    }

    if ((*SHIFTS & 2) != 0) {
      gotoxy (11,5);
      cprintf ("ShiftEsq");
    }

    if ((*SHIFTS & 4) != 0) {
      gotoxy (21,5);
      cprintf ("CTRL");
    }

    if ((*SHIFTS & 8) != 0) {
      gotoxy (27,5);
      cprintf ("ALT");
    }
  } while (!finalizou);             /* Aguarda pressionamento do ESC       */

  gotoxy (1,8);
  cprintf ("Fim de Execucao. Tecla ESC pressionada !!!!");

  setvect (9,velha_int_09h);        /* Restaura Int 9 original             */

