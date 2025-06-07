#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <filesystem>
#include <unordered_map>
#include <bitset>

class Shader
{
public:
    // ID of the shader
    // ----------------
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(std::string const& vertexPath, std::string const& fragmentPath);
    // constructeur avec spé du dossier (strings moins long)
    Shader(std::string const& folder, std::string const& vertexName, std::string const& fragmentName);

    Shader(std::bitset<4>, std::filesystem::path);

    // Ouais ptit constructeur avec la syntax dégueulasse du C++ toi même tu sais
    Shader(): ID(0) {};

    void paths(const char* vertexPath, const char* fragmentPath);

    // activate the shader
    // ------------------------------------------------------------------------
    void use() const;
    void unuse() const;
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const;
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const;
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const;
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
};

const std::bitset<3> HAS_NORMALS = 1;
const std::bitset<3> HAS_BASE_COLOR_TEX = 1 << 1;
const std::bitset<3> HAS_PBR_TEX = 1 << 2;

class ShaderStore {
    public:
        Shader& get_shader(std::bitset<4> flags);
        // et un ptit 2ème ;)
        ShaderStore(std::filesystem::path path): shaders_dir(path) {}

    private:
        std::unordered_map<std::bitset<4>, Shader> loaded_shaders;
        std::filesystem::path shaders_dir;
};

#endif