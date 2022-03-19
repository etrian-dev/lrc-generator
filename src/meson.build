curses_dep = dependency('ncurses')
sfml_dep = dependency('sfml-audio')
deps = [curses_dep, sfml_dep]
sources = ['main.cpp', 'lrc-generator.cpp', 'lrc-interface.cpp']
executable('lrg-gen', sources, dependencies: deps, include_directories: includes)
