session_scripts = [
    'openbox-session',
    'openbox-gnome-session',
    'openbox-kde-session',
]

foreach script : session_scripts
    configure_file(
        input: f'@script@.in',
        output: script,
        configuration: script_conf,
        install: true,
        install_dir: get_option('bindir'),
        install_mode: 'rwxr-xr-x',
    )
endforeach

xsession_desktops = [
    'openbox.desktop',
    'openbox-gnome.desktop',
    'openbox-kde.desktop',
]

foreach desktop : xsession_desktops
    configure_file(
        input: f'@desktop@.in',
        output: desktop,
        configuration: script_conf,
        install: true,
        install_dir: install_xsessionsdir,
    )
endforeach
