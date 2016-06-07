/*--------------------------------------------------------------------------*
 |                                                                          |
 |        MM   MM  UU   UU  LL     TTTTTT  II  TTTTTT  AAAAA   RRRRRR       |
 |        MMM MMM  UU   UU  LL       TT    II    TT   AA   AA  RR   RR      |
 |        MM M MM  UU   UU  LL       TT    II    TT   AAAAAAA  RRRRRR       |
 |        MM   MM  UU   UU  LL       TT    II    TT   AA   AA  RR   RR      |
 |        MM   MM   UUUUU   LLLLLLL  TT    II    TT   AA   AA  RR   RR      |
 |                                                                          |
 |                                                                          |
 |        Biblioteca de MultiTarefa para TURBOC versao 1.5 ou superior      |
 |                                                                          |
 *--------------------------------------------------------------------------*/
#include <dos.h>
#include <bios.h>

/* Definicao dos tipos de variaveis utilizadas */

#define Byte unsigned char
#define Word unsigned int
#define Boolean unsigned char
#define True 1
#define False 0

#define static

/*===================== Variaveis Globais de acesso =====================*/

Word contador = 10;      /* Contador de ciclos de ATIVACAO e DESATIVACAO */
Word t_ativ   = 4;       /* Numero de ciclos para ATIVAR o RESIDENTE     */
Word t_exec   = 1;       /* Numero de ciclos de execucao do RESIDENTE    */

/*===================== Variaveis Locais utilizadas =====================*/

/* Variaveis de Controle (Flags) */

static Boolean ativado      = False;  /* Indica se residente esta ativo  */
static Boolean bios_ocupado = False;  /* Indica se BIOS DISK ocupado     */
static Boolean esta_na_28h  = False;  /* Indica execucao da INT 28h      */
static Boolean muda_estado  = False;  /* Indica HOT KEY pressionada      */
static Boolean aguarda      = False;  /* Indica AGUARDO das int 8 e 28h  */

/* Enderecos do DOS_OCUPADO: Funcao 34h da Int 21h */

static Word segmento_dos = 0;         /* Segmento das variaveis do DOS   */
static Word dos_ocupado  = 0;         /* Endereco do flag DOS ocupado    */
static Byte guarda_dos   = 0;         /* Guarda DOS_OCUPADO na ativacao  */

/* Interrupcoes Alteradas */

static void interrupt (*velha_int_08h)();  /* Interrupcao de relogio     */
static void interrupt (*velha_int_13h)();  /* Interrupcao do BIOS DISK   */
static void interrupt (*velha_int_28h)();  /* Interrupcao de DOS OK      */

/* Interrupcoes Alteradas em Tempo de Ativacao */

static void interrupt (*velha_int_1bh)();  /* Interrupcao do CTRL-BREAK  */
static void interrupt (*velha_int_24h)();  /* Interrupcao Erro Critico   */

/* Area para salvamento do Contexto do DOS */

static Word psp_seg;                       /* Segmento do PSP corrente   */
static char far *dta;                      /* Endereco do DTA corrente   */

/* Area para salvamento dos registros da CPU */

static Word ss_res,   sp_res;              /* Pilha do residente         */
static Word ss_trans, sp_trans;            /* Pilha do transiente        */

/* Variaveis auxiliares de uso geral */

static Byte auxb;
static Word auxw;
static char far *auxp;
static Word flags_atual;
static void interrupt (*auxi)();

/* Registros de uso nas chamadas as interrupcoes */

static struct REGPACK reg;

/*===========================================================================
  INTERRUPCOES ALTERADAS NA ATIVACAO
 ===========================================================================*/

/*---------------------------------------------------------------------------
  INT 1Bh - Interrupcao do CTRL-BREAK
 ---------------------------------------------------------------------------*/
static void interrupt int_1bh()
{
   /* Nao faz nada. Apenas Cancela a acao do CTRL-BREAK */
}


/*---------------------------------------------------------------------------
   INT 24h - Interrupcao do erro critico
 ---------------------------------------------------------------------------*/
