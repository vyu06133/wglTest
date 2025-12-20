#version 450 compatibility
#pragma optimize(off)
in vec3 a_position;
in vec4 a_color;
in vec2 a_tex;

// CPU側から転送する
uniform mat4 u_mvp;
out vec4 v_color;
out vec3 v_position;
out vec2 v_tex;
out vec4 debugColor;


void main()
{
	debugColor = a_color;
	v_color = a_color;
	v_tex = a_tex;
	// プロジェクション行列を一度だけ適用
	vec4 projected_pos = u_mvp * vec4(a_position, 1.0);

	v_position = projected_pos.xyz; // 出力変数に代入
	gl_Position = projected_pos; // 最終位置として使用
}	

