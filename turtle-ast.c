#include "turtle-ast.h"

#define PI      3.14159265358979323846
#define SQRT2   1.41421356237309504880
#define SQRT3   1.73205080756887729352

/* BST (binary search tree) is used to provide some log(N) complexity solutions while operating over procedures and variables */
#define bst_left rel[0]
#define bst_right rel[1]
#define bst_parent rel[2]

#define bst_inline inline __attribute__((always_inline))

// This action is used to process the AVL rotations of the BST.
static bst_inline void bst_rotate(struct bst_node *a, const int rel_a, struct bst_node *b, const int rel_b) {
	if ((b->rel[rel_b] = a->rel[rel_a])) {
		a->rel[rel_a]->bst_parent = b;
	} else {
		b->rel[rel_b] = a->rel[rel_a];
	}
	b->bst_parent = a;
	a->rel[rel_a] = b;
}

// This action is used to process the AVL rotations of the BST.
static bst_inline void bst_swap(struct bst_node **root, const struct bst_node *a, struct bst_node *b) {
	if (a->bst_parent) {
		a->bst_parent->rel[a->bst_parent->bst_right == a] = b;
	} else {
		*root = b;
	}
	b->bst_parent = a->bst_parent;
}

// Return an integer, this function is used to compute the height of a particular node of the AVL BST.
static bst_inline int bst_height(const struct bst_node *node) {
	if (node->bst_right) {
		if (node->bst_left) {
			return 1 + node->rel[node->bst_right->height > node->bst_left->height]->height;
		} else {
			if (node->bst_right) {
				return 1 + node->bst_right->height;
			} else {
				return 1;
			}
		}
	} else {
		if (node->bst_left) {
			return 1 + node->bst_left->height;
		} else {
			if (node->bst_right) {
				return 1 + node->bst_right->height;
			} else {
				return 1;
			}
		}
	}
}

// Return a char, this function is used to compute the "rotation kind" name during the AVL rotation in the BST.
static bst_inline char bst_op_kind(const struct bst_node *node) {
	if (node->bst_left) {
		if (node->bst_right) {
			if (node->bst_left->height > node->bst_right->height) {
				return 'L';
			} else {
				return 'R';
			}
		} else {
			return 'L';
		}
	} else {
		return 'R';
	}
}

// Return a char, this function is used to compute the "rotation" name during the AVL rotation in the BST.
static bst_inline char bst_has_op(const struct bst_node *node) {
	switch ((node->bst_left ? node->bst_left->height : 0) - (node->bst_right ? node->bst_right->height : 0)) {
		case 2 :
			return 'L';
		case -2 :
			return 'R';
	}
	return 0;
}

// This action is used to rebalance the AVL BST tree.
static bst_inline void bst_rebalance(struct bst_node **root, struct bst_node *top) {
	struct bst_node *middle, *bottom;
	char op[3];
	int i;
	// Maybe "top" is now too tall.
	while (top) {
		i = top->height;
		top->height = bst_height(top);
		if (i == top->height) { // Height not change ?
			break; // Retracing can stop.
		}
		*op = bst_has_op(top);
		if (*op) {
			middle = top->rel[*op == 'R'];
			op[1] = bst_op_kind(middle);
			i = op[1] == 'R';
			bottom = middle->rel[i];
			top->height = bottom->height;
			if (op[0] == op[1]) {
				bst_swap(root, top, middle);
				bst_rotate(middle, !i, top, i); // LL or RR done.
			} else {
				bst_swap(root, top, bottom);
				bst_rotate(bottom, !i, middle, i);
				bst_rotate(bottom, i, top, !i); // LR or RL done.
				middle->height = bottom->height++; // The 3 height has changed.
			}
			// Rebalanced, nobody above would notice.
			break;
		}
		top = top->bst_parent;
	}
}

