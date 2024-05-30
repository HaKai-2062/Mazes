std::string mazeVertexShader = R"(

#version 330 core
layout (location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
out vec4 ourColor;
void main()
{
	gl_Position = vec4(aPos.xy, 1.0, 1.0);
	ourColor = aColor;
}

)";