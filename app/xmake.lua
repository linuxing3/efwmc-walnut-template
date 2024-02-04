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
add_includedirs('$(projectdir)/vendor/webgpu/include')
add_includedirs('$(projectdir)/vendor/webgpu/include/webgpu')
add_linkdirs('$(projectdir)/vendor/webgpu/bin/linux-x86_64')
add_links('wgpu')
add_deps('glfw3webgpu')
add_links('glfw3webgpu')
after_build(function(target)
    os.cp('$(scriptdir)/resources/shaders/wgpu', target:targetdir())
end)
target_end()