// Return a bst_entry, this function is used to access or insert an element into the manager's BST.
struct bst_entry *bst_at(struct bst_manager *const manager, const char *key) {
	struct bst_node *curr, *new_node;
	int i;
	if (manager->root) {
		curr = manager->root;
		for (;;) {
			i = strcmp(key, curr->entry.key);
			if (i > 0) {
				if (curr->bst_right) {
					curr = curr->bst_right;
				} else {
					if (manager->search_only) {
						return 0;
					}
					i = 1;
					break;
				}
			} else if (i) {
				if (curr->bst_left) {
					curr = curr->bst_left;
				} else {
					if (manager->search_only) {
						return 0;
					}
					i = 0;
					break;
				}
			} else {
				manager->affected = 0;
				return &curr->entry;
			}
		}
	} else if (manager->search_only) {
		return 0;
	} else {
		curr = 0;
		i = -1;
	}
	const size_t bytes = 1 + strlen(key);
	// This may replace root.
	new_node = *(i >= 0 ? &curr->rel[i] : &manager->root) = malloc(sizeof(struct bst_node) + bytes);
	if (new_node == 0)
		return 0 ;
	memset(new_node, 0, sizeof(struct bst_node));
	new_node->entry.key = (char *) (1 + new_node);
	memcpy(new_node->entry.key, key, bytes);
	new_node->height = 1;
	new_node->bst_parent = curr;
	bst_rebalance(&manager->root, curr);
	++manager->count;
	manager->affected = 1;
	return &new_node->entry;
}

// This action is used to destroy the BST.
void bst_destroy(struct bst_manager *const manager) {
	struct bst_node *stack[64], *curr;
	unsigned char state[64];
	int index = 0;
	if (manager->root) {
		curr = manager->root;
		for (;;) {
			while (curr) {
				stack[index] = curr;
				state[index] = 0;
				++index;
				curr = curr->rel[0];
			}
			for (;;) {
				if (index == 0) {
					memset(manager, 0, sizeof(struct bst_manager));
					manager->affected = 1;
					return;
				}
				--index;
				curr = stack[index];
				if (state[index] == 0) {
					state[index++] = 1;
					curr = curr->rel[1];
					break;
				} else {
					free(curr);
				}
			}
		}
	} else {
		manager->search_only = manager->affected = 0;
	}
}

// All makers do the same thing, one allocation followed by a configuration of the node.
// The configuration is relative to the specification of the Turtle project.

struct ast_node *make_forward(struct ast_node *const expr) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_FORWARD;
	node->children[0] = expr;
	node->children_count = 1;
	return node;
}

struct ast_node *make_backward(struct ast_node *const expr) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_BACKWARD;
	node->children[0] = expr;
	node->children_count = 1;
	return node;
}

struct ast_node *make_up() {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_UP;
	return node;
}

struct ast_node *make_down() {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return node;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_DOWN;
	return node;
}

struct ast_node *make_color(unsigned long long int const number) {
	// The hexa notation is used because it's compatible with many devices.
	// The hexa notation is now converted into 3 double, each representing one RGB channel.
	struct ast_node *const r = make_value((double) (number >> 32) / 65535.0);
	struct ast_node *const g = make_value((double) (number >> 16 & 0xFFFF) / 65535.0);
	struct ast_node *const b = make_value((double) (number & 0xFFFF) / 65535.0);
	if (r && g && b)
		return make_raw_color(r, g, b);
	if (r) free(r);
	if (g) free(g);
	if (b) free(b);
	return 0 ;
}

struct ast_node *make_raw_color(struct ast_node *const expr_r, struct ast_node *const expr_g, struct ast_node *const expr_b) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_COLOR;
	node->children[0] = expr_r;
	node->children[1] = expr_g;
	node->children[2] = expr_b;
	node->children_count = 3;
	return node;
}

struct ast_node *make_left(struct ast_node *const expr) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_LEFT;
	node->children[0] = expr;
	node->children_count = 1;
	return node;
}

struct ast_node *make_right(struct ast_node *const expr) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_RIGHT;
	node->children[0] = expr;
	node->children_count = 1;
	return node;
}

struct ast_node *make_heading(struct ast_node *const expr) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_HEADING;
	node->children[0] = expr;
	node->children_count = 1;
	return node;
}

