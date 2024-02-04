--- @diagnostic disable:undefined-global

add_rules('mode.debug', 'mode.release')

add_requires('efwmcwalnut')
add_requires('imgui-walnut walnut', { configs = { glfw = true, vulkan = true } })
add_requires('glfw-walnut walnut', { configs = { glfw_include = 'vulkan' } })
add_requires('spdlog', 'fmt', 'magic_enum')

-- main app
target('wgpuapp')
set_languages('c++20')
set_kind('binary')
add_files('**.cpp')
add_includedirs('.')
add_defines('RESOURCE_DIR="./wgpu"')
add_defines('WEBGPU_BACKEND_WGPU')
-- packges with link need
add_packages('spdlog', 'fmt', 'magic_enum')
add_packages('efwmcwalnut')
add_packages('glfw-walnut', 'imgui-walnut')
-- local packges with include and link need
add_includedirs('$(projectdir)/vendor/glfw3webgpu')
add_includedirs('$(projectdir)/vendor/webgpu/include')
add_includedirs('$(projectdir)/vendor/webgpu/include/webgpu')
add_linkdirs('$(projectdir)/vendor/webgpu/bin/linux-x86_64')
add_links('wgpu')
after_build(function(target)
    os.cp('$(scriptdir)/resources/shaders/wgpu', target:targetdir())
end)
target_end()

-- second
add_requires('stb')
includes('RendererCornell/module.lua')

-- target('renderer_cornell')
-- set_kind('binary')
-- add_files('RendererCornell/app.cpp')
-- add_deps('cornell_raytracing')
-- add_includedirs('RendererCornell')
-- target_end()
--
-- target('cornell_raytracing')
-- set_kind('$(kind)')
-- add_rules('module')
-- add_files('RendererCornell/raytracing/**.cpp')
-- add_includedirs('RendererCornell')
-- add_deps('cornell_util')
-- target_end()
--
-- target('cornell_util')
-- set_kind('$(kind)')
-- add_rules('module')
-- add_files('RendererCornell/util/**.cpp')
-- add_includedirs('RendererCornell')
-- add_packages('stb')
-- on_load(function(target)
--     if is_host('windows') then
--         target:add('defines', '_CRT_SECURE_NO_WARNINGS')
--         target:add('defines', '__cpp_consteval=201811', { public = true })
--     end
-- end)
-- target_end()
