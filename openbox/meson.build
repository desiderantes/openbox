openbox_sources = files(
    'actions/addremovedesktop.c',
    'actions/all.c',
    'actions/breakchroot.c',
    'actions/close.c',
    'actions/cyclewindows.c',
    'actions/debug.c',
    'actions/decorations.c',
    'actions/desktop.c',
    'actions/directionalwindows.c',
    'actions/dock.c',
    'actions/dockautohide.c',
    'actions/execute.c',
    'actions/exit.c',
    'actions/focus.c',
    'actions/focusfallback.c',
    'actions/focustobottom.c',
    'actions/fullscreen.c',
    'actions/growtoedge.c',
    'actions/iconify.c',
    'actions/if.c',
    'actions/kill.c',
    'actions/layer.c',
    'actions/lower.c',
    'actions/maximize.c',
    'actions/move.c',
    'actions/moverelative.c',
    'actions/moveresizeto.c',
    'actions/movetoedge.c',
    'actions/omnipresent.c',
    'actions/raise.c',
    'actions/raiselower.c',
    'actions/reconfigure.c',
    'actions/resize.c',
    'actions/resizerelative.c',
    'actions/restart.c',
    'actions/shade.c',
    'actions/shadelowerraise.c',
    'actions/showdesktop.c',
    'actions/showmenu.c',
    'actions/unfocus.c',
    'actions.c',
    'client.c',
    'client_list_combined_menu.c',
    'client_list_menu.c',
    'client_menu.c',
    'config.c',
    'debug.c',
    'dock.c',
    'event.c',
    'focus.c',
    'focus_cycle.c',
    'focus_cycle_indicator.c',
    'focus_cycle_popup.c',
    'frame.c',
    'framerender.c',
    'grab.c',
    'group.c',
    'keyboard.c',
    'keytree.c',
    'menu.c',
    'menuframe.c',
    'mouse.c',
    'moveresize.c',
    'openbox.c',
    'ping.c',
    'place.c',
    'place_overlap.c',
    'popup.c',
    'prompt.c',
    'resist.c',
    'screen.c',
    'session.c',
    'stacking.c',
    'startupnotify.c',
    'translate.c',
    'window.c',
)

openbox_deps = [
    obrender_dep,
    obt_dep,
    glib_dep,
    pango_dep,
    pangoxft_dep,
    xml_dep,
    x11_dep,
]
if xinerama_dep.found()
    openbox_deps += xinerama_dep
endif
if xrandr_dep.found()
    openbox_deps += xrandr_dep
endif
if use_xshape
    openbox_deps += xshape_dep
endif
if use_xsync
    openbox_deps += xsync_dep
endif
if xcursor_dep.found()
    openbox_deps += xcursor_dep
endif
if libsn_dep.found()
    openbox_deps += libsn_dep
endif
if use_sm
    openbox_deps += [sm_dep, ice_dep]
endif
if use_nls
    openbox_deps += intl_dep
endif

executable(
    'openbox',
    openbox_sources,
    include_directories: root_inc,
    dependencies: openbox_deps,
    c_args: [
        '-DG_LOG_DOMAIN="Openbox"',
        f'-DLOCALEDIR="@localedir@"',
        f'-DDATADIR="@datadir@"',
        f'-DCONFIGDIR="@configdir@"',
    ],
    export_dynamic: true,
    install: true,
)
