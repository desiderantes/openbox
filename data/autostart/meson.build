install_data(
    'openbox-xdg-autostart',
    install_dir: get_option('libexecdir'),
    install_mode: 'rwxr-xr-x',
)

configure_file(
    input: 'autostart.in',
    output: 'autostart',
    configuration: script_conf,
    install: true,
    install_dir: install_rcdir,
    install_mode: 'rwxr-xr-x',
)

configure_file(
    input: 'openbox-autostart.in',
    output: 'openbox-autostart',
    configuration: script_conf,
    install: true,
    install_dir: get_option('libexecdir'),
    install_mode: 'rwxr-xr-x',
)
