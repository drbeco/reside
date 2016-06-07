/*

  R E S I D E . H  - Definicao da Interface da biblioteca reside.c

*/

/* Definicao dos tipos de variaveis utilizadas */

#define Byte unsigned char
#define Word unsigned int
#define Boolean unsigned char
#define True 1
#define False 0

/*===================== Variaveis Globais de acesso =====================*/

/* Definicao das teclas de acesso (HOT KEY) */

extern Byte tecla_shift;              /* Teclas de Shift de acesso       */
extern Byte scan_tecla;               /* Scan Code da tecla auxiliar     */

/* Definicao das teclas de Shift, Alt e Ctrl, usadas em tecla_shift      */

#define SHIFT_D  0x1     /* Shift Direito  */
#define SHIFT_E  0x2     /* Shift Esquerdo */
#define CTRL     0x4     /* Control        */
#define ALT      0x8     /* Alt            */

/*===================== Rotinas  Globais  de acesso =====================*/

void fica_residente();                /* Inicializa processo residente   */
void retira_residente();              /* Retira programa da memoria      */
void volta_transiente();              /* Retorna o controle ao transiente*/
Boolean ja_instalado();               /* Retorna 1 se programa na memoria*/

