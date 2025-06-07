#include <tools/gltfloader.hpp>
#include <tools/shader.hpp>

#include <cstdio>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glad/glad.h>
#include <callbacks.hpp>

#define btogl(x) x ? GL_TRUE : GL_FALSE
#define tget(x, y) std::get<x>(y)

GltfModel::GltfModel(const std::string& filename, ShaderStore& shader_store) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool res = loader.LoadBinaryFromFile(&tiny_model, &err, &warn, filename);
    if (!warn.empty()) {
        printf("[Gltf] WARN: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("[Gltf] ERR: %s\n", err.c_str());
    }

    if (!res) printf("Failed to load glTF: %s\n", filename.c_str());
    else printf("Loaded glTF: %s\n", filename.c_str());

    const tinygltf::Scene& scene = tiny_model.scenes[tiny_model.defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        assert((scene.nodes[i] >= 0) && (static_cast<size_t>(scene.nodes[i]) < tiny_model.nodes.size()));
        children.push_back(GltfNode(tiny_model, tiny_model.nodes[scene.nodes[i]], shader_store, mat4(1.0f)));
    }
}

void GltfModel::draw(bool depth, glm::mat4 const& lightSpaceMatrix) const {
    for (const auto& child: children) {
        child.draw(depth, lightSpaceMatrix);
    }
}

// Warning, when setting uniform for a model, don't touch directly (by shader.setMat4) the view and projection matrices set them by passing them as arguments to the set_global_uniform function
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void GltfModel::set_global_uniforms(std::function<void(Shader*)> uniforms_fn, const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix) {
    for (auto& child: children) {
        child.set_node_uniforms(uniforms_fn, model_transform, view_matrix, projection_matrix);
    }
}

void GltfModel::set_global_uniforms(std::function<void(Shader*)> uniforms_fn, const mat4& view_matrix, const mat4& projection_matrix) {
    for (auto& child: children) {
        child.set_node_uniforms(uniforms_fn, view_matrix, projection_matrix);
    }
}

void GltfModel::set_global_uniforms(std::function<void(Shader*)> uniforms_fn) {
    for (auto& child: children) {
        child.set_node_uniforms(uniforms_fn);
    }
}

void GltfModel::set_global_uniforms(const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix) {
    for (auto& child: children) {
        child.set_node_uniforms(model_transform, view_matrix, projection_matrix);
    }
}

void GltfModel::set_global_uniforms(const mat4& model_transform) {
    for (auto& child: children) {
        child.set_node_uniforms(model_transform);
    }
}

