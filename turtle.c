#include <time.h>
#include <assert.h>

#include "turtle-ast.h"
#include "turtle-lexer.h"
#include "turtle-parser.h"

// I used the following link to draw my programs :
// https://en.wikipedia.org/wiki/T-square_(fractal)

int main() {
	// yydebug = 1 ;
	srand(time(NULL));
	struct ast root = {0};
	int ret = yyparse(&root);
	yylex_destroy();
	if (ret == 0) {
		assert(root.unit);
		struct context ctx = {0};
		context_create(&ctx);
		ast_eval(&root, &ctx);
		ret = context_destroy(&ctx);
		// ast_print(&root);
		if (ret == 1)
			fprintf(stderr, "Memory Allocation Error.\n");
	}
	ast_destroy(&root);
	return ret;
}