struct ast_node *make_position(struct ast_node *const expr_x, struct ast_node *const expr_y) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_POSITION;
	node->children[0] = expr_x;
	node->children[1] = expr_y;
	node->children_count = 2;
	return node;
}

struct ast_node *make_home() {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_HOME;
	return node;
}

struct ast_node *make_print(struct ast_node *const expr) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SIMPLE;
	node->u.cmd = CMD_PRINT;
	node->children[0] = expr;
	node->children_count = 1;
	return node;
}

struct ast_node *make_repeat(struct ast_node *const expr_count, struct ast_node *const to_repeat) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_REPEAT;
	node->children[0] = expr_count;
	node->children[1] = to_repeat;
	node->children_count = 2;
	return node;
}

struct ast_node *make_cmds_block(struct ast_node *const to_block) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_BLOCK;
	node->children[0] = to_block;
	node->children_count = 1;
	return node;
}

struct ast_node *make_call(struct bst_entry *const entry) {
	if (entry == 0)
		return 0;
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_CALL;
	node->u.bst_entry = entry; // everything is constant here, bst already holds the string
	return node;
}

struct ast_node *make_set(struct bst_entry *const entry, struct ast_node *const expr) {
	if (entry == 0)
		return 0;
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_SET;
	node->u.bst_entry = entry;
	node->children[0] = expr;
	node->children_count = 1;
	return node;
}

struct ast_node *make_proc(struct bst_entry *const entry, struct ast_node *const root) {
	if (entry == 0)
		return 0;
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_CMD_PROC;
	node->u.bst_entry = entry;
	node->children[0] = root;
	node->children_count = 1;
	return node;
}

struct ast_node *make_value(double const value) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_EXPR_VALUE;
	node->u.value = value;
	return node;
}

struct ast_node *make_name(struct bst_entry *const entry) {
	if (entry == 0)
		return 0;
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_EXPR_NAME;
	node->u.bst_entry = entry; // A BST entry is used to hold the identifier/name, it contains a constant key which is a char *
	return node;
}

struct ast_node *make_math_func(enum ast_func const func, struct ast_node *const expr) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_EXPR_FUNC;
	node->u.func = func;
	node->children[0] = expr;
	node->children_count = 1;
	return node;
}

struct ast_node *make_random(struct ast_node *const expr_low, struct ast_node *const expr_high) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_EXPR_FUNC;
	node->u.func = FUNC_RANDOM;
	node->children[0] = expr_low;
	node->children[1] = expr_high;
	node->children_count = 2;
	return node;
}

struct ast_node *make_expr_block(struct ast_node *const to_block) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_EXPR_BLOCK;
	node->children[0] = to_block;
	node->children_count = 1;
	return node;
}

struct ast_node *make_binop(char const op, struct ast_node *const lhs, struct ast_node *const rhs) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_EXPR_BINOP;
	node->u.op = op;
	node->children[0] = lhs;
	node->children[1] = rhs;
	node->children_count = 2;
	return node;
}

struct ast_node *make_unop(char const op, struct ast_node *const expr) {
	struct ast_node *const node = calloc(1, sizeof(struct ast_node));
	if (node == 0)
		return 0;
	node->kind = KIND_EXPR_UNOP;
	node->u.op = op;
	node->children[0] = expr;
	node->children_count = 1;
	return node;
}

// Destroying the AST tree using recursion.
void destroy_node(struct ast_node *const node) {
	struct ast_node *next = node->next;
	for (int i = 0 ; i < node->children_count ; ++i)
		destroy_node(node->children[i]);
	free(node);
	if (next)
		destroy_node(next);
}

// Initiate the AST (abstract syntax tree) destruction.
void ast_destroy(struct ast *const self) {
	if (self) {
		if (self->unit)
			destroy_node(self->unit);
		bst_destroy(&self->parsing);
	}
}

/*
 * context
 */

// A context is nothing without an AST, it's simply zeroed by this function.
void context_create(struct context *const self) {
	memset(self, 0, sizeof(struct context));
}

