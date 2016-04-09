#include "Renderer.h"
#include "Math.h"
#include "Assets.h"

#include <GL/glew.h>
#include <stdlib.h>

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
    MemorySegment vertexBuffer;

    u32 entriesCount;
    u32 entriesMax;

    AtlasAsset *atlas;
    u32 atlasId;
    AtlasAsset *fontAtlas;
    u32 fontAtlasId;

    u32 selectedTexture;

    u32 prevFrameObjects;
    u32 currFrameObjects;
} RenderContext;

static void Draw(RenderContext *ctx);
static void RenderObject(RenderContext *ctx, Rect2Di r, Color c, u32 image_id, u32 spritemapId);

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
    ShaderAsset *vs = AssetGetShader(assets, ASSET_SHADER_VERTEX);
    ShaderAsset *fs = AssetGetShader(assets, ASSET_SHADER_FRAGMENT);

    GLint vlen = (GLint)vs->size;
    GLint flen = (GLint)fs->size;
    const char * vv = vs->content;
    const char * ff = fs->content;
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
    AtlasAsset *atlas = AssetGetAtlas(assets, ASSET_SPRITEMAP);
    ctx->atlas = atlas;

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas->width, atlas->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas->data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ctx->atlasId = id;

    AtlasAsset *fontAtlas = AssetGetAtlas(assets, ASSET_FONT_SPRITEMAP);
    ctx->fontAtlas = fontAtlas;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fontAtlas->width, fontAtlas->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontAtlas->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ctx->fontAtlasId = id;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ctx->selectedTexture = id;
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
    u32 vertex_size = sizeof(RenderVertex) * 4;
    u32 available_memory = (memory.size - memory.used) / vertex_size;
    MemorySegment vertex_segment = AllocMemory(&memory, available_memory);

    ctx->entriesMax = available_memory / vertex_size;
    ctx->entriesCount = 0;
    ctx->vertexBuffer = vertex_segment;

    ctx->initialized = 1;
    return ctx;
}

void DrawImage(RenderContext *ctx, Rect2Di target, u32 image_id) {
    RenderObject(ctx, target, { 1.0f, 1.0f, 1.0f }, image_id, ASSET_SPRITEMAP);
}

void DrawSolidRect(RenderContext *ctx, Rect2Di r, Color c) {
    RenderObject(ctx, r, c, ASSET_TEXTURE_WHITE, ASSET_SPRITEMAP);
}

void DrawRect(RenderContext *ctx, Rect2Di rect, Color c) {

    // "Normalize" the rectangle, so that we don't draw empty corners.
    Rect2Di r = Normalize(rect);

    DrawSolidRect(ctx, Rect2Di(r.x, r.y, r.width, 1), c);
    DrawSolidRect(ctx, Rect2Di(r.x, r.y, 1, r.height), c);
    DrawSolidRect(ctx, Rect2Di(r.x, r.y + r.height - 1, r.width, 1), c);
    DrawSolidRect(ctx, Rect2Di(r.x + r.width - 1, r.y, 1, r.height), c);
}

void DrawText(RenderContext *ctx, const char *str, v2i position, Color c) {
    
    const i32 width = 10;
    const i32 height = 16;

    i32 offsetX = 0;
    const char *ptr = str;
    while (*ptr != 0) {
        RenderObject(ctx, { position.x + offsetX, position.y, width, height }, c, *ptr, ASSET_FONT_SPRITEMAP);
        offsetX += width;
        ptr++;
    }
}

AtlasAssetEntry *GetAtlasEntry(AtlasAsset *atlas, u32 id) {
    
    for (u32 i = 0; i < atlas->count; ++i) {
        if (atlas->entries[i].id == id) {
            return &atlas->entries[i];
        }
    }

    return 0;
}

void RenderObject(RenderContext *ctx, Rect2Di r, Color c, u32 image_id, u32 spritemapId) {
    bool32 flush = 0;

    u32 id = spritemapId == ASSET_SPRITEMAP ? ctx->atlasId : ctx->fontAtlasId;
    if (ctx->selectedTexture != id) {
        // Draw whatever is in the buffer, and switch texture.
        Draw(ctx);
        SegmentClear(&ctx->vertexBuffer);
        ctx->entriesCount = 0;

        glBindTexture(GL_TEXTURE_2D, id);
        ctx->selectedTexture = id;
        flush = 1;
    }

    // If the buffer is full, push the data to the graphics card and render what we got.
    if (ctx->entriesCount >= ctx->entriesMax) {
        Draw(ctx);
        SegmentClear(&ctx->vertexBuffer);
        ctx->entriesCount = 0;
    }

    AtlasAsset *atlas = spritemapId == ASSET_SPRITEMAP ? ctx->atlas : ctx->fontAtlas;
    AtlasAssetEntry *entry = GetAtlasEntry(atlas, image_id);

    if (!entry) {
        return;
    }
    
    RenderVertex vertices[4];
    vertices[0] = { { (r32)r.x, (r32)r.y, 0.f }, c, { entry->uvOrigin.x, 1.0f - entry->uvOrigin.y } };
    vertices[1] = { { (r32)r.x, (r32)(r.y + r.height), 0.f }, c, { entry->uvOrigin.x, 1.0f - entry->uvEnd.y } };
    vertices[2] = { { (r32)(r.x + r.width), (r32)(r.y + r.height), 0.f }, c, { entry->uvEnd.x, 1.0f - entry->uvEnd.y } };
    vertices[3] = { { (r32)(r.x + r.width), (r32)r.y, 0.f }, c, { entry->uvEnd.x, 1.0f - entry->uvOrigin.y } };

    for (int i = 0; i < 4; ++i) {
        RenderVertex *c_vert = PUSH_STRUCT(&ctx->vertexBuffer, RenderVertex);
        *c_vert = vertices[i];
    }

    ctx->entriesCount++;
    ctx->currFrameObjects++;
}

void RenderStart(RenderContext *ctx, v2i windowSize) {
    Assert(!ctx->rendering);
    Assert(ctx->vertexBuffer.base != NULL);
    Assert(ctx->entriesCount == 0);

    ctx->rendering = true;
    ctx->windowSize = windowSize;

    ctx->currFrameObjects = 0;
}

void RenderEnd(RenderContext *ctx) {
    Assert(ctx->rendering);

    Draw(ctx);

    SegmentClear(&ctx->vertexBuffer);
    ctx->entriesCount = 0;

    ctx->prevFrameObjects = ctx->currFrameObjects;

    ctx->rendering = false;
}

u32 RenderedObjects(RenderContext *ctx) {
    return ctx->prevFrameObjects;
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
    glBufferData(GL_ARRAY_BUFFER, ctx->entriesCount * 4 * sizeof(RenderVertex), ctx->vertexBuffer.base, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), 0);
    glEnableVertexAttribArray(0);

    // VBO for color data
    glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (GLvoid*)offsetof(RenderVertex, color));
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
    glDrawArrays(GL_QUADS, 0, 4 * ctx->entriesCount);

    glBindVertexArray(0);

    glDeleteBuffers(1, &vertexBufferObjID);
    glDeleteVertexArrays(1, &vertexArrayObjId);
}
