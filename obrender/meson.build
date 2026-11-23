rr_includedir = get_option('includedir') / 'openbox' / rr_version / 'obrender'

rr_version_conf = configuration_data()
rr_version_conf.set('RR_MAJOR_VERSION', rr_major_version)
rr_version_conf.set('RR_MINOR_VERSION', rr_minor_version)
rr_version_conf.set('RR_MICRO_VERSION', rr_micro_version)

rr_version_h = configure_file(
    input: 'version.h.in',
    output: 'version.h',
    configuration: rr_version_conf,
    install: true,
    install_dir: rr_includedir,
)

rr_public_headers = files(
    'color.h',
    'font.h',
    'geom.h',
    'gradient.h',
    'image.h',
    'instance.h',
    'mask.h',
    'render.h',
    'theme.h',
)

install_headers(
    rr_public_headers,
    install_dir: rr_includedir,
)

obrender_sources = files(
    'button.c',
    'color.c',
    'font.c',
    'gradient.c',
    'image.c',
    'imagecache.c',
    'instance.c',
    'mask.c',
    'render.c',
    'theme.c',
)

obrender_deps = [
    obt_dep,
    x11_dep,
    xft_dep,
    pango_dep,
    pangoxft_dep,
    glib_dep,
    xml_dep,
]
obrender_pc_requires = [
    libobt,
    glib_dep,
    xft_dep,
    pangoxft_dep,
    x11_dep,
]

if imlib2_dep.found()
    obrender_deps += imlib2_dep
    obrender_pc_requires += imlib2_dep
endif
if librsvg_dep.found()
    obrender_deps += librsvg_dep
    obrender_pc_requires += librsvg_dep
endif

libobrender = library(
    'obrender',
    obrender_sources,
    version: rr_so_version,
    soversion: rr_soversion,
    include_directories: root_inc,
    dependencies: obrender_deps,
    c_args: [
        '-DG_LOG_DOMAIN="ObRender"',
        f'-DDEFAULT_THEME="@default_theme@"',
    ],
    install: true,
)

obrender_dep = declare_dependency(
    link_with: libobrender,
    include_directories: root_inc,
    dependencies: [obt_dep, glib_dep, xft_dep, pangoxft_dep, x11_dep],
)
meson.override_dependency('obrender-3.5', obrender_dep)

pkg.generate(
    libobrender,
    filebase: 'obrender-3.5',
    name: 'ObRender',
    description: 'Openbox Render Library',
    version: rr_version,
    subdirs: 'openbox' / rr_version,
    requires: obrender_pc_requires,
)

rendertest = executable(
    'rendertest',
    files('test.c'),
    include_directories: root_inc,
    dependencies: [
        obrender_dep,
        obt_dep,
        glib_dep,
        pango_dep,
        pangoxft_dep,
        xml_dep,
        x11_dep,
    ],
    c_args: [
        '-DG_LOG_DOMAIN="RenderTest"',
    ],
    build_by_default: get_option('tests'),
    install: false,
)
