#ifndef PRETTY_PRINTER_H
#define PRETTY_PRINTER_H

#include <stdio.h>
#include "../ast/ast.h"

void pretty_print_ast (struct ast *node);

void pretty_print_ast_cmd (struct ast_cmd *node);
void pretty_print_ast_if (struct ast_if *node);
void pretty_print_ast_list (struct ast_list *node);

#endif /* ! PRETTY_PRINTER_H */