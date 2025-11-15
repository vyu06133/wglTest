#version 400 core

layout (location = 0) in vec3 a_position;
layout (location = 2) in vec4 a_color;
layout (location = 3) in vec2 a_tex;

// CPU側から転送する
uniform mat4 u_mvp;
out vec4 vertColor;
out vec3 vertPos;
out vec2 vertTex;

void main()
{
	vertColor = a_color;
	vertTex = a_tex;
    // プロジェクション行列を一度だけ適用
    vec4 projected_pos = u_mvp * vec4(a_position, 1.0);

    vertPos = projected_pos.xyz; // 出力変数に代入
    gl_Position = projected_pos;         // 最終位置として使用
}	

