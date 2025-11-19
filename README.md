## Setup for developers
### Required packages
```bash
$ sudo apt install build-essential ninja-build meson clang cmake libgtkmm-3.0-dev libglfw3-dev libglew-dev libglm-dev libgl-dev libepoxy-dev
```

### Testing / development
```bash
$ sudo apt install libgtest-dev clang-tidy clang-format pre-commit
```

### Installing latest version of clang and tools
**Recommended for development, standard packages are outdated!**
```bash
$ wget https://apt.llvm.org/llvm.sh
$ chmod +x llvm.sh
$ sudo ./llvm.sh 21

$ pip install meson pre-commit
```

### Pre-commit setup
```bash
$ pre-commit install
$ pre-commit run --all-files  # only need to do this once to init environment
```

### Meson project setup
```bash
$ CXX=clang++-21 meson setup buildDir
```

### Meson project compilation
```bash
$ cd buildDir
$ meson compile
```
