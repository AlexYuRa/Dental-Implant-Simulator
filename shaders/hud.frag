#version 460 core

out vec4 FragColor;

in vec2 TexCoord;

uniform vec4 color;
uniform bool useTexture;
uniform sampler2D hudTexture;

void main()
{
    if (useTexture) {
        FragColor = texture(hudTexture, TexCoord) * color;
    } else {
        FragColor = color;
    }
}
