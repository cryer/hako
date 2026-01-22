// 简单封装，需要在 main 中初始化
class BoxRenderer {
    GLuint VAO, VBO;
    Shader shader;
public:
    BoxRenderer() : shader(/*VertexShader*/, /*FragShader*/) { /*...init cube lines...*/ }
    // 实际在 main 中直接用简单的 glDrawArrays(GL_LINES) 即可，无需此类也可以
};