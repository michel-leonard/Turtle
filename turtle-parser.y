%{
#include <stdio.h>

#include "turtle-ast.h"

int yylex();
void yyerror(struct ast *ast, const char *);
#define YYDEBUG 1
%}

%debug
%defines

%define parse.error verbose

/* the lexer know the existence of the AST by using a global variable
it's needed because i'm using a BST helper */

%lex-param { struct ast *ast }
%parse-param { struct ast *ast }

/* i created my types :
 - the bst_entry is used to provide some O(log(n)) solutions to my user
 - the color is a simple number that is human readable (hex notation)
*/

%union {
  double value;
  unsigned long long int color;
  struct bst_entry * bst_entry ;
  struct ast_node * node ;
}

/* the list of tokens i'm using */

%token <value>		VALUE		"value"
%token <color>		COLOR		"color"
%token <bst_entry>	BST_ENTRY	"bst_entry"

/* COMMANDS */
%token			KW_BACKWARD
%token			KW_COLOR
%token			KW_DOWN
%token			KW_FORWARD
%token			KW_HEADING
%token			KW_HOME
%token			KW_LEFT
%token			KW_POSITION
%token			KW_PRINT
%token			KW_RIGHT
%token			KW_UP
%token			KW_COS
%token			KW_RANDOM
%token			KW_SIN
%token			KW_SQRT
%token			KW_TAN
%token			KW_CALL
%token			KW_PROC
%token			KW_REPEAT
%token			KW_SET

/* what i think about precedence, maybe some of them are useless */
%left '+' '-'
%left '*' '/'
%right '^'
%nonassoc UNOP

%type <node> unit cmds cmd  cmd1  cmd2 cmd3 cmd4 expr

%%

unit:
	cmds			{ $$ = $1; ast->unit = $$; }

cmds:
	cmd cmds		{ if ($1) { $1->next = $2; $$ = $1; } else { ast -> error_number = 1 ; YYERROR; } ; }
| /* empty */			{ $$ = NULL ; }

/* i divided the commands in 4 groups, it's useless but it work */
cmd:
cmd1 | cmd2 | cmd3 | cmd4

/* the very simple commands */
cmd1:
	KW_UP					{ $$ = make_up(); 							}
|	KW_DOWN					{ $$ = make_down(); 							}
|	KW_COLOR	COLOR			{ $$ = make_color($2);							}
|	KW_HOME					{ $$ = make_home(); 							}

/* the more complex commands, a command is also a block of commands */
cmd2:
	'{' cmds '}'				{ $$ = make_cmds_block($2);						}
|	KW_PROC		BST_ENTRY	cmd	{ $$ = make_proc($2, $3); 						}
|	KW_REPEAT	expr	cmd		{ $$ = make_repeat($2, $3) ;						}

/* the other commands are using expr */
cmd3:
	KW_COLOR	expr ',' expr ',' expr	{ $$ = make_raw_color($2, $4, $6);					}
|	KW_LEFT		expr			{ $$ = make_left($2); 							}
|	KW_RIGHT	expr			{ $$ = make_right($2); 							}
|	KW_HEADING	expr			{ $$ = make_heading($2); 						}
|	KW_POSITION	expr ',' expr		{ $$ = make_position($2, $4); 						}
|	KW_PRINT	expr			{ $$ = make_print($2); 							}
|	KW_SET		BST_ENTRY	expr	{ $$ = make_set($2, $3); 						}
|	KW_FORWARD	expr			{ $$ = make_forward($2); 						}
|	KW_BACKWARD	expr			{ $$ = make_backward($2); 						}

/* the last command is call */
cmd4:
	KW_CALL		BST_ENTRY		{ $$ = make_call($2); 							}

/* one expr is described like this :
the final value of an expr is always a double,
maybe after a complex recursive resolution */
expr:
	VALUE					{ $$ = make_value($1);							}
|	BST_ENTRY				{ $$ = make_name($1);							}
|	expr  '+'  expr				{ $$ = make_binop('+', $1, $3);						}
|	expr  '-'  expr				{ $$ = make_binop('-', $1, $3);						}
|	expr  '*'  expr				{ $$ = make_binop('*', $1, $3);						}
|	expr  '/'  expr				{ $$ = make_binop('/', $1, $3);						}
|	expr  '^'  expr				{ $$ = make_binop('^', $1, $3);						}
|	'('  expr  ')'				{ $$ = make_expr_block($2);						}
|	'-'  expr %prec UNOP			{ $$ = make_unop('-', $2);						}
|	'+'  expr %prec UNOP			{ $$ = make_unop('+', $2);						}
|	KW_COS		'(' expr ')'		{ $$ = make_math_func(FUNC_COS, $3);					}
|	KW_SIN		'(' expr ')'		{ $$ = make_math_func(FUNC_SIN, $3);			 		}
|	KW_TAN		'(' expr ')'		{ $$ = make_math_func(FUNC_TAN, $3);			 		}
|	KW_SQRT		'(' expr ')'		{ $$ = make_math_func(FUNC_SQRT, $3);			 		}
|	KW_RANDOM	'(' expr ',' expr ')'	{ $$ = make_random($3, $5);		 				}

%%

void yyerror(struct ast *ast, const char *msg) {
  (void) ast;
  fprintf(stderr, "%s\n", msg);
}