GltfNode::GltfNode(tinygltf::Model& root, tinygltf::Node node, ShaderStore& shader_store, mat4 parent_node_transform) {
    node_transform = mat4(1.0);
    if (node.translation.size() == 3) {
        vec3 translation(node.translation[0], node.translation[1], node.translation[2]);

        node_transform = glm::translate(mat4(1.0), translation);
    }

    if (node.rotation.size() == 4) {
        glm::quat rotation(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
        mat4 rotation_mat = glm::mat4_cast(rotation);

        node_transform *= rotation_mat;
    }

    if (node.scale.size() == 3) {
        vec3 scale(node.scale[0], node.scale[1], node.scale[2]);

        node_transform = glm::scale(node_transform, scale);
    }

    node_transform = parent_node_transform * node_transform;
    
    mesh = ((node.mesh >= 0) && (static_cast<size_t>(node.mesh) < root.meshes.size())) ?
        std::optional(GltfMesh(root, root.meshes[node.mesh], shader_store, node.emitter)) :
        std::nullopt;


    for (size_t i = 0; i < node.children.size(); i++) {
        assert((node.children[i] >= 0) && (static_cast<size_t>(node.children[i]) < root.nodes.size()));
        children.push_back(GltfNode(root, root.nodes[node.children[i]], shader_store, node_transform));
    }
}

void GltfNode::draw(bool depth, glm::mat4 const& lightSpaceMatrix) const {
    for (const auto& child: children) {
        child.draw(depth, lightSpaceMatrix);
    }

    if (mesh.has_value()) {
        mesh.value().draw(node_transform, depth, lightSpaceMatrix);
    }
}

inline void GltfNode::set_node_uniforms(std::function<void(Shader*)> uniforms_fn, const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix) {
    for (auto& child: children) {
        child.set_node_uniforms(uniforms_fn, model_transform, view_matrix, projection_matrix);
    }

    if (this->mesh.has_value()) {
        this->mesh.value().set_mesh_uniforms(uniforms_fn, model_transform, view_matrix, projection_matrix);
    }
}

inline void GltfNode::set_node_uniforms(std::function<void(Shader*)> uniforms_fn, const mat4& view_matrix, const mat4& projection_matrix) {
    for (auto& child: children) {
        child.set_node_uniforms(uniforms_fn, view_matrix, projection_matrix);
    }

    if (this->mesh.has_value()) {
        this->mesh.value().set_mesh_uniforms(uniforms_fn, view_matrix, projection_matrix);
    }
}

inline void GltfNode::set_node_uniforms(std::function<void(Shader*)> uniforms_fn) {
    for (auto& child: children) {
        child.set_node_uniforms(uniforms_fn);
    }

    if (this->mesh.has_value()) {
        this->mesh.value().set_mesh_uniforms(uniforms_fn);
    }
}

inline void GltfNode::set_node_uniforms(const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix) {
    for (auto& child: children) {
        child.set_node_uniforms(model_transform, view_matrix, projection_matrix);
    }

    if (this->mesh.has_value()) {
        this->mesh.value().set_mesh_uniforms(model_transform, view_matrix, projection_matrix);
    }
}

inline void GltfNode::set_node_uniforms(const mat4& model_transform) {
    for (auto& child: children) {
        child.set_node_uniforms(model_transform);
    }

    if (this->mesh.has_value()) {
        this->mesh.value().set_mesh_uniforms(model_transform);
    }
}

GltfMesh::GltfMesh(tinygltf::Model& root, tinygltf::Mesh mesh, ShaderStore& shader_store, int emmission) {
    for (const auto& prim : mesh.primitives) {
        primitives.push_back(GltfPrimitive(root, prim, shader_store, emmission));
    }
}

// Tuple printer (https://stackoverflow.com/a/41171552)
template <class TupleType, size_t... I>
void print_tuple_util(const TupleType &tuple_to_print, std::index_sequence<I...>) {
    std::cout << "(";
    (..., (std::cout << (I == 0? "" : ", ") << std::get<I>(tuple_to_print)));
    std::cout << ")" << std::endl;
}

template <class... T>
void print_tuple(const std::tuple<T...> &tuple_to_print) {
    print_tuple_util(tuple_to_print, std::make_index_sequence<sizeof...(T)>());
}

void GltfMesh::draw(const mat4& node_transform, bool depth, glm::mat4 const& lightSpaceMatrix) const {
    for (const auto& prim: primitives) {
        prim.draw(node_transform, depth, lightSpaceMatrix);
    }
}

inline void GltfMesh::set_mesh_uniforms(std::function<void(Shader*)> uniforms_fn, const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix) {
    for (auto& prim: primitives) {
        prim.set_primitive_uniforms(uniforms_fn, model_transform, view_matrix, projection_matrix);
    }
}

inline void GltfMesh::set_mesh_uniforms(std::function<void(Shader*)> uniforms_fn, const mat4& view_matrix, const mat4& projection_matrix) {
    for (auto& prim: primitives) {
        prim.set_primitive_uniforms(uniforms_fn, view_matrix, projection_matrix);
    }
}

inline void GltfMesh::set_mesh_uniforms(std::function<void(Shader*)> uniforms_fn) {
    for (auto& prim: primitives) {
        prim.set_primitive_uniforms(uniforms_fn);
    }
}

inline void GltfMesh::set_mesh_uniforms(const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix) {
    for (auto& prim: primitives) {
        prim.set_primitive_uniforms(model_transform, view_matrix, projection_matrix);
    }
}

inline void GltfMesh::set_mesh_uniforms(const mat4& model_transform) {
    for (auto& prim: primitives) {
        prim.set_primitive_uniforms(model_transform);
    }
}

GltfPrimitive::GltfPrimitive(tinygltf::Model& root, const tinygltf::Primitive& prim, ShaderStore& shader_store, int emmission) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    typedef struct {
        tinygltf::Accessor accessor;
        tinygltf::BufferView buffer_view;
        tinygltf::Buffer buffer;
    } BufferInfos;
    std::optional<BufferInfos> pos_buf_opt;
    std::optional<BufferInfos> normal_buf;
    std::optional<BufferInfos> texcoord_buf;

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    int stride = 3;

    bool has_normals = false;

    for (const auto &attr : prim.attributes) {
        assert((attr.second >= 0) && (static_cast<size_t>(attr.second) < root.accessors.size()));
        tinygltf::Accessor accessor = root.accessors[attr.second];
        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

        assert((accessor.bufferView >= 0) && (static_cast<size_t>(accessor.bufferView) < root.bufferViews.size()));
        tinygltf::BufferView buffer_view = root.bufferViews[accessor.bufferView];

        assert((buffer_view.buffer >= 0) && (static_cast<size_t>(buffer_view.buffer) < root.buffers.size()));
        tinygltf::Buffer buffer = root.buffers[buffer_view.buffer];

        if (attr.first.compare("POSITION") == 0) {
            pos_buf_opt = std::optional(BufferInfos { accessor, buffer_view, buffer });
            printf("model has positions\n");
        } else if (attr.first.compare("NORMAL") == 0) {
            normal_buf = std::optional(BufferInfos { accessor, buffer_view, buffer });
            stride += 3;
            printf("model has normals\n");
            has_normals = true;
        } else if (attr.first.compare("TEXCOORD_0") == 0) {
            texcoord_buf = std::optional(BufferInfos { accessor, buffer_view, buffer });
            stride += 2;
            printf("model has tex coords\n");
        } else {
            printf("[Gltf] Unsupported attributes found in model: %s -> Skipping !\n",  attr.first.c_str());
            continue;
        }
    }
    assert(pos_buf_opt.has_value());

    BufferInfos pos_buf = pos_buf_opt.value();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    if (texcoord_buf) {
        glVertexAttribPointer(1, 2, GL_FLOAT, btogl(texcoord_buf.value().accessor.normalized), stride * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    if (normal_buf) {
        glVertexAttribPointer(2, 3, GL_FLOAT, btogl(normal_buf.value().accessor.normalized), stride * sizeof(float), (void*)((stride - 3) * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    printf("Vertex count: %zu\n", pos_buf.accessor.count);

    std::vector<unsigned char> final_buffer;
    for (size_t idx = 0; idx < pos_buf.accessor.count; idx++) {
        final_buffer.insert(
            final_buffer.cend(),
            pos_buf.buffer.data.cbegin() + pos_buf.buffer_view.byteOffset + (idx * 3 * sizeof(float)),
            pos_buf.buffer.data.cbegin() + pos_buf.buffer_view.byteOffset + ((idx + 1) * 3 * sizeof(float))
        );

        if (texcoord_buf) {
            final_buffer.insert(
                final_buffer.cend(),
                texcoord_buf.value().buffer.data.cbegin() + texcoord_buf.value().buffer_view.byteOffset + (idx * 2 * sizeof(float)),
                texcoord_buf.value().buffer.data.cbegin() + texcoord_buf.value().buffer_view.byteOffset + ((idx + 1) * 2 * sizeof(float))
            );
        }

        if (normal_buf) {
            final_buffer.insert(
                final_buffer.cend(),
                normal_buf.value().buffer.data.cbegin() + normal_buf.value().buffer_view.byteOffset + (idx * 3 * sizeof(float)),
                normal_buf.value().buffer.data.cbegin() + normal_buf.value().buffer_view.byteOffset + ((idx + 1) * 3 * sizeof(float))
            );
        }
    }

    glBufferData(GL_ARRAY_BUFFER, final_buffer.size(), final_buffer.data(), GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    assert((prim.indices >= 0) && (static_cast<size_t>(prim.indices) < root.accessors.size()));
    tinygltf::Accessor indices_accessor = root.accessors[prim.indices];
    
    assert((indices_accessor.bufferView >= 0) && (static_cast<size_t>(indices_accessor.bufferView) < root.bufferViews.size()));
    tinygltf::BufferView indices_buffer_view = root.bufferViews[indices_accessor.bufferView];

    assert((indices_buffer_view.buffer >= 0) && (static_cast<size_t>(indices_buffer_view.buffer) < root.buffers.size()));
    tinygltf::Buffer indices_buffer = root.buffers[indices_buffer_view.buffer];

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_view.byteLength, indices_buffer.data.data() + indices_buffer_view.byteOffset, GL_STATIC_DRAW);

    glBindVertexArray(0);

    assert((prim.material >= 0) && (static_cast<size_t>(prim.material) < root.materials.size()));
    tinygltf::Material mat = root.materials[prim.material];

    material = GltfMaterial(root, mat, shader_store, has_normals);

    draw_mode = prim.mode;
    vertex_count = indices_accessor.count;
}

void GltfPrimitive::draw(const mat4& node_transform, bool depth, glm::mat4 const& lightSpaceMatrix) const {
    glBindVertexArray(vao);

    material.activate(node_transform, depth, lightSpaceMatrix);

    glDrawElements(draw_mode, vertex_count, GL_UNSIGNED_SHORT, static_cast<void*>(0));
    glBindVertexArray(0);
}

inline void GltfPrimitive::set_primitive_uniforms(std::function<void(Shader*)> uniforms_fn, const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix) {
    material.set_material_uniforms(uniforms_fn, model_transform, view_matrix, projection_matrix);
}

inline void GltfPrimitive::set_primitive_uniforms(std::function<void(Shader*)> uniforms_fn, const mat4& view_matrix, const mat4& projection_matrix) {
    material.set_material_uniforms(uniforms_fn, view_matrix, projection_matrix);
}

inline void GltfPrimitive::set_primitive_uniforms(std::function<void(Shader*)> uniforms_fn) {
    material.set_material_uniforms(uniforms_fn);
}

inline void GltfPrimitive::set_primitive_uniforms(const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix) {
    material.set_material_uniforms(model_transform, view_matrix, projection_matrix);
}

inline void GltfPrimitive::set_primitive_uniforms(const mat4& model_transform) {
    material.set_material_uniforms(model_transform);
}

GLuint load_texture_to_gpu(tinygltf::Model& root, tinygltf::TextureInfo texinfo) {
    assert((texinfo.index >= 0) && (static_cast<size_t>(texinfo.index) < root.textures.size()));
    tinygltf::Texture gltftex = root.textures[texinfo.index];

    assert((gltftex.source >= 0) && (static_cast<size_t>(gltftex.source) < root.images.size()));
    tinygltf::Image image = root.images[gltftex.source];

    assert((gltftex.sampler >= 0) && (static_cast<size_t>(gltftex.sampler) < root.textures.size()));
    tinygltf::Sampler sampler = root.samplers[gltftex.sampler];

    GLenum tex_components/*, tex_bits*/;
    switch (image.component) {
        case 3:
            tex_components = GL_RGB;
            //tex_bits = GL_RGB8;
            break;

        case 4:
            tex_components = GL_RGBA;
            //tex_bits = GL_RGBA8;
            break;
        default:
            printf("%d components isn't supported right now", image.component);
            throw -1;
    }

    assert(image.as_is == false);

    GLuint gputex;
    glGenTextures(1, &gputex);
    glBindTexture(GL_TEXTURE_2D, gputex);
    glTexImage2D(GL_TEXTURE_2D, 0, tex_components, image.width, image.height, 0, tex_components, image.pixel_type, image.image.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);

    return gputex;
}

GltfMaterial::GltfMaterial(tinygltf::Model& root, tinygltf::Material material, ShaderStore& shader_store, bool has_normals) {
    std::copy(material.pbrMetallicRoughness.baseColorFactor.cbegin(), material.pbrMetallicRoughness.baseColorFactor.cbegin() + 3, basecolor);
    metallic_factor = material.pbrMetallicRoughness.metallicFactor;
    roughness_factor = material.pbrMetallicRoughness.roughnessFactor;

    tinygltf::TextureInfo basecolor_texinfo = material.pbrMetallicRoughness.baseColorTexture;
    tinygltf::TextureInfo metallic_roughness_texinfo = material.pbrMetallicRoughness.metallicRoughnessTexture;

    std::bitset<4> shader_features;

    printf("Loading textures...\n");
    if (basecolor_texinfo.index >= 0) {
        basecolor_gputex = std::optional(load_texture_to_gpu(root, basecolor_texinfo));

        shader_features.set(1);
    } else {
        basecolor_gputex = std::nullopt;
    }

    if (metallic_roughness_texinfo.index >= 0) {
        metallic_roughness_gputex = std::optional(load_texture_to_gpu(root, metallic_roughness_texinfo));

        shader_features.set(2);
    } else {
        metallic_roughness_gputex = std::nullopt;
    }

    if (has_normals) {
        shader_features.set(0);
    }

    shader_features[3] = false;

    mat_shader = &shader_store.get_shader(shader_features);

    shader_features[3] = true;

    depth_shader = &shader_store.get_shader(shader_features);

    printf("Textures loaded !\n");
}

inline void GltfMaterial::activate(const mat4& node_transform, bool depth, glm::mat4 const& lightSpaceMatrix) const {
    if (depth){
        depth_shader->use();
        mat4 depth_transform = lightSpaceMatrix * node_transform * model_transform;
        depth_shader->setMat4("depth_transform", depth_transform);
    }
    else {
        mat_shader->use();
        mat4 modelMat = node_transform * model_transform;
        mat_shader->setMat4("model", modelMat);
        mat_shader->setMat4("transform", projection_transform * view_transform * modelMat);

        if (basecolor_gputex.has_value()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, basecolor_gputex.value());
        } else {
            mat_shader->setVec3("base_color", glm::vec3(basecolor[0], basecolor[1], basecolor[2]));
        }

        if (metallic_roughness_gputex.has_value()) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, metallic_roughness_gputex.value());
        } else {
            mat_shader->setFloat("metallic_factor", metallic_factor);
            mat_shader->setFloat("roughness_factor", roughness_factor);
        }
    }
}

inline void GltfMaterial::set_material_uniforms(std::function<void(Shader*)> uniforms_fn, const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix) {
    uniforms_fn(mat_shader);
    this->model_transform = model_transform;
    this->view_transform = view_matrix;
    this->projection_transform = projection_matrix;
}

inline void GltfMaterial::set_material_uniforms(std::function<void(Shader*)> uniforms_fn, const mat4& view_matrix, const mat4& projection_matrix) {
    uniforms_fn(mat_shader);
    this->view_transform = view_matrix;
    this->projection_transform = projection_matrix;
}

inline void GltfMaterial::set_material_uniforms(std::function<void(Shader*)> uniforms_fn) {
    uniforms_fn(mat_shader);
}

inline void GltfMaterial::set_material_uniforms(const mat4& model_transform, const mat4& view_matrix, const mat4& projection_matrix) {
    this->model_transform = model_transform;
    this->view_transform = view_matrix;
    this->projection_transform = projection_matrix;
}

inline void GltfMaterial::set_material_uniforms(const mat4& model_transform) {
    this->model_transform = model_transform;
}