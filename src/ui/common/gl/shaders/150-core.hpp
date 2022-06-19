#pragma once
static const char *vert_shader_src =
R"(#version 150 core
out vec2 v_tex;

const vec2 v_pos[4]=vec2[4](vec2(-1.0, 1.0),
                            vec2(-1.0,-1.0),
                            vec2( 1.0, 1.0),
                            vec2( 1.0,-1.0));
const vec2 t_pos[4]=vec2[4](vec2(0.0,0.0),
                            vec2(0.0,1.0),
                            vec2(1.0,0.0),
                            vec2(1.0,1.0));

void main()
{
    v_tex=t_pos[gl_VertexID];
    gl_Position=vec4(v_pos[gl_VertexID], 0.0, 1.0);
}
)";

static const char *frag_shader_src =
R"(#version 150 core
in vec2 v_tex;
uniform sampler2D texSampler;
out vec4 color;
void main()
{
    color=texture(texSampler, v_tex);
}
)";

