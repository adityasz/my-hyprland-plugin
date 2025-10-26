### A Hyprland plugin that provides a dispatcher to focus or launch apps

#### Installation

Hyprland headers need to be installed. See
[`CMakeLists.txt`](https://github.com/adityasz/myplugin/blob/master/CMakeLists.txt)
for the full list of dependencies.

```console
$ cmake -B build/release -S . -GNinja -DCMAKE_BUILD_TYPE=Release
$ cmake --build build/release
$ hyprctl plugin load $(realpath build/release/libmyplugin.so)
```

```hyprlang
plugin {
    myplugin {
        app_1 {
            class = kitty
            command = kitty
        }
        app_2 {
            class = zen
            command = zen-browser
        }
    }
}

bind = SUPER, 1, myplugin:1
bind = SUPER, 2, myplugin:2
```
