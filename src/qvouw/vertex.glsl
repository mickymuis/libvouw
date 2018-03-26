#version 150

in vec3 in_position;
in vec3 in_color;

uniform mat4 mat_mvp;

out VS_OUT {
    vec4 color;
} vs_out;

void main() {
  gl_Position = /*mat_mvp **/ vec4(in_position, 1.0);
  vs_out.color = vec4(in_color, 1.0);
}


/*#version 130

in vec3 in_position;
in vec3 in_color;

uniform mat4 mvp;

smooth out vec4 vertexColor;

void main() {
  gl_Position = mvp * vec4(in_position, 1.0);
  vertexColor = vec4(in_color, 1.0);
}*/
