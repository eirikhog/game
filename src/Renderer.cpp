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
    render_context *ctx = (render_context*)memory.base;
    ctx->initialized = false;
    ctx->rendering = false;
    ctx->sprites = NULL;

    ctx->buffer.count = 0;
    ctx->buffer.entries = (render_entry *)((uint8*)memory.base + sizeof(render_context) + sizeof(render_buffer));

    PrepareOpenGL(api);

    ctx->initialized = true;
    return ctx;
}

void render_rect(render_context *ctx, int32 x, int32 y, int32 width, int32 height, color c) {
    
    render_sprite obj = {};
    v3 test = { 1.0f, 2.0f, 3.0f };
    obj.vertices[0] = { (real32)x, (real32)y, 0.f };
    obj.vertices[1] = { (real32)x, (real32)(y + height), 0.f };
    obj.vertices[2] = { (real32)(x + width), (real32)(y + height), 0.f };
    obj.vertices[3] = { (real32)(x + width), (real32)y, 0.f };

    for (int i = 0; i < 4; ++i) {
        obj.colors[i] = { c.r, c.g, c.b };
    }

    unsigned int vertexArrayObjId;
    unsigned int vertexBufferObjID[2];
    glGenVertexArrays(1, &vertexArrayObjId);
    glBindVertexArray(vertexArrayObjId);
    glGenBuffers(2, vertexBufferObjID);

    // VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[0]);
    glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(real32), obj.vertices, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // VBO for color data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[1]);
    glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GLfloat), obj.colors, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    render_entry entry = { };
    entry.vab = vertexArrayObjId;
    entry.vbo[0] = vertexBufferObjID[0];
    entry.vbo[1] = vertexBufferObjID[1];
    add_entry(ctx, entry);

    glBindVertexArray(0);
    // TODO: Better memory management!
}

void render_start(render_context *ctx) {
    Assert(!ctx->rendering);
    Assert(ctx->sprites == NULL);
    ctx->rendering = true;
}

void render_end(render_context *ctx) {
    Assert(ctx->rendering);
    draw(ctx);

    // Delete buffers...
    for (uint32 i = 0; i < ctx->buffer.count; ++i) {
        render_entry *current = ctx->buffer.entries + i;
        glDeleteBuffers(2, current->vbo);
        glDeleteVertexArrays(1, &current->vab);
    }
    ctx->buffer.count = 0;

    ctx->rendering = false;
}

void draw(render_context *ctx) {    
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

        // Render all sprites...
        for (uint32 i = 0; i < ctx->buffer.count; ++i) {
            render_entry *sprite = ctx->buffer.entries + i;
            glBindVertexArray(sprite->vab);
            glDrawArrays(GL_QUADS, 0, 4);
        }

        glBindVertexArray(0);
    }
    
}
