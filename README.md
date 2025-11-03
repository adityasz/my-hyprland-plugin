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

  To enable this, generate the build sytem with `-DTILES_ARE_GROUPS=ON`.

  <details>
    <summary>This also destroys groups of floating windows.</summary>

    My configuration has window rules to set certain windows to be floating. The
    plugin code groups them too. I do not see the usefulness of grouping
    floating windows so I do not want to spend time figuring out how Hyprland
    applies window rules. So, I simply destroy floating window groups whenever
    they are created.
  </details>

- `moveintogrouporexec`: Available when `TILES_ARE_GROUPS=ON`. Similar to
  `moveorexec`, but inserts into the current group if possible.

# Configuration

```hyprlang
plugin {
    myplugin {
        app_0 {
            class = kitty
            command = kitty
        }
        app_1 {
            class = org.kde.dolphin
            command = dolphin
        }
    }
}

bind = SUPER,       0, myplugin:focusorexec, 0
bind = SUPER SHIFT, 0, myplugin:moveorexec,  0
bind = SUPER CTRL,  0, myplugin:exec,        0
bind = SUPER,       1, myplugin:focusorexec, 1
bind = SUPER SHIFT, 1, myplugin:moveorexec,  1
bind = SUPER CTRL,  1, myplugin:exec,        1
```

> [!NOTE]
>
> There can be up to 47 of these *quick access apps*.

## Installation

Hyprland headers need to be installed. See
[`CMakeLists.txt`](https://github.com/adityasz/myplugin/blob/master/CMakeLists.txt)
for the full list of dependencies.

```console
$ cmake -B build/release -S . -GNinja -DCMAKE_BUILD_TYPE=Release [-DTILES_ARE_GROUPS=<ON|OFF*>]
$ cmake --build build/release
$ hyprctl plugin load $(realpath build/release/libmyplugin.so)
```

Debug logs can be enabled with `-DDEBUG_LOGS=ON`.
