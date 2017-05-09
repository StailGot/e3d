#version 430

out vec4 color;
in float radius;

uniform vec2  viewport;
uniform float time;
uniform vec2 mouse;

void main()
{
  vec2 screen_space = gl_FragCoord.xy;
  vec2 normal_space = gl_FragCoord.xy / viewport;

  int size = int( 2 );

  if ( int( screen_space.x ) % size != 0 )
    discard;
  if ( int( screen_space.y ) % size != 0 )
    discard;

  vec2 center = vec2( vec2( mouse.x, viewport.y - mouse.y ) / viewport );

  if ( distance( normal_space, center ) > radius )
    discard;

  color = vec4( 0.9, 0.1, 0.1, 1 ) * ( radius + 0.7 );
}
