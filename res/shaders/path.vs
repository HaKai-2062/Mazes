std::string pathVertexShader = R"(

#version 330 core
layout (location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aRefPoint;
layout(location = 3) in vec2 aDimension;

out vec4 ourColor;
out vec2 ourPos;
out vec2 ourRefPoint;
out vec2 ourDimension;

void main()
{
    gl_Position = vec4(aPos.xy, 1.0, 1.0);
    ourColor = aColor;
    ourPos = aPos;
    ourRefPoint = aRefPoint;
    ourDimension = aDimension;
}

)";