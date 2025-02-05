#include "pipelinesetup.h"
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>

void createGraphicsPipeline(CorePipeline* pipeline, const char* vertexPath, const char* fragPath) {
    char* vertex_code = readShader(vertexPath);
    char* frag_code = readShader(fragPath);
    if (!vertex_code || !frag_code)
        return;

    #ifndef NDEBUG
    printf("Successfully read shaders\n");
    #endif

    free(vertex_code);
    free(frag_code);
}
