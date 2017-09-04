#version 330
#include "../foundation/effects.h"

in vec2 _texCoords;
out vec4 frag_color;

void main()
{
	frag_color = vec4(effects(gFramebuffer, _texCoords), 1);
}