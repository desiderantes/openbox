install_data(
    'rc.xml',
    'menu.xml',
    install_dir: install_rcdir,
)

install_data(
    'environment',
    install_dir: install_rcdir,
    install_mode: 'rwxr-xr-x',
)

install_data(
    'openbox.desktop',
    install_dir: install_appsdir,
)

install_data(
    'openbox.png',
    install_dir: install_pixmapdir,
)

install_data(
    'rc.xsd',
    'menu.xsd',
    install_dir: install_docdir,
)

install_data(
    'gnome-wm-properties/openbox.desktop',
    install_dir: install_gnomewmfilesdir,
)

install_data(
    'gnome-session/openbox-gnome.session',
    'gnome-session/openbox-gnome-fallback.session',
    install_dir: install_gnomesessiondir,
)

install_data(
    'xbm/bullet.xbm',
    'xbm/close.xbm',
    'xbm/desk.xbm',
    'xbm/desk_toggled.xbm',
    'xbm/iconify.xbm',
    'xbm/max.xbm',
    'xbm/max_toggled.xbm',
    'xbm/shade.xbm',
    'xbm/shade_toggled.xbm',
    install_dir: install_docxbmdir,
)

subdir('autostart')
subdir('xsession')
