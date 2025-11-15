## Setup for developers
### Initial setup
```bash
$ sudo apt install build-essential ninja-build meson clang cmake libgtkmm-3.0-dev libglfw3-dev libglew-dev libglm-dev libgl-dev libepoxy-dev libgtest-dev
```

### Meson project setup
```bash
$ CXX=clang++ meson setup buildDir
```

### Meson project compilation
```bash
$ cd buildDir
$ meson compile
```