// A context is destroyed by destroying its 2 trees (variables and procedures).
int context_destroy(struct context *const ctx) {
	bst_destroy(&ctx->variables);
	bst_destroy(&ctx->procedures);
	return ctx->error_number;
}

// Evaluation of an AST, with a given context.
// The context and the AST must be free of previous errors.
// 3 variables are set here, it will be possible update their value during the program execution.
void ast_eval(const struct ast *const self, struct context *const ctx) {
	if (self == 0 || self->error_number || ctx->error_number)
		return;
	// the ratio of the circumference of any circle to the diameter of that circle.
	struct bst_entry *entry = bst_at(&ctx->variables, "PI");
	entry->value.number = PI;
	// the length of the hypotenuse of an isosceles right triangle with legs of length 1.
	entry = bst_at(&ctx->variables, "SQRT2");
	entry ->value.number = SQRT2;
	// the positive real number that, when multiplied by itself, gives the number 3.
	entry = bst_at(&ctx->variables, "SQRT3");
	entry ->value.number = SQRT3;
	ast_eval_node(ctx, self->unit);
}

// This action is used, given a context to write the program output to STDOUT.
// It will only write something if necessary AND if no error was previously found.
void ast_eval_write_output(struct context *ctx) {
	if (ctx->error_number)
		return;
	if (ctx->let.r != ctx->r || ctx->let.g != ctx->g || ctx->let.b != ctx->b) {
		ctx->r = ctx->let.r;
		ctx->g = ctx->let.g;
		ctx->b = ctx->let.b;
		++ctx->lines_printed;
		fprintf(stdout, "Color\t%7.4f %7.4f %7.4f\n", ctx->r, ctx->g, ctx->b);
	}
	if (ctx->let.x != ctx->x || ctx->let.y != ctx->y) {
		ctx->x = ctx->let.x ;
		ctx->y = ctx->let.y ;
		++ctx->lines_printed;
		fputs(ctx->up ? "MoveTo\t" : "LineTo\t", stdout);
		fprintf(stdout, "%7.4f %7.4f\n", ctx->x, ctx->y);
	}
}

// This "eval" action is a simple switch that call the appropriate functions.
// It's calling itself over the "next" node after the current node evaluation is done, so it's recursive.
void ast_eval_node(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return;
	switch (node->kind) {
		case KIND_CMD_SIMPLE :
			switch (node->u.cmd) {
				case CMD_FORWARD: ast_eval_forward(ctx, node); break;
				case CMD_BACKWARD: ast_eval_backward(ctx, node); break;
				case CMD_UP : ast_eval_up(ctx); break;
				case CMD_DOWN : ast_eval_down(ctx); break;
				case CMD_COLOR : ast_eval_color(ctx, node); break;
				case CMD_LEFT: ast_eval_left(ctx, node); break;
				case CMD_RIGHT: ast_eval_right(ctx, node); break;
				case CMD_HEADING: ast_eval_heading(ctx, node); break;
				case CMD_POSITION: ast_eval_position(ctx, node); break;
				case CMD_HOME: ast_eval_home(ctx); break;
				case CMD_PRINT : ast_eval_print(ctx, node); break;
			}
			break;
		case KIND_CMD_REPEAT : ast_eval_repeat(ctx, node); break;
		case KIND_CMD_BLOCK : ast_eval_block(ctx, node); break;
		case KIND_CMD_CALL : ast_eval_call(ctx, node); break;
		case KIND_CMD_SET : ast_eval_set(ctx, node); break;
		case KIND_CMD_PROC : ast_eval_proc(ctx, node); break;
		default:
			ctx->error_number = 2;
			fprintf(stderr, "Unknown node to eval.");
	}
	if (node->next)
		ast_eval_node(ctx, node->next);
}

// Evaluate an expression, many possibilities at this point :
// - if the node is a simple value, then the value will be returned.
// - if the node is a simple name, then the corresponding value will be retrieved in O(log N), and returned.
// - if the node is a binary operation, both LHS and RHS are resolved recursively, and the result is returned.
// - if the node is a math function, the function will be executed after resolving it's argument, and the result returned.

