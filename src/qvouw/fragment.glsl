#version 150

//in vec4 in_color;
in GS_OUT {
    vec4 color;
} fs_in;

out vec4 out_color;

void main() {
  out_color = fs_in.color;
  //gl_FragColor = fs_in.color;
//gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );
}


/*#version 130

smooth in vec4 vertexColor;

out vec4 outputColor;

void main() {
//  outputColor = vertexColor;
gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );
}*/

