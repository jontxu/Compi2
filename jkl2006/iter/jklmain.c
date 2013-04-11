/* jklmain.c
   Analizador sint�ctico descendente iterativo del lenguaje JKL
   (c) JosuKa D�az Labrador 2006; see gpl.txt
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tpila.h"

/* ya incluye jkl.h */

/* Terminales de la gram�tica: est�n en jkl.h */

#define TER_OFFSET 257

/* recordar que DOLAR debe transformarse en SDOLAR
   (y lo mejor entonces es que SDOLAR lleve este primer c�digo)
   con la siguiente funci�n
*/

TTOKEN falsoyylex()
{
	TTOKEN token;

	token = yylex();
	if( token == DOLAR ) token = SDOLAR;
	return( token );
}

/* Variables de la gram�tica */

#define VAR_OFFSET 1000

/* adem�s tiene que ser el axioma, hala */

#define V_PROG	1000
#define V_DECL	1001
#define V_SENTC	1002
#define V_LSENT	1003
#define V_SENT	1004
#define V_PELSE	1005
#define V_EXPR	1006
#define V_EXPRP	1007
#define V_EAND	1008
#define V_EANDP	1009
#define V_EREL	1010
#define V_ERELP	1011
#define V_ARIT	1012
#define V_ARITP	1013
#define V_TERM	1014
#define V_TERMP	1015
#define V_FACT	1016
#define V_RANDO	1017

/* Acciones sem�nticas */

#define ACC_OFFSET 2000

#define A_PROG1	2000
#define A_PROG2	2001
#define A_PROG3	2002
#define A_DECL1	2003
#define A_READ1	2004
#define A_WRTE1	2005
#define A_WRTC1	2006
#define A_WRTL1	2007
#define A_NUM		2008
#define A_ID		2009
#define A_ADIT1	2010
#define A_ADIT2	2011
#define A_ASIG1	2012
#define A_ASIG2	2013
#define A_W1		2014
#define A_W2		2015
#define A_W3		2016
#define A_I1		2017
#define A_I2		2018
#define A_I3		2019
#define A_I4		2020


/* Resolver reglas de la 20 a la 40 */

/* Reglas de la gram�tica; por orden desde 0 (acciones entre corchetes):

 0             regla de relleno (todo 0)
 1   prog  ->  PPROG  ID  P_COMA  [A_PROG1]  decl  [A_PROG2]  sentc  PUNTO  [A_PROG3]
 2   decl  ->  lambda
 3          |  PVAR  ID  [A_DECL1]  P_COMA  decl
 4   sentc ->  PBEGIN  lsent  PEND
 5   lsent ->  lambda
 6          |  sent  lsent
 7   sent  ->  P_COMA
 8          |  ID  [A_ASIG1] ASIGN expr [A_ASIG2] P_COMA
 9          |  sentc
10          |  PREAD  ID  [A_READ1]  P_COMA
11          |  PWRITE  expr  [A_WRTE1]  P_COMA
12          |  PWRITC  CAD  [A_WRTC1]  P_COMA
13          |  PWRITL  [A_WRTL1]  P_COMA
14          |  PIF  expr  PTHEN [A_I1] sent  pelse 
15          |  PWHILE  [A_W1] expr  PDO [A_W2] sent [A_W3]
16          |  PDO  sent  PWHILE  expr  P_COMA
17          |  PFOR  ID  ASIGN  expr  PTODO  expr  PDO  sent
18   pelse ->  lambda [A_I2] (est� encima, como si fuera lam[A_I2]bda)
19          |  PELSE [A_I3] sent [A_I4]
20   expr  ->  eand  exprp
21   exprp ->  lambda
22          |  OP_OR  eand  exprp
23   eand  ->  erel  eandp
24   eandp ->  lambda
25          |  OP_AND  erel  eandp
26   erel  ->  arit  erelp
27   erelp ->  lambda
28          |  OP_REL  arit  erelp
29   arit  ->  term  aritp
30   aritp ->  lambda
31          |  OP_ADIT [A_ADIT1]  term [A_ADIT2]  aritp
32   term  ->  fact  termp
33   termp ->  lambda
34          |  OP_MULT  fact  termp
35   fact  ->  OP_NOT  fact
36          |  OP_ADIT  fact
37          |  rando
38   rando ->  NUM [A_NUM]
39          |  ID [A_ID]
40          |  PAR_ABR  expr  PAR_CER

   trucos:
   * solo hacen falta partes derechas
   * se meten ya invertidas
   * relleno a ceros para que haya hueco para insertar las acciones
*/

