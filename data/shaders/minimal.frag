#version 140

precision highp float; // needed only for version 1.30

in vec3 ex_color;
in vec2 UV;
out vec4 out_color;

uniform sampler2D texture_sampler;

void main(void)
{
    out_color = vec4(texture(texture_sampler, UV).rgb, 1.0) * vec4(ex_color, 1.0);
}