static void interrupt int_24h (bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flgs)
{
  ax = 0;                    /* Avisa ao dos para Ignorar o erro       */
  flags_atual = flgs;        /* Retorna em Flags_Atual o valor de flgs */
}


/*===========================================================================
  PROCESSOS DE ATIVACAO E DESATIVACAO
 ===========================================================================*/

/*---------------------------------------------------------------------------
  TROCA_CONTEXTO_DOS - Troca Contexto para o DOS
 ---------------------------------------------------------------------------*/
static void troca_contexto_dos()
{
  auxw = psp_seg;                           /* Troca endereco do PSP       */
  psp_seg = getpsp();                       /* corrente com o anterior     */
  reg.r_ax = 0x5000;
  reg.r_bx = auxw;
  intr (0x21,&reg);

  auxp = dta;                               /* Troca endereco do DTA       */
  dta = getdta();                           /* corrente com o anterior     */
  setdta (auxp);

  auxi = velha_int_1bh;                     /* Troca interrupcao de BREAK  */
  velha_int_1bh = getvect (0x1b);           /* corrente com a anterior     */
  setvect (0x1b,auxi);

  auxi = velha_int_24h;                     /* Troca interrupcao de BREAK  */
  velha_int_24h = getvect (0x24);           /* corrente com a anterior     */
  setvect (0x24,auxi);
}

/*---------------------------------------------------------------------------
  ATIVA - Ativa Programa Residente
 ---------------------------------------------------------------------------*/
static void ativa()
{
  aguarda=True;                            /* Pede para Int 8 e 28h aguardarem */

  ativado  = True;                      /* Avisa que esta ativado           */
  contador = t_exec;                    /* durante T_EXEC ciclos de execucao*/

  guarda_dos = peekb (segmento_dos,dos_ocupado);  /* Guarda estado do DOS   */

  ss_trans = _SS;                           /* Guarda pilha do Transiente   */
  sp_trans = _SP;

  disable();                                /* Desabilta interrupcoes       */
  _SS = ss_res;                             /* Restaura pilha do residente  */
  _SP = sp_res;
  enable();                                 /* Habilita interrupcoes        */

  troca_contexto_dos();                     /* Salva areas do DOS do transi-*/
                                            /* ente e restaura do residente */
  esta_na_28h = False;                      /* Nao esta mais na INT 28h     */
  aguarda = False;                          /* Nao precisa mais aguardar    */
}

/*---------------------------------------------------------------------------
  DESATIVA - Ativa Programa Residente
 ---------------------------------------------------------------------------*/
static void desativa()
{
  aguarda = True;                       /* Pede para Int 8 e 28h aguardarem */

  ativado = False;                      /* Avisa que esta desativado        */
  contador = t_ativ;                    /* durante T_ATIV ciclos de execucao*/

  troca_contexto_dos();                    /* Salva areas do DOS do residen-*/
                                           /* te e restaura do transiente   */
  guarda_dos = 0;

  ss_res = _SS;                            /* Guarda pilha do residente     */
  sp_res = _SP;

  disable();                               /* Desabilta interrupcoes        */
  _SS = ss_trans;                          /* Restaura pilha do transiente  */
  _SP = sp_trans;
  enable();                                /* Habilta interrupcoes          */

  esta_na_28h = False;                     /* Nao esta mais na INT 28h      */
  aguarda = False;                         /* Nao precisa mais aguardar     */
}

/*===========================================================================
  INTERRUPCOES ALTERADAS NO INICIO DE EXECUCAO
 ===========================================================================*/

/*---------------------------------------------------------------------------
  INT 08h - Interrupcao de Relogio
 ---------------------------------------------------------------------------*/
static void interrupt int_08h()
{
  (*velha_int_08h)();

  if (!muda_estado)            /* Se contador nao for zero  */
    if (--contador == 0) {     /* entao decrementa. Se zero */
      muda_estado = True;      /* muda o estado do programa */
    }
    else
      return;                 /* Senao, retorna            */

  if ((!aguarda) && (!esta_na_28h) && (muda_estado)) {
    if ((peekb (segmento_dos,dos_ocupado) == guarda_dos) && (!bios_ocupado)) {
       muda_estado = False;
       if (ativado)
         desativa();             /* Desativa programa residente */
       else
         ativa();                /* Ativa programa residente    */
     }
   }
}

