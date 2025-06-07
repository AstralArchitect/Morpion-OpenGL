#ifndef GLTF_LOADER_H
#define GLTF_LOADER_H

#include <optional>

#include <tiny_gltf.h>
#include <glad/glad.h>
#include <tools/shader.hpp>
#include <glm/glm.hpp>

using namespace glm;

class GltfMaterial {
    public:
        GltfMaterial(tinygltf::Model& root, tinygltf::Material mat, ShaderStore& shader_store, bool has_normals);
        GltfMaterial() {};
        inline void activate(const mat4& node_transform, glm::mat4 const& lightSpaceMatrix = glm::mat4(1.0f)) const;
        inline void set_material_uniforms(std::function<void(Shader*)>, const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_material_uniforms(std::function<void(Shader*)>, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_material_uniforms(std::function<void(Shader*)>);
        inline void set_material_uniforms(const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_material_uniforms(const mat4& model_transform);

    private:
        std::optional<GLuint> basecolor_gputex;
        std::optional<GLuint> metallic_roughness_gputex;
        double basecolor[3];
        double metallic_factor;
        double roughness_factor;
        Shader* mat_shader;
        Shader* depth_shader;
        mat4 model_transform;
        mat4 view_transform;
        mat4 projection_transform;
};

class GltfPrimitive {
    public:
        GltfPrimitive(tinygltf::Model& root, const tinygltf::Primitive& prim, ShaderStore& shader_store, int emmission);
        void draw(glm::mat4 const& node_transform, glm::mat4 const& lightSpaceMatrix) const ;
        void draw(glm::mat4 const& node_transform, bool no_shader) const;
        inline void set_primitive_uniforms(std::function<void(Shader*)>, const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_primitive_uniforms(std::function<void(Shader*)>, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_primitive_uniforms(std::function<void(Shader*)>);
        inline void set_primitive_uniforms(const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_primitive_uniforms(const mat4& model_transform);
    
    private:
        GLuint vao;
        int draw_mode;
        size_t vertex_count;
        GltfMaterial material;
        mat4 model_transform;
};

class GltfMesh {
    public:
        GltfMesh(tinygltf::Model& root, tinygltf::Mesh mesh, ShaderStore& shader_store, int emmission);
        void draw(glm::mat4 const& node_transform, glm::mat4 const& lightSpaceMatrix) const ;
        void draw(glm::mat4 const& node_transform, bool no_shader) const;
        inline void set_mesh_uniforms(std::function<void(Shader*)>, const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_mesh_uniforms(std::function<void(Shader*)>, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_mesh_uniforms(std::function<void(Shader*)>);
        inline void set_mesh_uniforms(const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_mesh_uniforms(const mat4& model_transform);

    private:
        std::vector<GltfPrimitive> primitives;
};

class GltfNode {
    public:
        GltfNode(tinygltf::Model& root, tinygltf::Node node, ShaderStore& shader_store, mat4 parent_node_transform);
        void draw(glm::mat4 const& lightSpaceMatrix) const ;
        void draw(bool no_shader) const;
        inline void set_node_uniforms(std::function<void(Shader*)>, const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_node_uniforms(std::function<void(Shader*)>, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_node_uniforms(std::function<void(Shader*)>);
        inline void set_node_uniforms(const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix);
        inline void set_node_uniforms(const mat4& model_transform);

    private:
        std::optional<GltfMesh> mesh;
        std::vector<GltfNode> children;
        mat4 node_transform;
};

class GltfModel {
    public:
        GltfModel() {}
        GltfModel(const std::string& filename, ShaderStore& shader_store);
        void draw(glm::mat4 const& lightSpaceMatrix) const ;
        void draw(bool no_shader = false) const;
        void drawWithoutTextures();
        // Warning !!!
        // -----------
        // When setting uniform for a model, don't touch directly (by shader.setMat4) the view and projection matrices set them by passing as arguments to the set_global_uniform function
        void set_global_uniforms(std::function<void(Shader*)>, const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix);
        void set_global_uniforms(std::function<void(Shader*)>, const mat4& view_matrix, const mat4& projection_matrix);
        void set_global_uniforms(std::function<void(Shader*)>);
        void set_global_uniforms(const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix);
        void set_global_uniforms(const mat4& model_transform);
    private:
        tinygltf::Model tiny_model;
        std::vector<GltfNode> children;
};

#endif