#include "Renderer.h"
#include "Math.h"
#include "Assets.h"

#include <GL/glew.h>
#include <malloc.h>

typedef struct {
    real32 x;
    real32 y;
    real32 z;
} render_vertex;

typedef struct {
    real32 r;
    real32 g;
    real32 b;
} render_color;

typedef struct render_context {
    bool initialized;
    bool rendering;

    memory_segment memory;
    memory_segment vertex_buffer;
    memory_segment color_buffer;
    memory_segment uv_buffer;

    uint32 entries_count;
    uint32 entries_max;

    // TODO: Remove this
    uint32 texture_width;
    uint32 texture_height;
    void *texture_data;
} render_context;

static void draw(render_context *ctx);

static void initialize_opengl(game_assets *assets) {
    GLenum err = glewInit();
    Assert(err == GLEW_OK);
    if (err != GLEW_OK)
    {
        //Problem: glewInit failed, something is seriously wrong.
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

    char *vs, *fs;

    // load shaders & get length of each
    uint32 vlen_fs;
    uint32 flen_fs;
    vs = get_shader(assets, ASSET_SHADER_VERTEX, &vlen_fs);
    fs = get_shader(assets, ASSET_SHADER_FRAGMENT, &flen_fs);

    GLint vlen = (GLint)vlen_fs;
    GLint flen = (GLint)flen_fs;
    const char * vv = vs;
    const char * ff = fs;
    glShaderSource(v, 1, &vv, &vlen);
    glShaderSource(f, 1, &ff, &flen);
    GLint compiled;

    glCompileShader(v);
    glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
    Assert(compiled);

    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
    Assert(compiled);

    GLuint p = glCreateProgram();
    glBindAttribLocation(p, 0, "in_position");
    glBindAttribLocation(p, 1, "in_color");
    glBindAttribLocation(p, 2, "in_vertexUV");

    glAttachShader(p, v);
    glAttachShader(p, f);

    glLinkProgram(p);
    glUseProgram(p);

    //GLuint res = glGetAttribLocation(p, "in_vertexUV");
}


void load_texture(game_assets *assets, render_context *ctx) {
    asset_image img = get_image(assets, ASSET_IMAGE_SPRITEMAP);
    Assert(img.data);

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.data);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}

render_context *render_init(game_assets *assets, memory_segment memory) {
    allocate_memory(&memory, sizeof(render_context));

    render_context *ctx = (render_context*)memory.base;
    ctx->initialized = false;
    ctx->rendering = false;
    ctx->memory = memory;

    initialize_opengl(assets);

    // Allocate memory buffer
    uint32 size_per_element = sizeof(real32) * 3 * 4; // 3 real32 coords, 4 vertices
    uint32 available_memory = (memory.size - memory.used) / size_per_element;
    memory_segment vertex_segment = allocate_memory(&memory, available_memory / 3);
    memory_segment color_segment = allocate_memory(&memory, available_memory / 3);
    memory_segment uv_segment = allocate_memory(&memory, available_memory / 3);

    ctx->entries_max = available_memory / (3 * sizeof(real32) * 3 * 4);
    ctx->entries_count = 0;
    ctx->vertex_buffer = vertex_segment;
    ctx->color_buffer = color_segment;
    ctx->uv_buffer = uv_segment;

    load_texture(assets, ctx);

    ctx->initialized = true;
    return ctx;
}

struct uv_coords {
    real32 u;
    real32 v;
};

void render_rect(render_context *ctx, int32 x, int32 y, int32 width, int32 height, color c) {
    Assert(ctx->entries_count < ctx->entries_max);
    
    render_vertex vertices[4];
    vertices[0] = { (real32)x, (real32)y, 0.f };
    vertices[1] = { (real32)x, (real32)(y + height), 0.f };
    vertices[2] = { (real32)(x + width), (real32)(y + height), 0.f };
    vertices[3] = { (real32)(x + width), (real32)y, 0.f };

    uv_coords coords[4];
    coords[0] = { 0.0f, 0.0f };
    coords[1] = { 0.0f, -1.0f };
    coords[2] = { 1.0f, -1.0f };
    coords[3] = { 1.0f, 0.0f };

    render_color colors[4];
    for (int i = 0; i < 4; ++i) {
        colors[i] = { c.r, c.g, c.b };
    }

    for (int i = 0; i < 4; ++i) {
        render_vertex *c_vert = PUSH_STRUCT(&ctx->vertex_buffer, render_vertex);
        *c_vert = vertices[i];

        render_color *c_color = PUSH_STRUCT(&ctx->color_buffer, render_color);
        *c_color = colors[i];

        uv_coords *c_coords = PUSH_STRUCT(&ctx->uv_buffer, uv_coords);
        *c_coords = coords[i];
    }

    ctx->entries_count++;
}

void render_rect(render_context *ctx, v2 pos, v2 size, color c) {
    render_rect(ctx, (int32)pos.x, (int32)pos.y, (int32)size.x, (int32)size.y, c);
}

void render_start(render_context *ctx) {
    Assert(!ctx->rendering);
    Assert(ctx->vertex_buffer.base != NULL);
    Assert(ctx->color_buffer.base != NULL);
    Assert(ctx->entries_count == 0);

    ctx->rendering = true;
}

void render_end(render_context *ctx) {
    Assert(ctx->rendering);

    draw(ctx);

    segment_clear(&ctx->vertex_buffer);
    segment_clear(&ctx->color_buffer);
    segment_clear(&ctx->uv_buffer);

    ctx->entries_count = 0;
    ctx->rendering = false;
}

void draw(render_context *ctx) {    

    unsigned int vertexArrayObjId;
    unsigned int vertexBufferObjID[2];
    glGenVertexArrays(1, &vertexArrayObjId);
    glBindVertexArray(vertexArrayObjId);
    glGenBuffers(2, vertexBufferObjID);

    // VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[0]);
    glBufferData(GL_ARRAY_BUFFER, ctx->entries_count * 4*3*sizeof(real32), ctx->vertex_buffer.base, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // VBO for color data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[1]);
    glBufferData(GL_ARRAY_BUFFER, ctx->entries_count * 4*3*sizeof(GLfloat), ctx->color_buffer.base, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // UV buffer
    GLuint uv_id;
    glGenBuffers(1, &uv_id);
    glBindBuffer(GL_ARRAY_BUFFER, uv_id);
    glBufferData(GL_ARRAY_BUFFER, ctx->entries_count * 4 * 2 * sizeof(GLfloat), ctx->uv_buffer.base, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // Cheating! We probably want this from somewhere else...
    GLint viewport_size[4];
    glGetIntegerv(GL_VIEWPORT, viewport_size);

    GLfloat width = (GLfloat)viewport_size[2];
    GLfloat height = (GLfloat)viewport_size[3];
    GLfloat size[2] = { width, height };

    GLint id;
    glGetIntegerv(GL_CURRENT_PROGRAM, &id);

    GLint loc = glGetUniformLocation(id, "size");
    if (loc != -1)
    {
        glUniform2f(loc, size[0], size[1]);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if (ctx->initialized) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glBindVertexArray(vertexArrayObjId);
        glDrawArrays(GL_QUADS, 0, 4 * ctx->entries_count);

        glBindVertexArray(0);
    }

    glDeleteBuffers(2, vertexBufferObjID);
    glDeleteVertexArrays(1, &vertexArrayObjId);
    glDeleteBuffers(1, &uv_id);
    
}
