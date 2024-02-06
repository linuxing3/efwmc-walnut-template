--- @diagnostic disable:undefined-global

add_rules('mode.debug', 'mode.releasedbg', 'mode.release')

if is_mode('debug') then
    set_optimize('none')
    set_runtimes('MDd')
elseif is_mode('releasedbg') then
    set_optimize('fastest')
    set_runtimes('MD')
    set_policy('build.optimization.lto', true)
elseif is_mode('release') then
    set_strip('all')
    set_symbols('hidden')
    set_optimize('fastest')
    set_runtimes('MT')
    add_defines('NDEBUG')
    set_policy('build.optimization.lto', true)
end

if is_mode('debug', 'releasedbg') then
    set_symbols('debug')
    set_policy('build.warning', true)
    -- use dynamic libraries acceleration on linking
    add_requireconfs('*', { configs = { shared = true } })
end
-- support utf-8 on msvc
if is_host('windows') then
    add_defines('UNICODE', '_UNICODE')
    add_cxflags('/utf-8', { tools = 'cl' })
end
-- disable sqrt function negative check
add_cxxflags('-fno-math-errno', { tools = { 'gcc', 'clang' } })

add_requires('efwmcwalnut')
add_requires('imgui-walnut walnut', { configs = { glfw = true, vulkan = true } })
add_requires('glfw-walnut walnut', { configs = { glfw_include = 'vulkan' } })
add_requires('spdlog', 'fmt', 'magic_enum')

-- main app
target('raytracing_example_app')
set_languages('c++20')
set_kind('binary')
add_files('**.cpp')
add_includedirs('.')
add_defines('RESOURCE_DIR="./wgpu"')
add_defines('WEBGPU_BACKEND_WGPU')
set_targetdir('.')
-- packges with link need
add_packages('spdlog', 'fmt', 'magic_enum')
add_packages('efwmcwalnut')
add_packages('glfw-walnut', 'imgui-walnut')
-- local packges with include and link need
add_includedirs('$(projectdir)/vendor/webgpu/include')
add_includedirs('$(projectdir)/vendor/webgpu/include/webgpu')
add_linkdirs('$(projectdir)/vendor/webgpu/bin/linux-x86_64')
add_links('wgpu', 'pthread', 'tbb')
add_deps('glfw3webgpu')
add_links('glfw3webgpu')
after_build(function(target)
    os.cp('$(projectdir)/resources/shaders/wgpu', target:targetdir())
end)
target_end()
