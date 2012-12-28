#include <stdio.h>
#include <stdlib.h>
#include "target.h"
#include "quads.h"
#include "globals.h"
#include "symtable.h"
#include "statements.h"
#include "expressions.h"
#include "declarations.h"
#include "y.tab.h"

struct string_lit *strings;
struct global *globals;
struct func_list *funcs;

print_target_code() {
	print_globals();
	print_strings();
	print_functions();
}

print_globals() {
	struct global *g = globals;
	struct symbol *sym;

	/*printf("\t.data\n");*/
	while(g = g->next) {
		sym = (struct symbol *)g->var;

		/* If globals could be initialized:
		printf("\t.align 4\n");
		printf("\t.type %s,@object\n",sym->id);
		printf("\t.size %s,",sym->id);
		*/

		printf("\t.comm %s,",sym->id);
		if (sym->type->nodetype == N_ARR)
			printf("%d",get_size_of_arr((struct arr_node *)(sym->type)));
		else
			printf("4");
		printf(",4");
		putchar('\n');
	}
	putchar('\n');
	return;
}

print_strings() {
	struct string_lit *s = strings;
	printf("\t.section\t.rodata\n");
	while(s = s->next) {
		printf(".LC%d:\n",s->num);
		printf("\t.string\t\"%s\"\n",s->str);
	}
	putchar('\n');
}

print_functions() {
	struct func_list *fl = funcs;
	int fnum = 0;
	while (fl = fl->next) {
		// Function header
		printf("\t.text\n");
		printf("\t.globl\t%s\n",fl->id);
		printf("\t.type\t%s, @function\n",fl->id);

		printf("%s:\n",fl->id); // function name
		printf("\tpushl %%ebp\n");
		printf("\tmovl %%esp, %%ebp\n");
		printf("\t.subl $%d, %%esp\n",(fl->num_locals+1)*4);

		print_function_body(fl->bb);

		// Function footer
		printf("\tleave\n\tret\n");
		printf(".LFE%d:\n",fnum);
		printf("\t.size %s, .-%s\n",fl->id,fl->id);
		fnum++;
	}
	putchar('\n');
}

print_function_body(struct block *bb) {
	struct quad *q;
	while (bb) {
		printf("%s:\n",bb->id);
		q = bb->first;

		// Go through the quads in the block
		while (q) {
			print_target_from_quad(q);
			q = q->next;
		}

		// Next basic block
		bb = bb->next;
	}
}

print_target_from_quad(struct quad *q) {
	struct symbol *r, *s1, *s2;
	r = (struct symbol *)q->result;
	s1 = (struct symbol *)q->src1;
	s2 = (struct symbol *)q->src2;

	switch(q->opcode) {
	case Q_FUNC_CALL:
		// handle assignment
		// PUSH IN REVERSE ORDER
		printf("\tcall %s\n",s1->id);
		break;
	case Q_FUNC_ARG:
		printf("\tmovl %s,%%eax\n",s1->id);
		printf("\tpushl %%eax\n");
		break;
	default:
		break;
	}
}

