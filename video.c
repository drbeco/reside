/*----------------------------------------------------------------------*
 | Programa  : V I D E O . C                                            |
 |                                                                      |
 | Funcao    : Liga ou Desliga o video atraves do pressionamento das    |
 |             teclas ALT ESC, exemplificando a manipulacao do video    |
 |             e da interrupcao de teclado (int 9h)                     |
 |                                                                      |
 | Linguagem : Turbo C, versao 1.5 ou superior                          |
 |                                                                      |
 *----------------------------------------------------------------------*/
#include <dos.h>

#define cga_video_on   0x29          /* Constantes assumidas pela porta */
#define cga_video_off  0x21          /* de video, de acordo com o       */
#define her_video_on   0x28          /* controlador                     */
#define her_video_off  0x20

unsigned _heaplen = 1;               /* Define areas iniciais do C      */
unsigned _stklen  = 128;

void interrupt (*velha_int_09h)();  /* Interrupcao de Teclado           */
unsigned char video_on = 1;         /* Indica Video ligado ou desligado */
unsigned char scan_tecla = 1;       /* Scan Code da tecla de acesso     */
unsigned char tecla_shift = 8;      /* Tecla de Shift de acesso         */
unsigned int  porta_video = 0x3d8;  /* Porta de video  (MSR)            */

unsigned char liga_video  = cga_video_on;
unsigned char desl_video  = cga_video_off;

unsigned char auxb;                 /* Byte auxiliar para int 9         */

/*--------------------------------------------------------------------------
  INT 09h - Interrupcao de Teclado
 --------------------------------------------------------------------------*/
void interrupt int_09h()
{
  if ((inportb(0x60) == scan_tecla) &&            /* HOT KEY pressionada ? */
      ((peekb (0x40,0x17) & tecla_shift) != 0)) {
    auxb = inportb(0x61);
    outportb (0x61,auxb | 0x80);                  /* Sim, avisa o reconhe- */
    outportb (0x61,auxb);                         /* cimento da tecla para */
    outportb (0x20,0x20);                         /* o controlador         */
    if (video_on) {                               /* Chaveia Video         */
      video_on = 0;
      outportb (porta_video,desl_video);
    }
    else {
      video_on = 1;
      outportb (porta_video,liga_video);
    }
  }
  else (*velha_int_09h)();                        /* Chama int Original    */
}

/*--------------------------------------------------------------------------
  MAIN - Funcao principal
 --------------------------------------------------------------------------*/
main()
{
  if ((peek (0,0x410) & 0x30) == 0x30) {  /* Video = HERCULES ? */
    porta_video = 0x3b8;                  /* sim, entao inicia  */
    liga_video = her_video_on;            /* variaveis          */
    desl_video = her_video_off;
  }
  velha_int_09h = getvect (0x09);         /* Desvia INT 0x9     */
  setvect (0x09,int_09h);
  keep (0,_SS + (_SP / 16) - _psp + 10);  /* Fica residente */
}

