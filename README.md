## Features

- `focusorexec`: A dispatcher to focus the last used window having the given
  application ID, or execute a command if no window with that application ID
  exists.

  <details>
    <summary>Configuration</summary>
    There can be up to 47 <i>quick access apps</i>. Why not take the app ID
    and exec commands as arguments in the dispatcher? Because it takes more
    clock cycles to pass strings around and find delimiters in them than to look
    up arrays. Configuration is only loaded once.
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

    bind = SUPER, 1, myplugin:focusorexec:1
    bind = SUPER, 2, myplugin:focusorexec:2
    ```
  </details>

- Keep tiled windows grouped. (The `group set` window rule only works when the
  window is opened.)

  To enable this, generate the build sytem with `-DGROUP_TILED_WINDOWS=ON`.

  <details>
    <summary>This also destroys groups of floating windows.</summary>

    My configuration has window rules to set certain windows to be floating. The
    plugin code groups them too. I do not see the usefulness of grouping
    floating windows so I do not want to spend time figuring out how Hyprland
    applies window rules. So, I simply destroy floating window groups whenever
    they are created.
  </details>

## Installation

Hyprland headers need to be installed. See
[`CMakeLists.txt`](https://github.com/adityasz/myplugin/blob/master/CMakeLists.txt)
for the full list of dependencies.

```console
$ cmake -B build/release -S . -GNinja -DCMAKE_BUILD_TYPE=Release [-DGROUP_TILED_WINDOWS=<ON|OFF*>]
$ cmake --build build/release
$ hyprctl plugin load $(realpath build/release/libmyplugin.so)
```
