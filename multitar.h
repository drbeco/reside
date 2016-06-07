/*

  M U L T I T A R . H  - Definicao da Interface da biblioteca MULTITAR.C

*/

/* Definicao dos tipos de variaveis utilizadas */

#define Byte unsigned char
#define Word unsigned int
#define Boolean unsigned char
#define True 1
#define False 0

/*===================== Variaveis Globais de acesso =====================*/

extern Word contador;    /* Contador de ciclos de ATIVACAO e DESATIVACAO */
extern Word t_ativ;      /* Numero de ciclos para ATIVAR o RESIDENTE     */
extern Word t_exec;      /* Numero de ciclos de execucao do RESIDENTE    */

/*===================== Rotinas  Globais  de acesso =====================*/

void fica_residente();
void retira_residente();
void volta_residente();

