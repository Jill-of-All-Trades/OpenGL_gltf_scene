#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec4 ColorFactor;

out vec4 FragColor;

uniform sampler2D tex_diffuse;

void main()
{   
    vec4 base_color = texture(tex_diffuse, TexCoords);
    if(base_color.a < 0.05) discard;

    // apply ColorFactor
    vec4 final_color = base_color * ColorFactor;

    FragColor = vec4(final_color);
}