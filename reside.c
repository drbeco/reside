/*--------------------------------------------------------------------------*
 |                                                                          |
 |             RRRRRR    EEEEEE   SSSSSS  II  DDDDDD    EEEEEE              |
 |             RR   RR  EE       SS       II  DD   DD  EE                   |
 |             RRRRRR   EEEEE     SSSSS   II  DD   DD  EEEEE                |
 |             RR   RR  EE            SS  II  DD   DD  EE                   |
 |             RR   RR   EEEEEE  SSSSSS   II  DDDDDD    EEEEEE              |
 |                                                                          |
 |                                                                          |
 |        Biblioteca de Residencia para Turbo C versao 1.5 ou superior      |
 |                                                                          |
 *--------------------------------------------------------------------------*/
#include <dos.h>
#include <bios.h>
#include <conio.h>

/* Definicao dos tipos de variaveis utilizadas */

#define Byte unsigned char
#define Word unsigned int
#define Boolean unsigned char
#define True 1
#define False 0

/*===================== Variaveis Globais de acesso =====================*/

/* Definicao das teclas de acesso (HOT KEY) */

Byte tecla_shift = 8;   /* ALT */     /* Teclas de Shift de acesso       */
Byte scan_tecla  = 1;   /* ESC */     /* Scan Code da tecla auxiliar     */

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
static void interrupt (*velha_int_09h)();  /* Interrupcao de Teclado     */
static void interrupt (*velha_int_13h)();  /* Interrupcao do BIOS DISK   */
static void interrupt (*velha_int_14h)();  /* Interrupcao de Intercambio */
static void interrupt (*velha_int_28h)();  /* Interrupcao de DOS OK      */

/* Interrupcoes Alteradas em Tempo de Ativacao */

static void interrupt (*velha_int_1bh)();  /* Interrupcao do CTRL-BREAK  */
static void interrupt (*velha_int_24h)();  /* Interrupcao Erro Critico   */

/* Areas para Salvamento do Contexto de Video */

static Byte tela_res   [4000];             /* Tela do programa residente */
static Byte tela_trans [4000];             /* e transiente               */
static Word posicao_res   = 0;             /* Posicao do cursor do pro-  */
static Word posicao_trans = 0;             /* residente e transiente     */
static Word forma_res     = 0;             /* Forma do cursor do progra- */
static Word forma_trans   = 0;             /* ma residente e transiente  */

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
  SALVA_VIDEO - Salva video do Transiente e restaura do Residente
 ---------------------------------------------------------------------------*/

static void salva_video()
{
  gettext (1,1,80,25,tela_trans);           /* Salva a tela do transiente  */
  reg.r_ax = 0x0300;
  reg.r_bx = 0;
  intr (0x10,&reg);                         /* Pega definicao e posicao do */
  posicao_trans = reg.r_dx;                 /* cursor do transiente        */
  forma_trans = reg.r_cx;

  puttext (1,1,80,25,tela_res);             /* Restaura Tela do residente  */
  reg.r_cx = forma_res;                     /* Restaura forma do cursor    */
  reg.r_ax = 0x0100;                        /* residente                   */
  intr (0x10,&reg);
  reg.r_bx = 0;                             /* Restaura posicao do cursor  */
  reg.r_dx = posicao_res;                   /* do residente                */
  reg.r_ax = 0x0200;
  intr (0x10,&reg);
}

/*---------------------------------------------------------------------------
  RESTAURA_VIDEO - Restaura video do Transiente e salva do Residente
 ---------------------------------------------------------------------------*/

static void restaura_video()
{
  gettext (1,1,80,25,tela_res);             /* Salva tela do residente     */
  reg.r_ax = 0x0300;                        /* Pega definicao e posicao do */
  reg.r_bx = 0;                             /* cursor do transiente        */
  intr (0x10,&reg);
  posicao_res = reg.r_dx;
  forma_res = reg.r_cx;

  puttext (1,1,80,25,tela_trans);           /* Restaura tela do transiente */
  reg.r_cx = forma_trans;                   /* Restaura forma do cursor    */
  reg.r_ax = 0x0100;                        /* do transiente               */
  intr (0x10,&reg);
  reg.r_bx = 0;                             /* Restaura posicao do cursor  */
  reg.r_dx = posicao_trans;                 /* do transiente               */
  reg.r_ax = 0x0200;
  intr (0x10, &reg);
}

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

/*
  ---------------------------------------------------------------------------
  ATIVA - Ativa Programa Residente
  ---------------------------------------------------------------------------
*/

static void ativa()
{
  aguarda++;                 /* Pede para int 8 e 28h aguardarem */

  ativado = True;

  guarda_dos = peekb (segmento_dos,dos_ocupado); /* Guarda estado do DOS */

  ss_trans = _SS;            /* Guarda pilha do transiente  */
  sp_trans = _SP;

  disable();
  _SS = ss_res;              /* Restaura pilha do residente */
  _SP = sp_res;
  enable();

  salva_video();             /* Salva video e areas do DOS  */
  troca_contexto_dos();

  esta_na_28h = False;       /* Desliga indicadores         */
  aguarda--;
}

/*
  ---------------------------------------------------------------------------
  DESATIVA - Ativa Programa Residente
  ---------------------------------------------------------------------------
*/

static void desativa()
{
  aguarda++;                 /* Pede para int 8 e 28h aguardarem */

  ativado = False;

  ss_res = _SS;              /* Salva pilha do residente         */
  sp_res = _SP;

  restaura_video();
  troca_contexto_dos();      /* Restaura video e areas do DOS    */

  guarda_dos = 0;            /* So volta a ativar com DOS desocupado */

  disable();
  _SS = ss_trans;            /* Restaura pilha do transiente     */
  _SP = sp_trans;
  enable();

  esta_na_28h = False;       /* Desliga indicadores              */
  aguarda--;
}

