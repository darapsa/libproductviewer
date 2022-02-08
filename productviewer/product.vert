uniform mat4 u_matrix;
attribute vec4 a_position;
attribute vec2 a_texcoord;
varying vec2 v_texcoord;
void main()
{
	gl_Position = u_matrix * a_position;
	v_texcoord = a_texcoord;
}
