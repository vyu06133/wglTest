#version 400 core

layout (location = 0) in vec3 cpu_position;
layout (location = 2) in vec4 cpu_color;
layout (location = 3) in vec2 cpu_tex;

// CPU������]������
uniform mat4 u_mvp;
out vec4 vertColor;
out vec3 vertPos;
out vec2 vertTex;

void main()
{
	vertColor = cpu_color;
	vertTex = cpu_tex;
    // �v���W�F�N�V�����s�����x�����K�p
    vec4 projected_pos = u_mvp * vec4(cpu_position, 1.0);

    vertPos = projected_pos.xyz; // �o�͕ϐ��ɑ��
    gl_Position = projected_pos;         // �ŏI�ʒu�Ƃ��Ďg�p
}	

