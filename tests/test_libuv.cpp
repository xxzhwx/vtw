#include "tests/test.h"

#include "uv.h"

void Test_LibUv()
{
    printf("uv version: %s\n", uv_version_string());

    uv_loop_t *loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
    uv_loop_init(loop);

    uv_idle_t idle;
    uv_idle_init(loop, &idle);
    uv_idle_start(&idle, [](uv_idle_t* i) {
        printf("idle ...\n");
        uv_idle_stop(i);
    });

    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);
}