// Error cases :
// - the error cases are handled by writing a message to STDERR + setting ctx->error_number to a non-zero value.
double ast_eval_expr(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return 0;
	if (node->kind == KIND_EXPR_VALUE)
		return node->u.value;
	if (node->kind == KIND_EXPR_NAME) {
		ctx->variables.search_only = 1;
		struct bst_entry *entry = bst_at(&ctx->variables, node->u.bst_entry->key);
		if (entry)
			return entry->value.number;
		else {
			ctx->error_number = 3;
			fprintf(stderr, "Unknown variable '%s'.", node->u.bst_entry->key);
			return 0;
		}
	}
	double lhs = ast_eval_expr(ctx, node->children[0]);
	if (node->kind == KIND_EXPR_BINOP) {
		const double rhs = ast_eval_expr(ctx, node->children[1]);
		double res;
		switch (node->u.op) {
			case '+' : res = lhs + rhs; break;
			case '-' : res = lhs - rhs; break;
			case '*' : res = lhs * rhs; break;
			case '/' : res = lhs / rhs; break;
			case '^' :
			default : res = pow(lhs, rhs); break;
		}
		if (isfinite(res))
			return res;
		ctx->error_number = 4;
		fprintf(stderr, "Operator '%c' failed because %g%c%g = %g isn’t finite, only integers from −2^53 to 2^53 are exactly represented.", node->u.op, lhs, node->u.op, rhs, res);
		return 0;
	}
	if (node->kind == KIND_EXPR_BLOCK)
		return lhs;
	if (node->kind == KIND_EXPR_UNOP)
		return lhs && node->u.op == '-' ? -lhs : lhs;
	if (node->kind == KIND_EXPR_FUNC) {
		switch (node->u.func) {
			case FUNC_RANDOM :;
				double rhs = ast_eval_expr(ctx, node->children[1]);
				if (lhs == rhs)
					return lhs;
				if (lhs > rhs) {
					ctx->error_number = 5;
					fprintf(stderr, "Function random(%.1f, %.1f) failed the 'ordered arguments' check.", lhs, rhs);
					return 0;
				}
				return lhs + (double) rand() / (double) RAND_MAX * (rhs - lhs);
				// [ lhs=0, rhs=1 ] yield a number greater than or equal to 0 and less than or equal to 1
			case FUNC_COS : return cos(lhs * TURTLE_DEG_TO_RAD);
			case FUNC_SIN : return sin(lhs * TURTLE_DEG_TO_RAD);
			case FUNC_TAN : return tan(lhs * TURTLE_DEG_TO_RAD);
			case FUNC_SQRT:
			default :
				if (lhs < 0.0) {
					ctx->error_number = 6;
					fprintf(stderr, "Function sqrt(%g) failed the 'argument greater or equal than zero' check.", lhs);
					return 0;
				}
				return sqrt(lhs);
		}
	}
	return 0;
}

// This action evaluate a "forward" node, it calls the writer if necessary.
void ast_eval_forward(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return;
	const double value = ast_eval_expr(ctx, node->children[0]);
	if (value) {
		ctx->let.x -= value * sin(ctx->angle * TURTLE_DEG_TO_RAD);
		ctx->let.y -= value * cos(ctx->angle * TURTLE_DEG_TO_RAD);
		ast_eval_write_output(ctx);
	}
}

// This action evaluate a "backward" node, it calls the writer if necessary.
void ast_eval_backward(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return;
	const double value = ast_eval_expr(ctx, node->children[0]);
	if (value) {
		ctx->let.x += value * sin(ctx->angle * TURTLE_DEG_TO_RAD);
		ctx->let.y += value * cos(ctx->angle * TURTLE_DEG_TO_RAD);
		ast_eval_write_output(ctx);
	}
}

// This action update the current state of the pen, after it the pen is UP (not writing).
void ast_eval_up(struct context *ctx) {
	if (ctx->error_number == 0)
		ctx->up = true;
}

