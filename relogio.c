/*----------------------------------------------------------------------*
 | Programa  : R E L O G I O . C                                        |
 |                                                                      |
 | Funcao    : Liga um relogio no canto superior direito da tela, exem- |
 |             plificando a utilizacao da interrupcao de relogio.       |
 |                                                                      |
 | Linguagem : Turbo C, versao 1.5 ou superior                          |
 |                                                                      |
 *----------------------------------------------------------------------*/
#include <dos.h>

unsigned _heaplen = 1;              /* Define a area utilizada pelo C */
unsigned _stklen = 256;

unsigned long int contador;         /* Contador de relogio do BIOS    */
unsigned char vetor [8];            /* Vetor para colocacao da hora   */
unsigned long int hora,             /* Operadores de tempo            */
                  minuto,
                  segundo;
unsigned long int resto,            /* Variaveis auxiliares           */
                  ant;
unsigned seg_tela;                  /* Segmento de dados da tela      */
void interrupt (*velha_int_08h)();  /* Interrupcao de relogio         */

/*---------------------------------------------------------------------------
  MOSTRA_HORA - Coloca na tela a hora corrente
 ---------------------------------------------------------------------------*/
void mostra_hora()
{
  int i;

  for (i=0; i < 8; i++)
    poke (seg_tela,144 + (i * 2), vetor[i] | 0x7000);
}

/*---------------------------------------------------------------------------
  BINDEC - Converte valor binario em decimal ASCII de 2 digitos
 ---------------------------------------------------------------------------*/
void bindec (unsigned char valor, char *vetor)
{
  int aux;
  aux = valor / 10;
  vetor [0] = aux + '0';
  vetor [1] = (valor % 10) + '0';
}

/*---------------------------------------------------------------------------
  INT_08H - Interrupcao de relogio desviada
 ---------------------------------------------------------------------------*/
void interrupt int_08h()
{
  (*velha_int_08h)();
  contador = *(unsigned long int far *) MK_FP (0,0x046C);
  hora = contador / 65543;
  resto = contador % 65543;
  minuto = resto / 1092;
  resto %= 1092;
  segundo = (resto * 100) / 1821;
  if (ant != segundo)  {           /* Se alterou os segundos, entao */
    ant = segundo;                 /* mostra a hora na tela         */
    bindec (hora,vetor);
    bindec (minuto,vetor+3);
    bindec (segundo,vetor+6);
    mostra_hora();
  }
}

/*---------------------------------------------------------------------------
  MAIN - Funcao principal
 ---------------------------------------------------------------------------*/
main()
{
  vetor [2] = ':';
  vetor [5] = ':';

  if ((peek (0,0x0410) & 0x30) == 0x30)
    seg_tela = 0xb000;
  else
    seg_tela = 0xb800;

  velha_int_08h = getvect (0x8);
  setvect (0x8,int_08h);

  keep (0,_SS + (_SP / 16) - _psp + 10);   /* Fica residente */
}

