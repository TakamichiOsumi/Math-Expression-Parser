#include <stdio.h>
#include <stdlib.h>
#include "MexprTree.h"
#include "ExportedParser.h"

/* Application data definition */
typedef struct app_data {
    char *name;
    char *val;
} app_data;

app_data app_array[] = {
    { .name = "a", .val = "1" },
    { .name = "b", .val = "3.0" },
    { .name = "c", .val = "5" },
    { .name = "d", .val = "-1" },
};

static tr_node *
app_fetch_data(variable *v, void *data){
    app_data *ary = (app_data *) data;
    tr_node *trn = gen_null_tr_node();

    if (strncmp(v->vname, "a", strlen("a")) == 0){
	trn->node_id = INT;
	trn->unv.ival = strtol(ary[0].val, (char **) NULL, 10);
    }else if (strncmp(v->vname, "b", strlen("b")) == 0){
	trn->node_id = DOUBLE;
	trn->unv.dval = strtod(ary[1].val, (char **) NULL);
    }else if (strncmp(v->vname, "c", strlen("c")) == 0){
	trn->node_id = INT;
	trn->unv.ival = strtol(ary[2].val, (char **) NULL, 10);
    }else if (strncmp(v->vname, "d", strlen("d")) == 0){
	trn->node_id = INT;
	trn->unv.ival = strtol(ary[3].val, (char **) NULL, 10);
    }else{
	/* Application should not pass any other name of data */
	assert(0);
    }

    return trn;
}

int
main(int argc, char **argv){

    resolve_and_evaluate_test(start_mathexpr_parse,
			      "((1 + 2) - 3) * 4 / (5 / 6) + (7 - 8) \n",
			      NULL, NULL);
    resolve_and_evaluate_test(start_mathexpr_parse,
			      "sqr(3) + min(10, 0) + sqrt(25.0)\n",
			      NULL, NULL);
    resolve_and_evaluate_test(start_ineq_mathexpr_parse,
			      "a <= 100\n", app_array, app_fetch_data);
    resolve_and_evaluate_test(start_ineq_mathexpr_parse,
			      "b <= c\n", app_array, app_fetch_data);

    return 0;
}