/*---------------------------------------------------------------------------
  INT 13h - Interrupcao de Teclado
 ---------------------------------------------------------------------------*/
static void interrupt int_13h (bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flgs)
{
  bios_ocupado++;                   /* Avisa que o BIOS DISK esta ocupado */
  (*velha_int_13h)();               /* Chama Int 13h original             */
  ax = _AX; cx = _CX; dx = _DX;     /* Pega registradores de retorno      */
  int_24h();                        /* Pega FLAGS da CPU corrente         */
  flgs = flags_atual;               /* e restaura para o retorno          */
  bios_ocupado--;                   /* Avisa que o BIOS DISK desocupou    */
}

/*---------------------------------------------------------------------------
  INT 28h - Interrupcao DOS desocupado para funcoes maiores que 0Ch
 ---------------------------------------------------------------------------*/
static void interrupt int_28h()
{
  esta_na_28h = True;            /* Int 28h sendo executada          */
  (*velha_int_28h)();
  if ((!aguarda) && (muda_estado) && (!bios_ocupado)) {
    muda_estado = False;
    if (!ativado)
      ativa();                   /* Ativa programa residente         */
    else
      desativa();
  }
  esta_na_28h = False;
}

/*---------------------------------------------------------------------------
  INICIA_PILHA - Inicializa a pilha do residente e fica na memoria
 ---------------------------------------------------------------------------*/
static void inicia_pilha()
{
  ss_res = _SS;         /* Pega a pilha corrente do residente */
  sp_res = _SP;

  keep (0,_SS + (_SP / 16) - _psp + 10);   /* Fica residente */
}

/*===========================================================================
                      Rotinas globais da Biblioteca
 ===========================================================================*/

/*---------------------------------------------------------------------------
  FICA_RESIDENTE - Inicializa processo de residencia e chama TSR
 ---------------------------------------------------------------------------*/
void fica_residente()
{

  reg.r_ax = 0x3400;              /* Pega o endereco da variavel do DOS */
  intr (0x21,&reg);
  segmento_dos = reg.r_es;
  dos_ocupado  = reg.r_bx;

  dta = getdta();                 /* Pega DTA e PSP corrente do residente   */
  psp_seg = _psp;                 /* _psp e' uma variavel global do TURBO C */

  velha_int_08h = getvect (0x08); /* Guarda enderecos das interrupcoes      */
  velha_int_13h = getvect (0x13);
  velha_int_28h = getvect (0x28);

  velha_int_24h = int_24h;
  velha_int_1bh = int_1bh;

  setvect (0x08,int_08h);         /* Inicializa as interrupcoes         */
  setvect (0x13,int_13h);
  setvect (0x28,int_28h);

  inicia_pilha();                 /* Inicia a pilha e fica residente    */
}

/*---------------------------------------------------------------------------
  RETIRA_RESIDENTE - Retira programa residente da memoria
 ---------------------------------------------------------------------------*/
void retira_residente()
{
  disable();
  aguarda = True;               /* Nao permite mais Ativacao/Desativacao */
  enable();

  setvect (0x08,velha_int_08h);  /* Restaura vetores de interrupcao  */
  setvect (0x13,velha_int_13h);
  setvect (0x28,velha_int_28h);
  setvect (0x1b,velha_int_1bh);

  exit (0);                      /* e volta ao DOS   */
}

/*---------------------------------------------------------------------------
  VOLTA_TRANSIENTE - Retorna o controle ao processo transiente
 ---------------------------------------------------------------------------*/
void volta_transiente()
{
  struct REGPACK regs;
  muda_estado = True;     /* Avisa mudanca de estado */
  intr (0x28,&regs);      /* Chama INT 28h           */
}

