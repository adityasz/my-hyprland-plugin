## Features

- `focusorexec`: A dispatcher to focus the last used window of an app or launch
  it.

- `moveorexec`: A dispatcher to focus the last used window of an app, moving it
  to the active workspace on the active monitor if needed, or launch it if there
  is no window.

  If the window is in a group on a different workspace, it is removed from that
  group prior to being moved.

- `exec`: A dispatcher to launch (a new window of) an app.

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

# Configuration

```hyprlang
plugin {
    myplugin {
        app_1 {
            class = kitty
            command = kitty
        }
    }
}

bind = SUPER,       1, myplugin:focusorexec:1
bind = SUPER SHIFT, 1, myplugin:moveorexec:1
bind = SUPER CTRL,  1, myplugin:exec:1
```

<details>
  <summary>Note</summary>
  There can be up to 47 of these <i>quick access apps</i>. Why not take the app
  ID and exec commands as arguments in the dispatchers? Because it takes more
  clock cycles to pass strings around and process them than to index arrays.
  Configuration is only loaded once. Also, the strings are shared here, so there
  is no redundancy.
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
