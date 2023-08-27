curses_dep = dependency('curses')
sfml_dep = dependency('sfml-audio')
loguru_dep = dependency('loguru')
cxxopts_dep = dependency('cxxopts')

deps = [curses_dep, sfml_dep, loguru_dep, cxxopts_dep]

sources = [
  'main.cpp',
  'lrc-generator.cpp',
  'lrc-interface.cpp',
]
executable('lrc-generator', sources, dependencies: deps, include_directories: includes, install: true)
