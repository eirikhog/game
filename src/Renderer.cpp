#include "Renderer.h"

#include <GL/glew.h>

// two vertex array objects, one for each object drawn
unsigned int vertexArrayObjID[2];
// three vertex buffer objects in this example
unsigned int vertexBufferObjID[3];

GLfloat vertices[] = { 0.0f,0.0f,0.0f, 0.0f,250.0f,0.0f, 100.0f,100.0f,0.0f };
GLfloat colours[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat vertices2[] = { 0.0f,0.0f,0.0f, 0.0f,-1.0f,0.0f, 1.0f,0.0f,0.0f };


void InitializeOpenGL(render_context *ctx) {
    glGenVertexArrays(2, &vertexArrayObjID[0]);
    glBindVertexArray(vertexArrayObjID[0]);
    glGenBuffers(2, vertexBufferObjID);

    // VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[0]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // VBO for color data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[1]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colours, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(vertexArrayObjID[1]);
    glGenBuffers(1, &vertexBufferObjID[2]);

    // VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[2]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GL_FLOAT), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    ctx->initialized = true;
}

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

    // TODO: FREE MEMORY!
    // VirtualFree(vs, 0, MEM_RELEASE);
    // VirtualFree(fs, 0, MEM_RELEASE);
}


// TODO: Initialization
render_context initialize_renderer(platform_api *api) {
    render_context ctx = {};
    PrepareOpenGL(api);
    InitializeOpenGL(&ctx);
    return ctx;
}

void render_rect(render_context *ctx, int32 x, int32 y, int32 width, int32 height, color c) {
}


void render_start(render_context *ctx) {
    // Do the actual rendering...
    //

    /*
    // Cheating! We probably want this from somewhere else...
    GLint viewport_size[4];
    glGetIntegerv(GL_VIEWPORT, viewport_size);

    int32 width = (int32)viewport[2];
    int32 height = (int32)viewport[3];
    */

    glClear(GL_COLOR_BUFFER_BIT);

    if (ctx->initialized) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glBindVertexArray(vertexArrayObjID[0]);	// First VAO
        glDrawArrays(GL_TRIANGLES, 0, 3);	// draw first object

        glBindVertexArray(vertexArrayObjID[1]);		// select second VAO
        glVertexAttrib3f((GLuint)1, 1.0, 0.0, 0.0); // set constant color attribute
        glDrawArrays(GL_TRIANGLES, 0, 3);	// draw second object

        glBindVertexArray(0);
    }
    
}
