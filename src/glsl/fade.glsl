uniform float offsetLeft;
uniform float offsetRight;
uniform float strengthLeft;
uniform float strengthRight;

uniform sampler2D u_texture1;

#define FADE_WIDTH 18

void
mainImage (out vec4 fragColor,
           in vec2  fragCoord,
           in vec2  resolution,
           in vec2  uv)
{
  float progress;

  fragColor = GskTexture (u_texture1, uv);

  progress = fragCoord.x - offsetLeft;
  progress = min (max (progress / FADE_WIDTH, 0), 1);
  fragColor *= (1 + strengthLeft * (progress - 1));

  progress = resolution.x - offsetRight - fragCoord.x;
  progress = min (max (progress / FADE_WIDTH, 0), 1);
  fragColor *= (1 + strengthRight * (progress - 1));
}
