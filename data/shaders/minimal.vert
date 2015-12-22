// Vertex shader

#version 140

uniform vec2 size;

in  vec3 in_Position;
in  vec3 in_Color;
out vec3 ex_Color;

void main(void)
{
    // Transform to pixel values.
    // TODO: Transform the Z coordinates?
	mat4 projectionMatrix = mat4( 2.0/size.x, 0.0, 0.0, -1.0,
                              0.0, -2.0/size.y, 0.0, 1.0,
                              0.0, 0.0, 1, 0,
                              0.0, 0.0, 0.0, 1.0);    
	gl_Position = vec4(in_Position, 1.0);
    gl_Position *= projectionMatrix;
	ex_Color = in_Color;
}