// This action update the current state of the pen, after it the pen is DOWN (writing).
void ast_eval_down(struct context *ctx) {
	if (ctx->error_number == 0)
		ctx->up = false;
}

// This action evaluates a color, which must be rightly specified.
// In the normal case the current color is updated, but will not be written to STDOUT (a PEN move will trigger it).
// An error is shown on stderr if anything's wrong.
void ast_eval_color(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return;
	ctx->let.r = ast_eval_expr(ctx, node->children[0]);
	ctx->let.g = ast_eval_expr(ctx, node->children[1]);
	ctx->let.b = ast_eval_expr(ctx, node->children[2]);
	if (ctx->let.r < 0.0 || ctx->let.r > 1.0 || ctx->let.g < 0.0 || ctx->let.g > 1.0 || ctx->let.b < 0.0 || ctx->let.b > 1.0) {
		const char *channel = ctx->let.r < 0.0 || ctx->let.r > 1.0 ? "RED" : ctx->let.g < 0.0 || ctx->let.g > 1.0 ? "GREEN" : "BLUE";
		fprintf(stderr, "Color in red/green/blue format is out of range on channel %s.\nUse 3 numbers in [0, 1] or a specify a keyword (red, green, blue, cyan, magenta, yellow, black, gray, white) to use a color.", channel);
		ctx->error_number = 7;
	}
}

// This action simply update the current angle of the Turtle.
void ast_eval_left(struct context *ctx, struct ast_node *node) {
	if (node && ctx->error_number == 0)
		ctx->angle += ast_eval_expr(ctx, node->children[0]);
}

// This action simply update the current angle of the Turtle.
void ast_eval_right(struct context *ctx, struct ast_node *node) {
	if (node && ctx->error_number == 0)
		ctx->angle -= ast_eval_expr(ctx, node->children[0]);
}

// This action simply update the current heading of the Turtle.
void ast_eval_heading(struct context *ctx, struct ast_node *node) {
	if (node && ctx->error_number == 0)
		ctx->angle = -ast_eval_expr(ctx, node->children[0]);
}

// This action updates the absolute position of the Turtle, and write the change to STDOUT.
void ast_eval_position(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return;
	ctx->let.x = ast_eval_expr(ctx, node->children[0]);
	ctx->let.y = ast_eval_expr(ctx, node->children[1]);
	ast_eval_write_output(ctx);
}

// This action reset all the PEN parameters.
// After it every parameter is equal to zero, the color is black.
// Any change will be written to STDOUT
void ast_eval_home(struct context *ctx) {
	if (ctx->error_number)
		return;
	ctx->angle = ctx->up = 0;
	ctx->let.r = ctx->let.g = ctx->let.b = 0;
	ctx->let.x = ctx->let.y = 0;
	ast_eval_write_output(ctx);
}

// This action is used for debug, its able to write the value of a node as a double.
void ast_eval_print(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return;
	fprintf(stderr, "%g\n", ast_eval_expr(ctx, node->children[0]));
}

// This action is performing a for loop for the user, executing the previously specified node at every iteration.
// There is a limit, the loop can't be longer than a very large configurable number (see TURTLE_REPEAT_MAX_ITERATIONS).
void ast_eval_repeat(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number) return ;
	const double value = ast_eval_expr(ctx, node->children[0]);
	if (ctx->error_number) return ;
	if (value > TURTLE_REPEAT_MAX_ITERATIONS) {
		fprintf(stderr, "Command repeat %g ... failed : argument is greater than a safety limit of %lli iterations.", value, TURTLE_REPEAT_MAX_ITERATIONS);
		ctx->error_number = 8;
	} else if (value < 0) {
		// do nothing.
	} else if (value >= 1) {
		const long long int count = (long long int) value;
		for (long long int i = 0 ; i < count && !ctx->error_number ; ++i) {
			ast_eval_node(ctx, node->children[1]);
		}
	}
}

// This action is a wrapper, is simply evaluated normally.
void ast_eval_block(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return;
	ast_eval_node(ctx, node->children[0]);
}

