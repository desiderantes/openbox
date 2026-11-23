themes = {
    'Artwiz-boxed': [
        'themerc',
    ],
    'Bear2': [
        'close.xbm',
        'close_pressed.xbm',
        'desk.xbm',
        'desk_toggled.xbm',
        'iconify.xbm',
        'iconify_pressed.xbm',
        'max.xbm',
        'max_pressed.xbm',
        'max_toggled.xbm',
        'shade.xbm',
        'shade_pressed.xbm',
        'themerc',
    ],
    'Clearlooks': [
        'themerc',
    ],
    'Clearlooks-3.4': [
        'themerc',
    ],
    'Clearlooks-Olive': [
        'themerc',
    ],
    'Mikachu': [
        'bullet.xbm',
        'close.xbm',
        'desk.xbm',
        'iconify.xbm',
        'max.xbm',
        'themerc',
    ],
    'Natura': [
        'close.xbm',
        'close_hover.xbm',
        'desk.xbm',
        'desk_hover.xbm',
        'desk_toggled.xbm',
        'iconify.xbm',
        'iconify_hover.xbm',
        'max.xbm',
        'max_hover.xbm',
        'max_toggled.xbm',
        'shade.xbm',
        'shade_hover.xbm',
        'themerc',
    ],
    'Onyx': [
        'themerc',
    ],
    'Onyx-Citrus': [
        'themerc',
    ],
    'Orang': [
        'themerc',
    ],
    'Syscrash': [
        'max.xbm',
        'max_disabled.xbm',
        'max_pressed.xbm',
        'max_toggled.xbm',
        'themerc',
    ],
}

foreach theme_name, theme_files : themes
    theme_paths = []
    foreach f : theme_files
        theme_paths += theme_name / 'openbox-3' / f
    endforeach
    install_data(
        theme_paths,
        install_dir: install_themedir / theme_name / 'openbox-3',
    )
endforeach
