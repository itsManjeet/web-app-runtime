executable(
    meson.project_name(),
    'main.c',
    'rt-app.h', 'rt-app.c',
    'rt-window.h', 'rt-window.c',
    'rt-webview.h', 'rt-webview.c',
    dependencies : [
        dependency('glib-2.0'),
        dependency('gtk+-3.0'),
        dependency('webkit2gtk-4.1'),
    ],
    install : true)