// This action is executing a call, the user want to call a procedure.
// The action is searching the procedure in the context BST, the complexity of the search O(log N).
// If found the procedure will be executed, otherwise the program will abort with an error to STDERR.
void ast_eval_call(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return;
	ctx->procedures.search_only = 1;
	struct bst_entry *entry = bst_at(&ctx->procedures, node->u.bst_entry->key);
	if (entry == 0) {
		ctx->error_number = 9;
		fprintf(stderr, "Procedure '%s' does not exists.", node->u.bst_entry->key);
		return;
	} else {
		struct ast_node *target = entry->value.node;
		++ctx->nested_call_count;
		ast_eval_node(ctx, target);
		--ctx->nested_call_count;
	}
}

// This action is used to create or update a variable.
// - The variable is constructed bt bst_at or retrieved by bst_at (return a bst_entry).
// - Then the value is resolved (maybe recursively) by ast_eval_expr (return a double).
// - Then the double value is registered into the bst_entry for a future usage.
void ast_eval_set(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return;
	ctx->variables.search_only = 0 ;
	struct bst_entry *entry = bst_at(&ctx->variables, node->u.bst_entry->key);
	if (entry)
		entry ->value.number = ast_eval_expr(ctx, node->children[0]);
	else {
		ctx->error_number = 10;
	}
}

// This action is used to create a procedure.
// - The procedure is constructed bt bst_at (return a bst_entry).
// - Then the corresponding node is registered into the bst_entry for a future usage.
// There is multiple error cases:
// - if the procedure already exist the program will abort with a message to STDERR.
// - if the procedure is created inside another procedure the program will abort with a message to STDERR.
void ast_eval_proc(struct context *ctx, struct ast_node *node) {
	if (node == 0 || ctx->error_number)
		return;
	if (ctx->nested_call_count) {
		ctx->error_number = 11;
		fprintf(stderr, "Procedure '%s' declaration failed : nested procedure are not allowed.", node->u.bst_entry->key);
		return;
	}
	ctx->procedures.search_only = 0;
	struct bst_entry *entry = bst_at(&ctx->procedures, node->u.bst_entry->key);
	if (entry) {
		if (ctx->procedures.affected)
			entry->value.node = node->children[0];
		else
			fprintf(stderr, "Procedure '%s' declaration failed : the procedure already exists.", node->u.bst_entry->key);
	}
	else {
		ctx->error_number = 12;
	}
}

/*
 * Print, do not execute.
 * These functions are performing a tree traversal.
 * The source code is interpreted and just printed to STDERR, it's used for debug.
 * These functions are simple, they don't check many things but just print.
 */

void ast_print(const struct ast *self) {
	ast_print_node(self->unit);
}

void ast_print_node(struct ast_node *node) {
	if (!node) {
		fputs("Null Pointer.\n", stderr);
		return;
	}
	switch (node->kind) {
		case KIND_CMD_SIMPLE :
			switch (node->u.cmd) {
				case CMD_FORWARD: ast_print_forward(node); break;
				case CMD_BACKWARD: ast_print_backward(node); break;
				case CMD_UP : ast_print_up(); break;
				case CMD_DOWN : ast_print_down(); break;
				case CMD_COLOR : ast_print_color(node); break;
				case CMD_LEFT: ast_print_left(node); break;
				case CMD_RIGHT: ast_print_right(node); break;
				case CMD_HEADING: ast_print_heading(node); break;
				case CMD_POSITION: ast_print_position(node); break;
				case CMD_HOME: ast_print_home(); break;
				case CMD_PRINT : ast_print_print(node); break;
			}
			break;
		case KIND_CMD_REPEAT : ast_print_repeat(node); break;
		case KIND_CMD_BLOCK : ast_print_block(node); break;
		case KIND_CMD_CALL : ast_print_call(node); break;
		case KIND_CMD_SET : ast_print_set(node); break;
		case KIND_CMD_PROC : ast_print_proc(node); break;
		default:
			fputs("Unknown node", stderr);
	}
	fputc('\n', stderr);
	if (node->next)
		ast_print_node(node->next);
}

