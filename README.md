## Setup for developers
### Initial setup
```bash
$ sudo apt install build-essential ninja-build meson clang cmake libgtkmm-3.0-dev libglfw3-dev libglew-dev libglm-dev libgl-dev libepoxy-dev libgtest-dev
```

### Meson project setup
```bash
$ CXX=clang++ meson setup buildDir --prefix=$HOME/.local
$ meson install -C buildDir --tags ui  # currently this only installs ui file
```

### Meson project compilation and running
```bash
$ meson compile -C buildDir
$ ./buildDir/src/di-renderer
```

### Meson project testing
```bash
$ meson test -C buildDir
```
