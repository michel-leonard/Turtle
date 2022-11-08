#ifndef TURTLE_AST_H
#define TURTLE_AST_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>

#define TURTLE_DEG_TO_RAD				0.0174532925199432957692369076848861271344287188854172545609719144
#define TURTLE_REPEAT_MAX_ITERATIONS	140737488355328LL

// simple commands
enum ast_cmd {
	CMD_UP, CMD_DOWN, CMD_RIGHT, CMD_LEFT, CMD_HEADING, CMD_FORWARD, CMD_BACKWARD, CMD_POSITION, CMD_HOME, CMD_COLOR, CMD_PRINT,
};

// internal functions
enum ast_func {
	FUNC_COS, FUNC_RANDOM, FUNC_SIN, FUNC_SQRT, FUNC_TAN,
};

// kind of a node in the abstract syntax tree
enum ast_kind {
	KIND_CMD_SIMPLE, KIND_CMD_REPEAT, KIND_CMD_BLOCK, KIND_CMD_PROC, KIND_CMD_CALL, KIND_CMD_SET,
	KIND_EXPR_FUNC, KIND_EXPR_VALUE, KIND_EXPR_UNOP, KIND_EXPR_BINOP, KIND_EXPR_BLOCK, KIND_EXPR_NAME,
};

#define AST_CHILDREN_MAX 3

// a node in the abstract syntax tree
struct ast_node {
	enum ast_kind kind; // kind of the node
	union {
		enum ast_cmd cmd;   // kind == KIND_CMD_SIMPLE
		double value;       // kind == KIND_EXPR_VALUE, for literals
		char op;            // kind == KIND_EXPR_BINOP or kind == KIND_EXPR_UNOP, for operators in expressions
		enum ast_func func; // kind == KIND_EXPR_FUNC, a function
		struct bst_entry * bst_entry ; // kind == KIND_EXPR_NAME, the key of procedures and variables
	} u;
	size_t children_count;  // the number of children of the node
	struct ast_node *children[AST_CHILDREN_MAX];  // the children of the node (arguments of commands, etc)
	struct ast_node *next;  // the next node in the sequence
};

// My BST documentation is located here : https://bit.ly/C-AVL

struct bst_entry {
	char *key;
	union {
		struct ast_node * node ;
		double number ;
		struct {
			bool is_proc_name ;
			bool is_var_name ;
		} parsing;
	} value;
};

struct bst_node {
	struct bst_entry entry;
	struct bst_node *rel[3]; // [0, 1, 2] = [left child, right child, parent]
	int height;
};

struct bst_manager {
	struct bst_node *root;
	size_t count;
	int affected;
	int search_only;
};

struct bst_entry *bst_at(struct bst_manager *, const char *);
void bst_destroy(struct bst_manager *);

// root of the abstract syntax tree
struct ast {
	struct ast_node *unit;
	struct bst_manager parsing ;
	int error_number ;
};

// do not forget to destroy properly! no leaks allowed!
void ast_destroy(struct ast *self);

// the execution context
struct context {
	double x;
	double y;
	double angle;
	bool up;
	double r;
	double g;
	double b;
	struct {
		double r;
		double g;
		double b;
		double y;
		double x;
	} let ;
	size_t lines_printed ;
	size_t nested_call_count;
	struct bst_manager variables ;
	struct bst_manager procedures ;
	int error_number ;
};

// do not forget to destroy properly! no leaks allowed!
int context_destroy(struct context *ctx);

// create an initial context
void context_create(struct context *self);

// print the tree as if it was a Turtle program
void ast_print(const struct ast *self);

// evaluate the tree and generate some basic primitives
void ast_eval(const struct ast *self, struct context *ctx);

struct ast_node * make_forward(struct ast_node * expr);
struct ast_node * make_backward(struct ast_node * expr);
struct ast_node * make_up();
struct ast_node * make_down();
struct ast_node * make_color(unsigned long long int number);
struct ast_node * make_raw_color(struct ast_node * expr_r, struct ast_node * expr_g, struct ast_node * expr_b);
struct ast_node * make_left(struct ast_node * expr);
struct ast_node * make_right(struct ast_node * expr);
struct ast_node * make_heading(struct ast_node * expr);
struct ast_node * make_position(struct ast_node * expr_x, struct ast_node * expr_y);
struct ast_node * make_home();
struct ast_node * make_print(struct ast_node * expr);
struct ast_node * make_repeat(struct ast_node * expr_count, struct ast_node * to_repeat);
struct ast_node * make_cmds_block(struct ast_node * to_block);
struct ast_node * make_call(struct bst_entry * entry);
struct ast_node * make_set(struct bst_entry * entry, struct ast_node *expr);
struct ast_node * make_proc(struct bst_entry * entry, struct ast_node *root);
struct ast_node * make_value(double value);
struct ast_node * make_name(struct bst_entry * entry);
struct ast_node * make_math_func(enum ast_func func, struct ast_node * expr);
struct ast_node * make_random(struct ast_node * expr_low, struct ast_node * expr_high);
struct ast_node * make_expr_block(struct ast_node * to_block);
struct ast_node * make_binop(char op, struct ast_node * lhs, struct ast_node * rhs);
struct ast_node * make_unop(char op, struct ast_node * expr);

void ast_eval_node(struct context *ctx, struct ast_node *node);
double ast_eval_expr(struct context *ctx, struct ast_node *node);
void ast_eval_forward(struct context *ctx, struct ast_node *node);
void ast_eval_backward(struct context *ctx, struct ast_node *node);
void ast_eval_up(struct context *ctx);
void ast_eval_down(struct context *ctx);
void ast_eval_color(struct context *ctx, struct ast_node *node);
void ast_eval_left(struct context *ctx, struct ast_node *node);
void ast_eval_right(struct context *ctx, struct ast_node *node);
void ast_eval_heading(struct context *ctx, struct ast_node *node);
void ast_eval_position(struct context *ctx, struct ast_node *node);
void ast_eval_home(struct context *ctx);
void ast_eval_print(struct context *ctx, struct ast_node *node);
void ast_eval_repeat(struct context *ctx, struct ast_node *node);
void ast_eval_block(struct context *ctx, struct ast_node *node);
void ast_eval_call(struct context *ctx, struct ast_node *node);
void ast_eval_set(struct context *ctx, struct ast_node *node);
void ast_eval_proc(struct context *ctx, struct ast_node *node);

void ast_print_node(struct ast_node *node);
void ast_print_expr(struct ast_node *node);
void ast_print_forward(struct ast_node *node);
void ast_print_backward(struct ast_node *node);
void ast_print_up();
void ast_print_down();
void ast_print_color(struct ast_node *node);
void ast_print_left(struct ast_node *node);
void ast_print_right(struct ast_node *node);
void ast_print_heading(struct ast_node *node);
void ast_print_position(struct ast_node *node);
void ast_print_home();
void ast_print_print(struct ast_node *node);
void ast_print_repeat(struct ast_node *node);
void ast_print_block(struct ast_node *node);
void ast_print_call(struct ast_node *node);
void ast_print_set(struct ast_node *node);
void ast_print_proc(struct ast_node *node);


// As usual I finished so early that I stopped this project for 3 weeks.
// I even made my own viewer at : https://turtle.sur.ovh/ .
// Thank you to all the teaching staff.

#endif /* TURTLE_AST_H */
