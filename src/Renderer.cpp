#include "Renderer.h"
#include "Math.h"
#include "Assets.h"

#include <GL/glew.h>
#include <stdlib.h>


typedef struct {
    uint32 id;
    v2f uv_origin;
    v2f uv_end;
    void *bitmap;
} AtlasEntry;

typedef struct {
    AtlasEntry entries[4];
} Atlas;

typedef struct {
    v3 position;
    Color color;
    v2f uv;
} RenderVertex;

typedef struct RenderContext {
    bool32 initialized;
    bool32 rendering;

    v2i windowSize;

    MemorySegment memory;
    MemorySegment vertex_buffer;

    uint32 entries_count;
    uint32 entries_max;

    AtlasAsset *atlas;
} RenderContext;

static void Draw(RenderContext *ctx);
static void RenderObject(RenderContext *ctx, Rect2Di r, Color c, AssetId image_id);

static void InitializeOpenGL(GameAssets *assets) {
    GLenum err = glewInit();
    Assert(err == GLEW_OK);
    if (err != GLEW_OK)
    {
        //Problem: glewInit failed, something is seriously wrong.
        InvalidCodePath();
    }
    
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

    // load shaders & get length of each
    ShaderAsset vs = AssetGetShader(assets, ASSET_SHADER_VERTEX);
    ShaderAsset fs = AssetGetShader(assets, ASSET_SHADER_FRAGMENT);

    GLint vlen = (GLint)vs.size;
    GLint flen = (GLint)fs.size;
    const char * vv = vs.content;
    const char * ff = fs.content;
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
}

static void LoadTextures(GameAssets *assets, RenderContext *ctx) {
    AtlasAsset *a = AssetGetAtlas(assets, ASSET_ATLAS1);
    ctx->atlas = a;

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, a->width, a->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, a->data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

RenderContext *RenderInit(GameAssets *assets, MemorySegment memory) {
    AllocMemory(&memory, sizeof(RenderContext));

    RenderContext *ctx = (RenderContext*)memory.base;
    ctx->initialized = 0;
    ctx->rendering = 0;
    ctx->memory = memory;

    InitializeOpenGL(assets);
    LoadTextures(assets, ctx);

    // Allocate memory buffer
    uint32 vertex_size = sizeof(RenderVertex) * 4;
    uint32 available_memory = (memory.size - memory.used) / vertex_size;
    MemorySegment vertex_segment = AllocMemory(&memory, available_memory);

    ctx->entries_max = available_memory / vertex_size;
    ctx->entries_count = 0;
    ctx->vertex_buffer = vertex_segment;

    ctx->initialized = 1;
    return ctx;
}

void DrawImage(RenderContext *ctx, Rect2Di target, AssetId image_id) {
    RenderObject(ctx, target, { 1.0f, 1.0f, 1.0f }, image_id);
}

void DrawSolidRect(RenderContext *ctx, Rect2Di r, Color c) {
    RenderObject(ctx, r, c, ASSET_TEXTURE_WHITE);
}

void DrawRect(RenderContext *ctx, Rect2Di r, Color c) {
    DrawSolidRect(ctx, Rect2Di(r.x, r.y, r.width, 1), c);
    DrawSolidRect(ctx, Rect2Di(r.x, r.y, 1, r.height), c);
    DrawSolidRect(ctx, Rect2Di(r.x, r.y + r.height - 1, r.width, 1), c);
    DrawSolidRect(ctx, Rect2Di(r.x + r.width - 1, r.y, 1, r.height), c);
}


AtlasAssetEntry GetAtlasEntry(AtlasAsset *atlas, AssetId id) {
    
    for (uint32 i = 0; i < atlas->count; ++i) {
        if (atlas->entries[i].id == id) {
            return atlas->entries[i];
        }
    }

    InvalidCodePath();
    return{};
}

void RenderObject(RenderContext *ctx, Rect2Di r, Color c, AssetId image_id) {

    // If the buffer is full, push the data to the graphics card and render what we got.
    if (ctx->entries_count >= ctx->entries_max) {
        Draw(ctx);
        SegmentClear(&ctx->vertex_buffer);
        ctx->entries_count = 0;
    }

    AtlasAssetEntry entry = GetAtlasEntry(ctx->atlas, image_id);
    
    RenderVertex vertices[4];
    vertices[0] = { { (real32)r.x, (real32)r.y, 0.f }, c, { entry.uvOrigin.x, 1.0f - entry.uvOrigin.y } };
    vertices[1] = { { (real32)r.x, (real32)(r.y + r.height), 0.f }, c, { entry.uvOrigin.x, 1.0f - entry.uvEnd.y } };
    vertices[2] = { { (real32)(r.x + r.width), (real32)(r.y + r.height), 0.f }, c, { entry.uvEnd.x, 1.0f - entry.uvEnd.y } };
    vertices[3] = { { (real32)(r.x + r.width), (real32)r.y, 0.f }, c, { entry.uvEnd.x, 1.0f - entry.uvOrigin.y } };

    for (int i = 0; i < 4; ++i) {
        RenderVertex *c_vert = PUSH_STRUCT(&ctx->vertex_buffer, RenderVertex);
        *c_vert = vertices[i];
    }

    ctx->entries_count++;
}

void RenderStart(RenderContext *ctx, v2i windowSize) {
    Assert(!ctx->rendering);
    Assert(ctx->vertex_buffer.base != NULL);
    Assert(ctx->entries_count == 0);

    ctx->rendering = true;
    ctx->windowSize = windowSize;
}

void RenderEnd(RenderContext *ctx) {
    Assert(ctx->rendering);

    Draw(ctx);

    SegmentClear(&ctx->vertex_buffer);
    ctx->entries_count = 0;

    ctx->rendering = false;
}

void Draw(RenderContext *ctx) {
    Assert(ctx->initialized);

    unsigned int vertexArrayObjId;
    unsigned int vertexBufferObjID;
    glGenVertexArrays(1, &vertexArrayObjId);
    glBindVertexArray(vertexArrayObjId);
    glGenBuffers(1, &vertexBufferObjID);

    // VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID);
    glBufferData(GL_ARRAY_BUFFER, ctx->entries_count * 4 * sizeof(RenderVertex), ctx->vertex_buffer.base, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), 0);
    glEnableVertexAttribArray(0);

    // VBO for color data
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (GLvoid*)offsetof(RenderVertex, color));
    glEnableVertexAttribArray(1);

    // UV buffer
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (GLvoid*)offsetof(RenderVertex, uv));
    glEnableVertexAttribArray(2);
    
    GLfloat width = (GLfloat)ctx->windowSize.x;
    GLfloat height = (GLfloat)ctx->windowSize.y;
    GLfloat size[2] = { width, height };

    GLint id;
    glGetIntegerv(GL_CURRENT_PROGRAM, &id);

    GLint loc = glGetUniformLocation(id, "size");
    if (loc != -1)
    {
        glUniform2f(loc, size[0], size[1]);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glBindVertexArray(vertexArrayObjId);
    glDrawArrays(GL_QUADS, 0, 4 * ctx->entries_count);

    glBindVertexArray(0);

    glDeleteBuffers(1, &vertexBufferObjID);
    glDeleteVertexArrays(1, &vertexArrayObjId);
}
