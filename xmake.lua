--- @diagnostic disable:undefined-global

set_project('efwmcwalnutapp')

set_version('0.0.1')

set_warnings('all')
set_languages('c++20')

set_allowedplats('windows', 'linux', 'macosx')

includes('vendor')
includes('examples')
