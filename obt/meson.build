obt_includedir = get_option('includedir') / 'openbox' / obt_version / 'obt'

obt_version_conf = configuration_data()
obt_version_conf.set('OBT_MAJOR_VERSION', obt_major_version)
obt_version_conf.set('OBT_MINOR_VERSION', obt_minor_version)
obt_version_conf.set('OBT_MICRO_VERSION', obt_micro_version)

obt_version_h = configure_file(
    input: 'version.h.in',
    output: 'version.h',
    configuration: obt_version_conf,
    install: true,
    install_dir: obt_includedir,
)

obt_public_headers = files(
    'display.h',
    'keyboard.h',
    'link.h',
    'paths.h',
    'prop.h',
    'signal.h',
    'xml.h',
    'xqueue.h',
)

install_headers(
    obt_public_headers,
    install_dir: obt_includedir,
)

obt_sources = files(
    'display.c',
    'keyboard.c',
    'link.c',
    'paths.c',
    'prop.c',
    'signal.c',
    'xml.c',
    'xqueue.c',
)

obt_deps = [
    x11_dep,
    glib_dep,
    xml_dep,
]
if xinerama_dep.found()
    obt_deps += xinerama_dep
endif
if xrandr_dep.found()
    obt_deps += xrandr_dep
endif
if use_xshape
    obt_deps += xshape_dep
endif
if use_xsync
    obt_deps += xsync_dep
endif

libobt = library(
    'obt',
    obt_sources,
    version: obt_so_version,
    soversion: obt_soversion,
    gnu_symbol_visibility: 'hidden',
    include_directories: root_inc,
    implicit_include_directories: false,
    dependencies: obt_deps,
    c_args: [
        '-DG_LOG_DOMAIN="Obt"',
        f'-DLOCALEDIR="@localedir@"',
        f'-DDATADIR="@datadir@"',
        f'-DCONFIGDIR="@configdir@"',
    ],
    install: true,
)

obt_dep = declare_dependency(
    link_with: libobt,
    include_directories: root_inc,
    dependencies: [glib_dep, xml_dep, x11_dep],
)
meson.override_dependency('obt-3.5', obt_dep)

pkg.generate(
    libobt,
    filebase: 'obt-3.5',
    name: 'Obt',
    description: 'Openbox Toolkit Library',
    version: obt_version,
    subdirs: 'openbox' / obt_version,
    requires: [glib_dep, xml_dep, x11_dep],
)

obt_tests = [
    'bsearch',
    'link',
    'paths',
    'signal',
]

foreach name : obt_tests
    t = executable(
        f'test_@name@',
        files(f'@name@_unittest.c'),
        include_directories: root_inc,
        implicit_include_directories: false,
        dependencies: [obt_dep, glib_dep],
        c_args: [
            '-UG_DISABLE_ASSERT',
            '-DG_LOG_DOMAIN="Obt-Test"',
            f'-DLOCALEDIR="@localedir@"',
            f'-DDATADIR="@datadir@"',
            f'-DCONFIGDIR="@configdir@"',
        ],
        install: false,
    )
    test(
        name,
        t,
        args: ['--tap'],
        protocol: 'tap',
        suite: 'obt',
    )
endforeach
