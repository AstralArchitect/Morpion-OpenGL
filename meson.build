project('Morpion-OpenGL', 'c', 'cpp', version : 'pre-release', default_options: ['cpp_std=c++20', 'default_library=static'])

cpp_compiler = meson.get_compiler('cpp')
if cpp_compiler.get_id() == 'gcc'
  add_global_arguments('-std=gnu++20', language: 'cpp')
endif

cpp_dir = 'src/'

personnal_cpp_libs = cpp_dir + 'libs/'

tools_dir = cpp_dir + 'tools/'

opengl_lib = cpp_dir + 'OpenGL_libs/'

cpp_sources = [cpp_dir + 'main.cpp', cpp_dir + 'render.cpp']
opengl_libs_sources = [opengl_lib + 'glad.c']
personnal_cpp_sources = [cpp_dir + 'callbacks.cpp']
tools_sources = [tools_dir + 'camera.cpp', tools_dir + 'shader.cpp', tools_dir + 'gltfloader.cpp']

sources = [cpp_sources, opengl_libs_sources, personnal_cpp_sources, tools_sources]
include_dir = include_directories('Include')
glfw_dep = dependency('glfw3')
glm_dep = dependency('glm')

cmake = import('cmake')
opt_var = cmake.subproject_options()
opt_var.add_cmake_defines({
    'TINYGLTF_BUILD_LOADER_EXAMPLE': false,
    'TINYGLTF_HEADER_ONLY': false,
    'TINYGLTF_INSTALL': false
})

tinygltf = cmake.subproject('tinygltf', options: opt_var)
tinygltf_dep = tinygltf.dependency('tinygltf')

executable('main', sources: sources, include_directories: include_dir, dependencies: [glfw_dep, glm_dep, tinygltf_dep])