/*
 * etiq #INIT ? A_WHILE1
 * cod <expr> 
 * si_falso_ir_a #FIN ? A_WHILE2
 * cod <sent>
 * ir_a #INIT ? A_WHILE3
 * etiq #FIN
 */

int reglasG[41][12] = {
/*  0 */	 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/*  1 */	,{ A_PROG3, PUNTO, V_SENTC, A_PROG2, V_DECL, P_COMA, A_PROG1, ID, PPROG, 0, 0, 0 }
/*  2 */	,{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/*  3 */	,{ V_DECL, P_COMA, A_DECL1, ID, PVAR, 0, 0, 0, 0, 0, 0, 0 }
/*  4 */	,{ PEND, V_LSENT, PBEGIN, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/*  5 */	,{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/*  6 */	,{ V_LSENT, V_SENT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/*  7 */	,{ P_COMA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/*  8 */	,{ P_COMA, A_ASIG2, V_EXPR, ASIGN, A_ASIG1, ID, 0, 0, 0, 0, 0, 0, }
/*  9 */	,{ V_SENTC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 10 */	,{ P_COMA, A_READ1, ID, PREAD, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 11 */	,{ P_COMA, A_WRTE1, V_EXPR, PWRITE, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 12 */	,{ P_COMA, A_WRTC1, CAD, PWRITC, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 13 */	,{ P_COMA, A_WRTL1, PWRITL, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 14 */	,{ V_PELSE, V_SENT, A_I1, PTHEN, V_EXPR, PIF, 0, 0, 0, 0, 0 }
/* 15 */	,{ A_W3, V_SENT, A_W2, PDO, V_EXPR, A_W1, PWHILE, 0, 0, 0, 0, 0 }
/* 16 */	,{ P_COMA, V_EXPR, PWHILE, V_SENT, PDO, 0, 0, 0, 0, 0, 0, 0 }
/* 17 */	,{ V_SENT, PDO, V_EXPR, PTODO, V_EXPR, ASIGN, ID, PFOR, 0, 0, 0, 0 }
/* 18 */	,{ A_I2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 19 */	,{ A_I4, V_SENT, A_I3, PELSE, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 20 */	,{ V_EXPRP, V_EAND, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 21 */	,{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 22 */	,{ V_EXPRP, V_EAND, OP_OR, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 23 */	,{ V_EANDP, V_EREL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 24 */	,{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 25 */	,{ V_EANDP, V_EREL, OP_AND, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 26 */	,{ V_ERELP, V_ARIT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 27 */	,{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 28 */	,{ V_ERELP, V_ARIT, OP_REL, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 29 */	,{ V_ARITP, V_TERM, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 30 */	,{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 31 */	,{ V_ARITP, A_ADIT2, V_TERM, A_ADIT1, OP_ADIT, 0, 0, 0, 0, 0, 0, 0}
/* 32 */	,{ V_TERMP, V_FACT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 33 */	,{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 34 */	,{ V_TERMP, V_FACT, OP_MULT, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 35 */	,{ V_FACT, OP_NOT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 36 */	,{ V_FACT, OP_ADIT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 37 */	,{ V_RANDO, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 38 */	,{ A_NUM, NUM, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 39 */	,{ A_ID, ID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* 40 */	,{ PAR_CER, V_EXPR, PAR_ABR, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

/* Tabla LL(1)
   Las columnas deben ir en el orden correlativo de terminales marcado en jkl.h:
   SDOLAR, PAR_ABR, PAR_CER, P_COMA, OP_OR, OP_AND, OP_REL, OP_ADIT, OP_MULT,
   OP_NOT, NUM, ID, ASIGN, PUNTO, CAD, PBEGIN, PEND, PPROG, PVAR, PREAD,
   PWRITE, PWRITC, PWRITL, PIF, PTHEN, PELSE, PWHILE, PDO, PFOR, PTODO
*/

int tablaLL1[18][30] = {
/* cols:            SDOLAR , PAR_ABR, PAR_CER, P_COMA , OP_OR  , OP_AND , OP_REL , OP_ADIT, OP_MULT, OP_NOT , NUM    , ID     , ASIGN  , PUNTO  , CAD    , PBEGIN , PEND   , PPROG  , PVAR   , PREAD  , PWRITE , PWRITC , PWRITL , PIF    , PTHEN  , PELSE  , PWHILE , PDO    , PFOR   , PTODO */
/* prog  */	 {  0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 1      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0   }
/* decl  */	,{  0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 2      , 0      , 0      , 3      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0   }
/* sentc */	,{  0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 4      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0   }
/* lsent */	,{  0      , 0      , 0      , 6      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 6      , 0      , 0      , 0      , 6      , 5      , 0      , 0      , 6      , 6      , 6      , 6      , 6      , 0      , 0      , 6      , 6      , 6      , 0   }
/* sent  */	,{  0      , 0      , 0      , 7      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 8      , 0      , 0      , 0      , 9      , 0      , 0      , 0      , 10     , 11     , 12     , 13     , 14     , 0      , 0      , 15     , 16     , 17     , 0   }
/* pelse */	,{  0      , 0      , 0      , 18     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 18     , 0      , 0      , 0      , 18     , 18     , 0      , 0      , 18     , 18     , 18     , 18     , 18     , 0      , 19     , 18     , 18     , 18     , 0   }
/* expr  */	,{  0      , 20     , 0      , 0      , 0      , 0      , 0      , 20     , 0      , 20     , 20     , 20     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0   }
/* exprp */	,{  0      , 0      , 21     , 21     , 22     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 21     , 0      , 0      , 21     , 0      , 21  }
/* eand  */	,{  0      , 23     , 0      , 0      , 0      , 0      , 0      , 23     , 0      , 23     , 23     , 23     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0   }
/* eandp */	,{  0      , 0      , 24     , 24     , 24     , 25     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 24     , 0      , 0      , 24     , 0      , 24  }
/* erel  */	,{  0      , 26     , 0      , 0      , 0      , 0      , 0      , 26     , 0      , 26     , 26     , 26     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0   }
/* erelp */	,{  0      , 0      , 27     , 27     , 27     , 27     , 28     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 27     , 0      , 0      , 27     , 0      , 27  }
/* arit  */	,{  0      , 29     , 0      , 0      , 0      , 0      , 0      , 29     , 0      , 29     , 29     , 29     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0   }
/* aritp */	,{  0      , 0      , 30     , 30     , 30     , 30     , 30     , 31     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 30     , 0      , 0      , 30     , 0      , 30  }
/* term  */	,{  0      , 32     , 0      , 0      , 0      , 0      , 0      , 32     , 0      , 32     , 32     , 32     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0   }
/* termp */	,{  0      , 0      , 33     , 33     , 33     , 33     , 33     , 33     , 34     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 33     , 0      , 0      , 33     , 0      , 33  }
/* fact  */	,{  0      , 37     , 0      , 0      , 0      , 0      , 0      , 36     , 0      , 35     , 37     , 37     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0   }
/* rando */	,{  0      , 40     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 38     , 39     , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0   }
};

/* Los archivos: se inician en main */

extern FILE *yyin;        /* entrada: el nombre es de lex */
       FILE *fichCod;     /* salida de c�digo MPV */

/* unidades l�xicas de anticipaci�n y actual */

TTOKEN tokenACT, tokenANT;
YYSTYPE atribACT, atribANT;

void parear( TTOKEN token )
{
	if( token == tokenANT ) {
		tokenACT = tokenANT; atribACT = atribANT;
		tokenANT = falsoyylex();
			/* esta llamada actualiza lateralmente atribANT */
	} else
		yyerror( "syntactic error: token incorrecto" );
}

/* pilas sint�ctica y sem�ntica */

TPILA *pilaSint;
TPILA *pilaSem;

/* generaci�n de c�digo */

void gc0( char *code )
{
	fprintf( fichCod, "%s\n", code );
}

void gc1( char *code, char *arg )
{
	fprintf( fichCod, "%s\t%s\n", code, arg );
}

void gcC( char *coment )
{
	fprintf( fichCod, "; %s\n", coment );
}

/* Las funciones de etiquetas */

char* etiqNew(char* text) 
{
  char* etiq = malloc(sizeof(text) * (strlen(text) + 1));
  int i;
  for (i = 0; i < strlen(text); i++) {
    etiq[i] = text[i];
  }
  return etiq;
}

void etiqFree(char* etiq) 
{
  free(etiq);
  return;
}

/* Las acciones sem�nticas */

void accPROG1()
{
	gcC( "comienzo del programa: declaraciones globales" );
}

void accPROG2()
{
	gcC( "comienzo del programa: instrucciones" );
	gc0( "inicio" );
}

void accPROG3()
{
	gc0( "fin" );
	gcC( "fin del programa" );
}

void accDECL1()
{
	gc1( "globali", atribACT.TCadena );
}

void accREAD1()
{
	gc1( "valori", atribACT.TCadena );
	gc0( "leeri" );
	gc0( ":=" );
}

void accWRTE1()
{
	gc0( "escribiri" );
}

void accWRTC1()
{
	gc1( "escribirs", atribACT.TCadena );
}

void accWRTL1()
{
	gc0( "escribirln" );
}

void accNUM()
{
	gc1( "insi", atribACT.TCadena );
	return;
}

void accID()
{
	gc1( "valord", atribACT.TCadena );
	return;
}

void accADIT1()
{
	//pilaSem neutra 0
	pilaSem = pilaPUSH( pilaSem, atribACT );
	return;
}


void accADIT2()
{
	 YYSTYPE op;
	 op = pilaTOP( pilaSem );
	 if (op.TCodigo == 1) gc0("+");
	 else if (op.TCodigo == 2) gc0("-");
	 else yyerror( "no deber�a darse ");
	 pilaSem = pilaPOP( pilaSem );
	 return;
}

void accASIG1() 
{
  gc1("valori", atribACT.TCadena );
  return;
}

void accASIG2() 
{
  gc0(":=");
  return;
}

void accWHILE1()
{
  //pilaSem neutra 9
  YYSTYPE eI, eF; //etiqInit, etiqFin;
  eI.TCadena = etiqNew("");
  eF.TCadena = etiqNew("");
  gc1 ("etiq", eI.TCadena );
  pilaSem = pilaPUSH( pilaSem, eI );
  pilaSem = pilaPUSH( pilaSem, eF );
  return;
  //pilaSem +2 eF eI
}

void accWHILE2()
{
  //pilaSem +2 eF eI
  YYSTYPE eF; //etiqInit, etiqFIn;
  eF = pilaTOP( pilaSem );
  gc1 ("si_falso_ir_a", eF.TCadena);
  return;
  //pilaSem +2 eF eI
}

void accWHILE3()
{
  //pilaSem neutra 9
  YYSTYPE eI, eF; //etiqInit, etiqFIn;
  eF = pilaTOP( pilaSem );
  pilaSem = pilaPOP( pilaSem);
  eI = pilaTOP( pilaSem );
  gc1 ("ir_a", eI.TCadena );
  gc1 ("etiq", eF.TCadena );
  etiqFree(eI.TCadena);
  etiqFree(eF.TCadena);
  return;
  //pilaSem +2 eF eI
}

void accIF1()
{
  //pilaSem neutra 0
  YYSTYPE eE; //etiqElse
  eE.TCadena = etiqNew("");
  gc1( "si-falso-ir-a", eE.TCadena );
  pilaSem = pilaPUSH( pilaSem, eE );
  return;
  //pilaSem +1 eE
}

void accIF3()
{
  //pilaSem neutra 0
  YYSTYPE eE; //etiqElse
  eE = pilaTOP( pilaSem );
  gc1("etiq", eE.TCadena );
  pilaSem = pilaPOP( pilaSem );
  etiqFree( eE.TCadena );
  return;
  //pilaSem +1 eE
}

void accIF4()
{
  //pilaSem neutra 0
  YYSTYPE eE, eF; //etiqElse
  
  eE = pilaTOP( pilaSem );
  pilaSem = pilaPOP( pilaSem );
  eF.TCadena = etiqNew("");
  gc1("ir-a", eE.TCadena );
  gc1("etiq", eF.TCadena );
  pilaSem = pilaPUSH( pilaSem, eF );
  etiqFree( eE.TCadena );
  return;
  //pilaSem +1 eE
}

void accIF2()
{
  //pilaSem neutra 0
  YYSTYPE eF; //etiqElse
  eF = pilaTOP( pilaSem );
  gc1("etiq", eF.TCadena );
  pilaSem = pilaPOP( pilaSem );
  etiqFree( eF.TCadena );
  return;
  //pilaSem +1 eE
}

/* El �ndice de acciones: esta s� que es buena */

void (*tablaACC[21])() = {
	accPROG1, accPROG2, accPROG3, accDECL1, accREAD1, accWRTE1, accWRTC1, accWRTL1, accNUM, accID, accADIT1, 
	accADIT2, accASIG1, accASIG2, accWHILE1, accWHILE2, accWHILE3, accIF1, accIF2, accIF3, accIF4
};

void compilar()
{
	YYSTYPE toppila;
	int regla, var, term;
	int i;

	tokenACT = atribACT.TCodigo = 0;
	tokenANT = falsoyylex();
		/* esta llamada actualiza lateralmente atribANT */

	toppila.TCodigo = VAR_OFFSET;
	pilaSint = pilaPUSH( pilaSint, toppila );

		/* empieza el tema */
	while( pilaNOVACIA( pilaSint ) ) {
		toppila = pilaTOP( pilaSint );
		pilaSint = pilaPOP( pilaSint );
		if( toppila.TCodigo >= ACC_OFFSET ) {
				/* es una accion: ejecutar */
			tablaACC[ toppila.TCodigo - ACC_OFFSET ]();
		} else if( toppila.TCodigo >= VAR_OFFSET ) {
				/* es una variable: expandir */
			var = toppila.TCodigo - VAR_OFFSET;
			term = tokenANT - TER_OFFSET;
			regla = tablaLL1[var][term];
			if( regla == 0 )
				yyerror( "syntactic error: no hay regla" );
			else
				for( i=0; reglasG[regla][i]; i++ ) {
					toppila.TCodigo = reglasG[regla][i];
					pilaSint = pilaPUSH( pilaSint, toppila );
				}
		} else /* se supone que solo puede ser terminal: parear */
			parear( toppila.TCodigo );
	}

		/* acab� el tema */
	if( tokenANT != SDOLAR )
		yyerror( "syntactic error: programa deber�a haber acabado" );
}

main( int argc, char *argv[] )
{
	char fuente[256];

	if( argc == 2 ) {
		strcpy( fuente, argv[1] );
		strcat( fuente, ".jkl" );
		if( ( yyin = fopen( fuente, "r" ) ) != NULL )
		{
			strcpy( fuente, argv[1] );
			strcat( fuente, ".mpv" );
			fichCod = fopen( fuente, "w" );

			/* empieza la compilaci�n */
			compilar();
			/* ha terminado la compilaci�n */

			fclose( yyin );
			fclose( fichCod );
		}
		else printf( "No se encontro el fichero %s\n", fuente );
	}
	else
	{
		/* fnsplit( argv[0], NULL, NULL, fuente, NULL ); */
		printf( "Usage: %s jkl-source-file\n", argv[0] );
		printf( "  jkl-source-file: sin extension, buscara .jkl\n");
	}
}
