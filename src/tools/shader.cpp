#include <tools/shader.hpp>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
void checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

std::string load_shader_file(std::filesystem::path shader_path) {
    std::ifstream shader_file;
    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::string shader_code;
    try {
        shader_file.open(shader_path);
        std::stringstream shader_stream;
        shader_stream << shader_file.rdbuf();
        shader_file.close();

        shader_code = shader_stream.str();
    } catch (std::ifstream::failure& e) {
        std::cout << "[Shader Loader] Error when loading shader (" << shader_path << "): " << e.what() << std::endl;
    }

    return shader_code;
}

unsigned char compile_shader_code(const char* vertex_shader_code, const char* fragment_shader_code) {
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_shader_code, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_shader_code, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // shader Program
    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return ID;
}

// constructor generates the shader on the fly
// ------------------------------------------------------------------------
Shader::Shader(std::string const& vertex_path, std::string const& fragment_path) {
    std::string vertex_shader_code = load_shader_file(std::filesystem::path(vertex_path));
    std::string fragment_shader_code = load_shader_file(std::filesystem::path(fragment_path));

    ID = compile_shader_code(vertex_shader_code.c_str(), fragment_shader_code.c_str());
}

Shader::Shader(std::string const& folder, std::string const& vertex_path, std::string const& fragment_path) {
    std::string vertex_path2 = folder + vertex_path;
    std::string fragment_path2 = folder + fragment_path;
    
    std::string vertex_shader_code = load_shader_file(std::filesystem::path(vertex_path2));
    std::string fragment_shader_code = load_shader_file(std::filesystem::path(fragment_path2));

    ID = compile_shader_code(vertex_shader_code.c_str(), fragment_shader_code.c_str());
}

Shader::Shader(std::bitset<4> flags, std::filesystem::path shaders_folder) {
    std::string vertexFile = flags[3] ? "depth.vs" : "pbr.vs";
    std::string fragmentFile = flags[3] ? "depth.fs" : "pbr.fs";

    std::string vertex_code = load_shader_file(shaders_folder / vertexFile);
    std::string fragment_code = load_shader_file(shaders_folder / fragmentFile);

    size_t vertex_version_idx = vertex_code.find("#version");
    size_t vertex_end_version_idx = vertex_code.find("\n", vertex_version_idx) + 1;

    size_t fragment_version_idx = fragment_code.find("#version");
    size_t fragment_end_version_idx = fragment_code.find("\n", fragment_version_idx) + 1;

    if (flags.test(0)) {
        vertex_code.insert(vertex_end_version_idx, "#define HAS_NORMALS\n");
        fragment_code.insert(fragment_end_version_idx, "#define HAS_NORMALS\n");
    }
    if (flags.test(1)) {
        vertex_code.insert(vertex_end_version_idx, "#define HAS_BASE_COLOR_TEX\n");
        fragment_code.insert(fragment_end_version_idx, "#define HAS_BASE_COLOR_TEX\n");
    }
    if (flags.test(2)) {
        vertex_code.insert(vertex_end_version_idx, "#define HAS_PBR_TEX\n");
        fragment_code.insert(fragment_end_version_idx, "#define HAS_PBR_TEX\n");
    }

    ID = compile_shader_code(vertex_code.c_str(), fragment_code.c_str());
}

// activate the shader
// ------------------------------------------------------------------------
void Shader::use() const
{ 
    glUseProgram(ID); 
}
void Shader::unuse() const
{
    glUseProgram(0);
}

// utility uniform functions
// ------------------------------------------------------------------------
void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{ 
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec2(const std::string &name, float x, float y) const
{ 
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{ 
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec3(const std::string &name, float x, float y, float z) const
{ 
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{ 
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const
{ 
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

Shader& ShaderStore::get_shader(std::bitset<4> flags) {
    if (loaded_shaders.contains(flags)) {
        return loaded_shaders[flags];
    } else {
        Shader new_shader = Shader(flags, shaders_dir);
        loaded_shaders[flags] = new_shader;

        return loaded_shaders[flags];
    }
}