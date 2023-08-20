curses_dep = dependency('curses')
sfml_dep = dependency('sfml-audio')
loguru_dep = dependency('loguru')
cli11_dep = dependency('cli11')

deps = [curses_dep, sfml_dep, loguru_dep, cli11_dep]

sources = [
  'main.cpp',
  'lrc-generator.cpp',
  'lrc-interface.cpp',
]
executable('lrc-generator', sources, dependencies: deps, include_directories: includes, install: true)