void ast_print_expr(struct ast_node *node) {
	switch (node->kind) {
		case KIND_EXPR_VALUE :
			fprintf(stderr, "%g", node->u.value);
			break;
		case KIND_EXPR_NAME:
			fputs(node->u.bst_entry->key, stderr);
			break;
		case KIND_EXPR_BINOP :
			ast_print_expr(node->children[0]);
			fprintf(stderr, " %c ", node->u.op);
			ast_print_expr(node->children[1]);
			break;
		case KIND_EXPR_BLOCK :
			ast_print_expr(node->children[0]);
			break;
		case KIND_EXPR_UNOP :
			fputc(node->u.op, stderr);
			ast_print_expr(node->children[0]);
			break;
		case KIND_EXPR_FUNC :
		default:
			switch (node->u.func) {
				case FUNC_RANDOM :
					fputs("random(", stderr);
					ast_print_expr(node->children[0]);
					fputs(", ", stderr);
					ast_print_expr(node->children[1]);
					break;
				case FUNC_COS :
					fputs("cos(", stderr);
					ast_print_expr(node->children[0]);
					break;
				case FUNC_SIN :
					fputs("sin(", stderr);
					ast_print_expr(node->children[0]);
					break;
				case FUNC_TAN :
					fputs("tan(", stderr);
					ast_print_expr(node->children[0]);
					break;
				case FUNC_SQRT:
				default :
					fputs("sqrt(", stderr);
					ast_print_expr(node->children[0]);
					break;
			}
			fputs(")", stderr);
	}
}

void ast_print_forward(struct ast_node *node) {
	fputs("fw ", stderr);
	ast_print_expr(node->children[0]);
}

void ast_print_backward(struct ast_node *node) {
	fputs("bw ", stderr);
	ast_print_expr(node->children[0]);
}

void ast_print_up() {
	fputs("up", stderr);
}

void ast_print_down() {
	fputs("down ", stderr);
}

void ast_print_color(struct ast_node *node) {
	fputs("color ", stderr);
	ast_print_expr(node->children[0]);
	fputs(", ", stderr);
	ast_print_expr(node->children[1]);
	fputs(", ", stderr);
	ast_print_expr(node->children[2]);
}

void ast_print_left(struct ast_node *node) {
	fputs("left ", stderr);
	ast_print_expr(node->children[0]);
}

void ast_print_right(struct ast_node *node) {
	fputs("right ", stderr);
	ast_print_expr(node->children[0]);
}

void ast_print_heading(struct ast_node *node) {
	fputs("heading ", stderr);
	ast_print_expr(node->children[0]);
}

void ast_print_position(struct ast_node *node) {
	fputs("pos ", stderr);
	ast_print_expr(node->children[0]);
	fputs(", ", stderr);
	ast_print_expr(node->children[1]);
}

void ast_print_home() {
	fputs("home ", stderr);
}

void ast_print_print(struct ast_node *node) {
	fputs("print ", stderr);
	ast_print_expr(node->children[0]);
}

void ast_print_repeat(struct ast_node *node) {
	fputs("repeat ", stderr);
	ast_print_expr(node->children[0]);
	fputs(" ", stderr);
	ast_print_node(node->children[1]);
}

void ast_print_block(struct ast_node *node) {
	fputs("{\n", stderr);
	if (node->children[0]) {
		ast_print_node(node->children[0]);
	} else
		fputs("Empty Node.", stderr);
	fputs("}", stderr);
}

void ast_print_call(struct ast_node *node) {
	fputs("call ", stderr);
	fputs(node->u.bst_entry->key, stderr);
}

void ast_print_set(struct ast_node *node) {
	fprintf(stderr, "set %s ", node->u.bst_entry->key);
	ast_print_expr(node->children[0]);
}

void ast_print_proc(struct ast_node *node) {
	fputs("proc ", stderr);
	fputs(node->u.bst_entry->key, stderr);
	fputc('\n', stderr);
	ast_print_node(node->children[0]);
}