/*
  ===========================================================================
  INTERRUPCOES ALTERADAS NO INICIO DE EXECUCAO
  ===========================================================================
*/

/*
  ---------------------------------------------------------------------------
  INT 08h - Interrupcao de Relogio
  ---------------------------------------------------------------------------
*/

static void interrupt int_08h()
{
  (*velha_int_08h)();

  if ((!aguarda) && (!esta_na_28h) && (muda_estado)) {
    if ((peekb (segmento_dos,dos_ocupado) == guarda_dos) && (!bios_ocupado)) {
      muda_estado = False;
      if (ativado)
        desativa();
      else
        ativa();
    }
  }
}

/*
  ---------------------------------------------------------------------------
  INT 09h - Interrupcao de Teclado
  ---------------------------------------------------------------------------
*/

static void interrupt int_09h()
{
  if ((inportb(0x60) == scan_tecla) &&            /* HOT KEY pressionada ? */
      ((peekb (0x40,0x17) & tecla_shift) != 0)) {
    auxb = inportb(0x61);
    outportb (0x61,auxb | 0x80);                  /* Sim, avisa o reconhe- */
    outportb (0x61,auxb);                         /* cimento da tecla para */
    outportb (0x20,0x20);                         /* o controlador e liga  */
    muda_estado = True;                           /* indicador de mudanca  */
  }                                               /* de estado.            */
  else (*velha_int_09h)();                        /* Nao, chama int normal */
}

/*
  ---------------------------------------------------------------------------
  INT 13h - Interrupcao de Teclado
  ---------------------------------------------------------------------------
*/

static void interrupt int_13h (bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flgs)
{
  bios_ocupado++;                  /* Avisa que o BIOS esta ocupado */
  (*velha_int_13h)();              /* Chama Int anterior            */
  ax = _AX; cx = _CX; dx = _DX;    /* Restaura parametros retornados*/
  int_24h();                       /* Chama INT 24 para ler os flags*/
  flgs = flags_atual;              /* da CPU, retornados            */
  bios_ocupado--;                  /* Avisa que o BIOS desocupou    */
}

/*
  ---------------------------------------------------------------------------
  INT 14h - Interrupcao usada para interfaceamento com o programa residente
  ---------------------------------------------------------------------------
*/
static void interrupt int_14h (bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flgs)
{
  switch ((ax >> 8) & 0xff) {
    case 0x80 : ax |= 0xff00;            /* Avisa que esta instalado */
                break;
    case 0x81 : muda_estado = True;      /* Pede mudanca de estado   */
                break;
    case 0x82 : break;
    case 0x83 : dx = (tecla_shift << 8) | /* Retorna HOT KEY */
                      scan_tecla;
                break;
    case 0x84 : tecla_shift = (dx >> 8) & 0xff; /* Alerta HOT KEY */
                scan_tecla = dx & 0xff;
                break;
    default   : _AX = ax; _DX = dx;       /* Chama interrupcao normal */
                (*velha_int_14h)();
                ax = _AX;
  }
}

/*
  ---------------------------------------------------------------------------
  INT 28h - Interrupcao DOS desocupado para funcoes maiores que 0Ch
  ---------------------------------------------------------------------------
*/

static void interrupt int_28h()
{
  esta_na_28h = True;
  (*velha_int_28h)();
  if ((!aguarda) && (muda_estado) && (!bios_ocupado)) {
    muda_estado = False;
    if (ativado)
      desativa();
    else
      ativa();
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
  gettext (1,1,80,25,tela_res);   /* Salva tela corrente do residente   */
  reg.r_ax = 0x0300;
  intr (0x10,&reg);
  posicao_res = reg.r_dx;         /* Salva cursor corrente do residente */
  forma_res = reg.r_cx;

  reg.r_ax = 0x3400;              /* Pega o endereco da variavel do DOS */
  intr (0x21,&reg);
  segmento_dos = reg.r_es;
  dos_ocupado  = reg.r_bx;

  dta = getdta();                 /* Pega DTA e PSP corrente do residente   */
  psp_seg = _psp;                 /* _psp e' uma variavel global do TURBO C */

  velha_int_08h = getvect (0x08); /* Guarda enderecos originais das     */
  velha_int_09h = getvect (0x09); /* Interrupcoes desviadas             */
  velha_int_13h = getvect (0x13);
  velha_int_14h = getvect (0x14);
  velha_int_28h = getvect (0x28);

  velha_int_24h = int_24h;
  velha_int_1bh = int_1bh;

  setvect (0x08,int_08h);         /* Inicializa as interrupcoes         */
  setvect (0x09,int_09h);
  setvect (0x13,int_13h);
  setvect (0x14,int_14h);
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
  setvect (0x09,velha_int_09h);
  setvect (0x13,velha_int_13h);
  setvect (0x14,velha_int_14h);
  setvect (0x28,velha_int_28h);
  setvect (0x1b,velha_int_1bh);

  restaura_video();              /* Restaura video do Transiente  */
  exit (0);                      /* e volta ao DOS                */
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

/*---------------------------------------------------------------------------
  JA_INSTALADO - Retorna 1 se o programa ja estiver instalado na memoria
 ---------------------------------------------------------------------------*/

Boolean ja_instalado()
{
  struct REGPACK regs;

  regs.r_ax = 0x8000;                    /* Chama int 0x14  */
  intr (0x14,&regs);
  if ((regs.r_ax & 0xff00) == 0xff00)    /* retorna sim, se AH = 0xff */
    return (True);
  else
    return (False);
}

