# Setup for developers
## Required packages
### Ubuntu
```bash
$ sudo apt install build-essential ninja-build cmake libgtkmm-3.0-dev libglfw3-dev libglew-dev libglm-dev libgl-dev libepoxy-dev libgtest-dev
$ pip install meson pre-commit

$ wget https://apt.llvm.org/llvm.sh
$ chmod +x llvm.sh
$ sudo ./llvm.sh 21 all

$ sudo update-alternatives /usr/bin/clang++ clang++ /usr/bin/clang++-21 1
$ sudo update-alternatives /usr/bin/clang-format clang-format /usr/bin/clang-format-21 1
$ sudo update-alternatives /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-21 1
$ sudo update-alternatives /usr/bin/run-clang-tidy run-clang-tidy /usr/bin/run-clang-tidy-21 1
```
### Arch
```bash
$ sudo pacman -S base-devel ninja clang clang-tools-extra cmake gtkmm3 glfw glew glm libepoxy gtest pre-commit meson
```

## Pre-commit setup
```bash
$ pre-commit install
$ pre-commit run --all-files  # only need to do this once to init environment
```

## Meson project setup
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
