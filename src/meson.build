curses_dep = dependency('ncurses')
sfml_dep = dependency('sfml-audio')
th_dep = dependency('threads')
deps = [curses_dep, sfml_dep]
sources = ['main.cpp', 'lrc-generator.cpp', 'lrc-interface.cpp']
executable('lrc-gen', sources, dependencies: deps, include_directories: includes)
