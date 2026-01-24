#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include "../../../src/ast/ast.h"

// TestSuite definition
TestSuite(TestPipe);

Test(TestPipe, init_basic)
{
    struct ast_pipe *pipe = ast_pipeline_init(0);
    
    cr_assert_not_null(pipe);
    cr_assert_eq(pipe->base.type, AST_PIPE);
    cr_assert_eq(pipe->negated, 0);
    cr_assert_null(pipe->pipelines);
    cr_assert_null(pipe->redirections);
    
    ast_pipe_destroy(pipe);
}

Test(TestPipe, init_negated)
{
    struct ast_pipe *pipe = ast_pipeline_init(1);
    
    cr_assert_eq(pipe->negated, 1);
    
    ast_pipe_destroy(pipe);
}

Test(TestPipe, with_pipeline_list)
{
    struct ast_pipe *pipe = ast_pipeline_init(0);
    struct ast_list *list = ast_list_init();
    
    pipe->pipelines = list;
    
    cr_assert_not_null(pipe->pipelines);
    
    ast_pipe_destroy(pipe);
}

Test(TestPipe, destroy_null)
{
    ast_pipe_destroy(NULL);
}