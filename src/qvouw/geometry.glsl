#version 150

layout(points) in;
layout (triangle_strip, max_vertices=4) out;

uniform mat4 mat_mvp;

/*in vec4 in_color[1];
out vec4 out_color;*/

in VS_OUT {
    vec4 color;
} gs_in[1];
 
out GS_OUT {
    vec4 color;
} gs_out;

void main() {
    vec4 pos = gl_in[0].gl_Position;

    vec4 v =pos - vec4( 0.01, 0.01, 0, 0 );
    gl_Position = mat_mvp * v;
    gs_out.color =gs_in[0].color;
//    out_color =in_color[0];
    EmitVertex();
    
    v =pos + vec4( -0.01, 0.01, 0, 0 );
    gl_Position = mat_mvp * v;
    gs_out.color =gs_in[0].color;
//    out_color =in_color[0];
    EmitVertex();
    
    v =pos + vec4( 0.01, -0.01, 0, 0 );
    gl_Position = mat_mvp * v;
    gs_out.color =gs_in[0].color;
//    out_color =in_color[0];
    EmitVertex();
    
    v =pos + vec4( 0.01, 0.01, 0, 0 );
    gl_Position = mat_mvp * v;
    gs_out.color =gs_in[0].color;
//    out_color =in_color[0];
    EmitVertex();
    EndPrimitive();
}
