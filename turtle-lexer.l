%{
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>


#include	"turtle-ast.h"
#include	"turtle-parser.h"

#define YY_DECL int yylex(struct ast * ast)

%}

%option	warn
%option	8bit

/* don't generate a default rule */
%option	nodefault
%option	noyywrap

/* maintain the line number */
%option	yylineno

 /* numbers */
digit							[0-9]
non_zero_digit					[1-9]
integer							0|({non_zero_digit}{digit}*)
float_1							\.{digit}+
float_2							{integer}\.{digit}*
float							{float_1}|{float_2}
exp								({float}|{integer})[eE][+-]?{integer}
hex								0[xX][a-fA-F0-9]{1,8}

 /* identifier */
identifier						[A-Z][A-Z0-9]*

%%

 /* convert the numbers into double type */
{integer}						{	yylval.value	=	strtod(yytext,	NULL);	return	VALUE;	}
{float}							{	yylval.value	=	strtod(yytext,	NULL);	return	VALUE;	}
{exp}							{	yylval.value	=	strtod(yytext,	NULL);	return	VALUE;	}
{hex}							{	yylval.value	=	strtod(yytext,	NULL);	return	VALUE;	}

 /* transform the colors into their hex value */
"red"							{	yylval.color = 0xFFFF00000000; return COLOR;		        }
"green"							{	yylval.color = 0x0000FFFF0000; return COLOR;		        }
"blue"							{	yylval.color = 0x00000000FFFF; return COLOR;		        }
"cyan"							{	yylval.color = 0x0000FFFFFFFF; return COLOR;		        }
"yellow"						{	yylval.color = 0xFFFFFFFF0000; return COLOR;		        }
"magenta"						{	yylval.color = 0xFFFF0000FFFF; return COLOR;		        }
"black"							{	yylval.color = 0x000000000000; return COLOR;		        }
"gray"							{	yylval.color = 0x800080008000; return COLOR;		        }
"white"							{	yylval.color = 0xFFFFFFFFFFFF; return COLOR;		        }

 /* read the keywords */
"backward"						{	return	KW_BACKWARD;										}
"bw"							{	return	KW_BACKWARD;										}
"color"							{	return	KW_COLOR;											}
"down"							{	return	KW_DOWN;											}
"forward"						{	return	KW_FORWARD;											}
"fw"							{	return	KW_FORWARD;											}
"heading"						{	return	KW_HEADING;											}
"hd"						    {	return	KW_HEADING;											}
"home"							{	return	KW_HOME;											}
"left"							{	return	KW_LEFT;											}
"lt"							{	return	KW_LEFT;											}
"position"						{	return	KW_POSITION;										}
"pos"						    {	return	KW_POSITION;										}
"print"							{	return	KW_PRINT;											}
"right"							{	return	KW_RIGHT;											}
"rt"							{	return	KW_RIGHT;											}
"up"							{	return	KW_UP;												}
"cos"							{	return	KW_COS;												}
"random"						{	return	KW_RANDOM;											}
"sin"							{	return	KW_SIN;												}
"sqrt"							{	return	KW_SQRT;											}
"tan"							{	return	KW_TAN;												}
"call"							{	return	KW_CALL;											}
"proc"							{	return	KW_PROC;											}
"repeat"						{	return	KW_REPEAT;											}
"set"							{	return	KW_SET;												}

 /* faster */
[-+*/^,(){}]                    { return *yytext;                                               }

 /* add the identifiers to a BST, so no duplicate allocation will be done, and it's fast */
{identifier}					{	yylval.bst_entry = bst_at(&ast->parsing, yytext);  return BST_ENTRY;   }

 /* handle comments, and special comments */
";"     						;
"//"[^\n]*						;
"#"[^\n]*						;
[[:space:]]+					;

    /* parsing error, it's simply exiting */
.								{	fprintf(stderr,	"Unknown token: '%s' at line %d.\n", yytext, yylineno); exit(EXIT_FAILURE); }
%%
