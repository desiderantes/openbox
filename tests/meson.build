manual_tests = [
    'aspect',
    'big',
    'borderchange',
    'confignotify',
    'confignotifymax',
    'cursorio',
    'duplicatesession',
    'extentsrequest',
    'fakeunmap',
    'fallback',
    'focusout',
    'fullscreen',
    'grav',
    'groupmodal',
    'grouptran',
    'grouptran2',
    'grouptrancircular',
    'grouptrancircular2',
    'iconifydelay',
    'icons',
    'mapiconic',
    'mingrow',
    'modal',
    'modal2',
    'modal3',
    'noresize',
    'oldfullscreen',
    'override',
    'overrideinputonly',
    'positioned',
    'resize',
    'restack',
    'showhide',
    'skiptaskbar',
    'skiptaskbar2',
    'stackabove',
    'stacking',
    'strut',
    'title',
    'urgent',
    'usertimewin',
    'wmhints',
]

foreach name : manual_tests
    executable(
        name,
        files(f'@name@.c'),
        dependencies: [glib_dep, x11_dep],
        install: false,
    )
endforeach

if use_xshape
    executable(
        'shape',
        files('shape.c'),
        dependencies: [glib_dep, x11_dep, xshape_dep],
        install: false,
    )
endif
