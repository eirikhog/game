#include "Renderer.h"
#include "Math.h"
#include "Assets.h"

#include <GL/glew.h>

typedef struct {
    uint32 id;
    v2 uv_origin;
    v2 uv_end;
    void *bitmap;
} AtlasEntry;

typedef struct {
    AtlasEntry entries[4];
} Atlas;

typedef struct {
    v3 position;
    Color color;
    v2 uv;
} RenderVertex;

typedef struct RenderContext {
    bool initialized;
    bool rendering;

    v2 windowSize;

    MemorySegment memory;
    MemorySegment vertex_buffer;

    uint32 entries_count;
    uint32 entries_max;

    // TODO: Remove this
    uint32 texture_width;
    uint32 texture_height;
    void *texture_data;

    AtlasAsset atlas;
} RenderContext;

static void draw(RenderContext *ctx);
static void render_object(RenderContext *ctx, int32 x, int32 y, int32 width, int32 height, Color c, AssetId image_id);

static void initialize_opengl(GameAssets *assets) {
    GLenum err = glewInit();
    Assert(err == GLEW_OK);
    if (err != GLEW_OK)
    {
        //Problem: glewInit failed, something is seriously wrong.
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

    // load shaders & get length of each
    ShaderAsset vs = asset_get_shader(assets, ASSET_SHADER_VERTEX);
    ShaderAsset fs = asset_get_shader(assets, ASSET_SHADER_FRAGMENT);

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

#include <stdio.h>

void load_textures(GameAssets *assets, RenderContext *ctx) {
    //ImageAsset img = asset_get_image(assets, ASSET_IMAGE_SPRITEMAP);
    //Assert(img.data);

    AtlasAsset a = asset_get_atlas(assets, ASSET_ATLAS1);
    ctx->atlas = a;

    FILE *fp = fopen("image.rgba", "w");
    fwrite(a.data, 32 * a.width * a.height, 1, fp);

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, a.width, a.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, a.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glGenerateMipmap(GL_TEXTURE_2D);
    //
    
    // Create texture atlas
    // TODO: This should be generated from asset file.
}

RenderContext *render_init(GameAssets *assets, MemorySegment memory) {
    allocate_memory(&memory, sizeof(RenderContext));

    RenderContext *ctx = (RenderContext*)memory.base;
    ctx->initialized = false;
    ctx->rendering = false;
    ctx->memory = memory;

    initialize_opengl(assets);
    load_textures(assets, ctx);

    // Allocate memory buffer
    // TODO: Make more efficient.
    uint32 vertex_size = sizeof(RenderVertex) * 4;
    uint32 available_memory = (memory.size - memory.used) / vertex_size;
    MemorySegment vertex_segment = allocate_memory(&memory, available_memory);

    ctx->entries_max = available_memory / vertex_size;
    ctx->entries_count = 0;
    ctx->vertex_buffer = vertex_segment;

    ctx->initialized = true;
    return ctx;
}

void render_image(RenderContext *ctx, int32 x, int32 y, int32 width, int32 height, AssetId image_id) {
    render_object(ctx, x, y, width, height, { 1.0f, 1.0f, 1.0f }, image_id);
}

void render_rect(RenderContext *ctx, int32 x, int32 y, int32 width, int32 height, Color c) {
    render_object(ctx, x, y, width, height, c, ASSET_TEXTURE_WHITE);
}

AtlasAssetEntry get_atlas_entry(AtlasAsset *atlas, AssetId id) {
    // TODO: Dynamic size
    for (int i = 0; i < 8; ++i) {
        if (atlas->entries[i].id == id) {
            return atlas->entries[i];
        }
    }
    Assert(0);
}

void render_object(RenderContext *ctx, int32 x, int32 y, int32 width, int32 height, Color c, AssetId image_id) {

    // If the buffer is full, push the data to the graphics card and render what we got.
    if (ctx->entries_count >= ctx->entries_max) {
        draw(ctx);
        segment_clear(&ctx->vertex_buffer);
        ctx->entries_count = 0;
    }

    AtlasAssetEntry entry = get_atlas_entry(&ctx->atlas, image_id);
    
    RenderVertex vertices[4];
    vertices[0] = { { (real32)x, (real32)y, 0.f }, c, { entry.uvOrigin.x, 1.0f - entry.uvOrigin.y } };
    vertices[1] = { { (real32)x, (real32)(y + height), 0.f }, c, { entry.uvOrigin.x, 1.0f - entry.uvEnd.y } };
    vertices[2] = { { (real32)(x + width), (real32)(y + height), 0.f }, c, { entry.uvEnd.x, 1.0f - entry.uvEnd.y } };
    vertices[3] = { { (real32)(x + width), (real32)y, 0.f }, c, { entry.uvEnd.x, 1.0f - entry.uvOrigin.y } };
    //vertices[0] = { { (real32)x, (real32)y, 0.f }, c,{ entry.uvOrigin.x, entry.uvEnd.y } };
    //vertices[1] = { { (real32)x, (real32)(y + height), 0.f }, c,{ entry.uvOrigin.x, entry.uvOrigin.y } };
    //vertices[2] = { { (real32)(x + width), (real32)(y + height), 0.f }, c,{ entry.uvEnd.x, entry.uvOrigin.y } };
    //vertices[3] = { { (real32)(x + width), (real32)y, 0.f }, c,{ entry.uvEnd.x, entry.uvEnd.y } };

    for (int i = 0; i < 4; ++i) {
        RenderVertex *c_vert = PUSH_STRUCT(&ctx->vertex_buffer, RenderVertex);
        *c_vert = vertices[i];
    }

    ctx->entries_count++;
}

void render_rect(RenderContext *ctx, v2 pos, v2 size, Color c) {
    render_rect(ctx, (int32)pos.x, (int32)pos.y, (int32)size.x, (int32)size.y, c);
}

void render_start(RenderContext *ctx, v2 windowSize) {
    Assert(!ctx->rendering);
    Assert(ctx->vertex_buffer.base != NULL);
    Assert(ctx->entries_count == 0);

    ctx->rendering = true;
    ctx->windowSize = windowSize;
}

void render_end(RenderContext *ctx) {
    Assert(ctx->rendering);

    draw(ctx);

    segment_clear(&ctx->vertex_buffer);

    ctx->entries_count = 0;
    ctx->rendering = false;
}

void draw(RenderContext *ctx) {
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
