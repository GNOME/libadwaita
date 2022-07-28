uniform float offsetLeft;
uniform float offsetRight;
uniform float strengthLeft;
uniform float strengthRight;
uniform float widthLeft;
uniform float widthRight;

uniform sampler2D u_texture1;

void
mainImage (out vec4 fragColor,
           in vec2  fragCoord,
           in vec2  resolution,
           in vec2  uv)
{
  float progress;

  fragColor = GskTexture (u_texture1, uv);

  progress = fragCoord.x - offsetLeft;
  progress = min (max (progress / widthLeft, 0.0), 1.0);
  fragColor *= (1.0 + strengthLeft * (progress - 1.0));

  progress = resolution.x - offsetRight - fragCoord.x;
  progress = min (max (progress / widthRight, 0.0), 1.0);
  fragColor *= (1.0 + strengthRight * (progress - 1.0));
}
