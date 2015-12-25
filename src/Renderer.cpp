#include "Renderer.h"
#include "Math.h"

#include <GL/glew.h>
#include <malloc.h>

typedef struct render_entry {
    uint32 vab;
    uint32 vbo[2];
} render_entry;

typedef struct render_buffer {
    render_entry *entries;
    uint32 count;
} render_buffer;

typedef struct render_context {
    bool initialized;
    bool rendering;
    render_entry *sprites;
    render_buffer buffer;

    memory_segment memory;
    memory_segment vertex_buffer;
    memory_segment color_buffer;

    uint32 entries_count;
    uint32 entries_max;
} render_context;

static void draw(render_context *ctx);

void PrepareOpenGL(platform_api *api) {
    GLenum err = glewInit();
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
    vs = api->ReadEntireFile("../data/shaders/minimal.vert", &vlen_fs);
    fs = api->ReadEntireFile("../data/shaders/minimal.frag", &flen_fs);

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
    glBindAttribLocation(p, 0, "in_Position");
    glBindAttribLocation(p, 1, "in_Color");

    glAttachShader(p, v);
    glAttachShader(p, f);

    glLinkProgram(p);
    glUseProgram(p);

    GLuint res = glGetAttribLocation(p, "in_Position");
    // TODO: FREE MEMORY!
    // VirtualFree(vs, 0, MEM_RELEASE);
    // VirtualFree(fs, 0, MEM_RELEASE);
}

static void add_entry(render_context *ctx, render_entry entry) {
    *(ctx->buffer.entries + ctx->buffer.count) = entry;
    ctx->buffer.count++;
}

// TODO: Initialization
render_context *render_init(platform_api *api, memory_segment memory) {
    allocate_memory(&memory, sizeof(render_context));

    render_context *ctx = (render_context*)memory.base;
    ctx->initialized = false;
    ctx->rendering = false;
    ctx->sprites = NULL;

    ctx->buffer.count = 0;
    ctx->buffer.entries = (render_entry *)((uint8*)memory.base + sizeof(render_context) + sizeof(render_buffer));

    ctx->memory = memory;

    PrepareOpenGL(api);

    // Allocate memory buffer
    uint32 size_per_element = sizeof(real32) * 3 * 4; // 3 real32 coords, 4 vertices
    uint32 available_memory = (memory.size - memory.used) / size_per_element;
    memory_segment vertex_segment = allocate_memory(&memory, available_memory / 2);
    memory_segment color_segment = allocate_memory(&memory, available_memory / 2);

    ctx->entries_max = available_memory / 2;
    ctx->entries_count = 0;
    ctx->vertex_buffer = vertex_segment;
    ctx->color_buffer = color_segment;

    ctx->initialized = true;
    return ctx;
}

void render_rect(render_context *ctx, int32 x, int32 y, int32 width, int32 height, color c) {
    
    render_sprite obj = {};
    obj.vertices[0] = { (real32)x, (real32)y, 0.f };
    obj.vertices[1] = { (real32)x, (real32)(y + height), 0.f };
    obj.vertices[2] = { (real32)(x + width), (real32)(y + height), 0.f };
    obj.vertices[3] = { (real32)(x + width), (real32)y, 0.f };

    for (int i = 0; i < 4; ++i) {
        obj.colors[i] = { c.r, c.g, c.b };
    }
    
    real32 *vert_pos = (real32*)ctx->vertex_buffer.base + ctx->entries_count * 4 * 3;
    real32 *color_pos = (real32*)ctx->color_buffer.base + ctx->entries_count * 4 * 3;
    for (int i = 0; i < 4; ++i) {
        vert_pos[0] = obj.vertices[i].x;
        vert_pos[1] = obj.vertices[i].y;
        vert_pos[2] = obj.vertices[i].z;
        color_pos[0] = obj.colors[i].x;
        color_pos[1] = obj.colors[i].y;
        color_pos[2] = obj.colors[i].z;
        vert_pos += 3;
        color_pos += 3;
    }

    ctx->entries_count++;
}

void render_start(render_context *ctx) {
    Assert(!ctx->rendering);
    Assert(ctx->sprites == NULL);
    ctx->rendering = true;
}

void render_end(render_context *ctx) {
    Assert(ctx->rendering);
    draw(ctx);

    ctx->buffer.count = 0;
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
    
}
