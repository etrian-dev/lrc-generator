curses_dep = dependency('curses')
sfml_dep = dependency('sfml-audio')
deps = [curses_dep, sfml_dep]
loguru_dirs = include_directories('../loguru')
cxxopts_dirs = include_directories('../cxxopts/include')
sources = [
  'main.cpp',
  'lrc-generator.cpp',
  'lrc-interface.cpp',
  '../loguru/loguru.cpp'
]
executable('lrc-generator', sources, dependencies: deps, include_directories: [includes, loguru_dirs, cxxopts_dirs], install: true)
