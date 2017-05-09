#version 430

in layout( location = 0 ) vec4 position;

out float radius;

uniform float time;
uniform vec2 viewport;

const float pi = acos( -1 );
const float g  = 9.8;

void main()
{
  radius = max( min( sin( time * ( pi * 0.3 ) ) * 0.01 * g + 0.2, 0.5 ), 0.1 );
  gl_PointSize = max( viewport.x, viewport.y );
  gl_Position  = position;
}
