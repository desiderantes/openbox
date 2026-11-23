install_data(
    'rc-mouse-focus.xml',
    install_dir: install_docdir,
)

manpages = [
    'openbox.1',
    'openbox-session.1',
    'openbox-gnome-session.1',
    'openbox-kde-session.1',
    'obxprop.1',
]

foreach man : manpages
    man_file = configure_file(
        input: f'@man@.in',
        output: man,
        configuration: script_conf,
    )
    install_man(man_file)
endforeach
