// Vertex shader

#version 140

uniform vec2 size;

in vec3 in_position;
in vec4 in_color;
in vec2 in_vertexUV;
out vec4 ex_color;
out vec2 UV;

void main(void)
{
    // Transform to pixel values.
    mat4 projectionMatrix = mat4( 2.0/size.x, 0.0, 0.0, -1.0,
                                  0.0, -2.0/size.y, 0.0, 1.0,
                                  0.0, 0.0, 1, 0,
                                  0.0, 0.0, 0.0, 1.0);

    mat4 scaleMatrix = mat4( 1.0, 0.0, 0.0, 0.0,
                             0.0, 1.0, 0.0, 0.0,
                             0.0, 0.0, 1.0, 0.0,
                             0.0, 0.0, 0.0, 1.0);

    gl_Position = vec4(in_position, 1.0);
    gl_Position *= scaleMatrix;
    gl_Position *= projectionMatrix;
    
    ex_color = in_color;
    UV = in_vertexUV;
}
