project(
    'openbox',
    'c',
    version: '3.7',
    license: 'GPL-2.0-or-later',
    meson_version: '>= 1.10.0',
    default_options: [
        'c_std=gnu23,gnu17',
        'warning_level=2',
        'b_ndebug=if-release',
    ],
)

pkg = import('pkgconfig')
i18n = import('i18n')
cc = meson.get_compiler('c')

ob_version = meson.project_version()
ob_bugreport = 'http://bugzilla.icculus.org'
default_theme = get_option('default-theme')

# Library versioning:
# RR_ is for obrender, OBT_ is for obt; keep major/minor in sync, treat the rest separately.
# How to update .so versioning on release:
#   *_micro_version += 1
#   *_interface_age += 1
#   *_binary_age += 1
# If any functions have been added, set *_interface_age to 0.
# If backwards compatibility has been broken, set *_binary_age and *_interface_age to 0.
# If you set anything to 0, also bump *_minor_version.
rr_major_version = 3
rr_minor_version = 6
rr_micro_version = 33
rr_interface_age = 1
rr_binary_age = 1
rr_version = f'@rr_major_version@.@rr_minor_version@'

rr_current = rr_micro_version - rr_interface_age
rr_revision = rr_interface_age
rr_age = rr_binary_age - rr_interface_age
rr_current_minus_age = rr_current - rr_age
rr_soversion = f'@rr_current_minus_age@'
rr_so_version = f'@rr_current_minus_age@.@rr_age@.@rr_revision@'

obt_major_version = 3
obt_minor_version = 6
obt_micro_version = 5
obt_interface_age = 3
obt_binary_age = 3
obt_version = f'@obt_major_version@.@obt_minor_version@'

obt_current = obt_micro_version - obt_interface_age
obt_revision = obt_interface_age
obt_age = obt_binary_age - obt_interface_age
obt_current_minus_age = obt_current - obt_age
obt_soversion = f'@obt_current_minus_age@'
obt_so_version = f'@obt_current_minus_age@.@obt_age@.@obt_revision@'

# Resolved paths for C macros and generated scripts/manpages
prefix = get_option('prefix')
bindir = prefix / get_option('bindir')
libexecdir = prefix / get_option('libexecdir')
datadir = prefix / get_option('datadir')
localedir = prefix / get_option('localedir')
sysconfdir = prefix / get_option('sysconfdir')
configdir = sysconfdir / 'xdg'
rcdir = configdir / 'openbox'

# Installation directories (relative to prefix unless overridden as absolute)
install_docdir = get_option('datadir') / 'doc' / 'openbox'
install_docxbmdir = install_docdir / 'xbm'
install_themedir = get_option('datadir') / 'themes'
install_rcdir = get_option('sysconfdir') / 'xdg' / 'openbox'
install_xsessionsdir = get_option('datadir') / 'xsessions'
install_gnomesessiondir = get_option('datadir') / 'gnome-session' / 'sessions'
install_gnomewmfilesdir = get_option('datadir') / 'gnome' / 'wm-properties'
install_pixmapdir = get_option('datadir') / 'pixmaps'
install_appsdir = get_option('datadir') / 'applications'

# Compiler flags and global defines
# Note: openbox/config.h is Openbox's runtime configuration header, so build
# feature macros are passed via compiler arguments rather than a config.h file.
add_project_arguments(
    cc.get_supported_arguments(['-fno-strict-aliasing', '-Wno-unused-parameter']),
    language: 'c',
)

if get_option('buildtype') == 'debug'
    add_project_arguments('-DDEBUG=1', language: 'c')
elif get_option('buildtype') in ['release', 'minsize']
    add_project_arguments('-DG_DISABLE_ASSERT', language: 'c')
endif

project_name = meson.project_name()
add_project_arguments(
    f'-DPACKAGE_NAME="@project_name@"',
    f'-DPACKAGE_VERSION="@ob_version@"',
    f'-DPACKAGE_BUGREPORT="@ob_bugreport@"',
    language: 'c',
)

# Required dependencies
glib_dep = dependency('glib-2.0', version: '>= 2.28.0')
pango_dep = dependency('pango', version: '>= 1.8.0')
pangoxft_dep = dependency('pangoxft', version: '>= 1.8.0')
xft_dep = dependency('xft')
xml_dep = dependency('libxml-2.0', version: '>= 2.6.0')
x11_dep = dependency('x11')
xau_dep = dependency('xau')
add_project_arguments('-DAUTH=1', language: 'c')

# Optional dependencies
libsn_dep = dependency(
    'libstartup-notification-1.0',
    version: '>= 0.8',
    required: get_option('startup-notification'),
)
if libsn_dep.found()
    add_project_arguments('-DUSE_LIBSN=1', language: 'c')
endif

xcursor_dep = dependency('xcursor', required: get_option('xcursor'))
if xcursor_dep.found()
    add_project_arguments('-DUSE_XCURSOR=1', language: 'c')
endif

imlib2_dep = dependency('imlib2', required: get_option('imlib2'))
if imlib2_dep.found()
    add_project_arguments('-DUSE_IMLIB2=1', language: 'c')
endif

librsvg_dep = dependency('librsvg-2.0', required: get_option('librsvg'))
if librsvg_dep.found()
    add_project_arguments('-DUSE_LIBRSVG=1', language: 'c')
endif

sm_opt = get_option('session-management')
sm_dep = dependency('sm', required: sm_opt)
ice_dep = dependency('ice', required: sm_opt)
use_sm = sm_dep.found() and ice_dep.found()
if use_sm
    add_project_arguments('-DUSE_SM=1', language: 'c')
endif

# X11 extensions
xkb_opt = get_option('xkb')
use_xkb = false
if not xkb_opt.disabled()
    use_xkb = (
        cc.has_header('X11/XKBlib.h', dependencies: x11_dep)
        and cc.has_function('XkbBell', dependencies: x11_dep)
    )
    if xkb_opt.enabled() and not use_xkb
        error(
            'XKB support requested, but X11/XKBlib.h or XkbBell() was not found',
        )
    endif
endif
if use_xkb
    add_project_arguments('-DXKB=1', language: 'c')
endif

xrandr_dep = dependency('xrandr', required: get_option('xrandr'))
if xrandr_dep.found()
    add_project_arguments('-DXRANDR=1', language: 'c')
endif

xshape_opt = get_option('xshape')
xshape_dep = dependency('xext', required: xshape_opt)
use_xshape = false
if xshape_dep.found()
    use_xshape = (
        cc.has_header(
        'X11/extensions/shape.h',
            dependencies: [x11_dep, xshape_dep],
            )
        and cc.has_function(
        'XShapeCombineShape',
            dependencies: [x11_dep, xshape_dep],
            )
    )
    if xshape_opt.enabled() and not use_xshape
        error(
            'XShape support requested, but X11/extensions/shape.h or XShapeCombineShape() was not found',
        )
    endif
endif
if use_xshape
    add_project_arguments('-DSHAPE=1', language: 'c')
endif

xinerama_dep = dependency('xinerama', required: get_option('xinerama'))
if xinerama_dep.found()
    add_project_arguments('-DXINERAMA=1', language: 'c')
endif

xsync_opt = get_option('xsync')
xsync_dep = dependency('xext', required: xsync_opt)
use_xsync = false
if xsync_dep.found()
    use_xsync = (
        cc.has_header(
        'X11/extensions/sync.h',
            dependencies: [x11_dep, xsync_dep],
            )
        and cc.has_function(
        'XSyncInitialize',
            dependencies: [x11_dep, xsync_dep],
            )
    )
    if xsync_opt.enabled() and not use_xsync
        error(
            'XSync support requested, but X11/extensions/sync.h or XSyncInitialize() was not found',
        )
    endif
endif
if use_xsync
    add_project_arguments('-DSYNC=1', language: 'c')
endif

nls_opt = get_option('nls')
intl_dep = dependency('intl', required: nls_opt)
msgfmt_prog = find_program('msgfmt', required: nls_opt)
use_nls = intl_dep.found() and msgfmt_prog.found()
if use_nls
    add_project_arguments('-DENABLE_NLS=1', language: 'c')
endif

# Shared configuration data for scripts, desktop files, and manpages
script_conf = configuration_data()
script_conf.set('version', ob_version)
script_conf.set('configdir', configdir)
script_conf.set('rcdir', rcdir)
script_conf.set('libexecdir', libexecdir)
script_conf.set('bindir', bindir)

# Generate top-level version.h
version_conf = configuration_data()
version_conf.set('OB_VERSION', ob_version)
configure_file(
    input: 'version.h.in',
    output: 'version.h',
    configuration: version_conf,
)

root_inc = include_directories('.')

install_data(
    'COMPLIANCE',
    'README',
    'AUTHORS',
    'CHANGELOG',
    'COPYING',
    install_dir: install_docdir,
)

subdir('obt')
subdir('obrender')
subdir('openbox')
subdir('tools')
subdir('data')
subdir('doc')
subdir('themes')
if use_nls
    subdir('po')
endif
if get_option('tests')
    subdir('tests')
endif

summary(
    {
        'Prefix': prefix,
        'Binaries': bindir,
        'Libexec': libexecdir,
        'Config dir': configdir,
        'Data dir': datadir,
        'Locale dir': localedir,
        'Default theme': default_theme,
    },
    section: 'Directories',
)

summary(
    {
        'Startup Notification': libsn_dep.found(),
        'X Cursor': xcursor_dep.found(),
        'Session Management': use_sm,
        'Imlib2': imlib2_dep.found(),
        'SVG (librsvg)': librsvg_dep.found(),
        'XKB': use_xkb,
        'XRandR': xrandr_dep.found(),
        'XShape': use_xshape,
        'Xinerama': xinerama_dep.found(),
        'XSync': use_xsync,
        'NLS (gettext)': use_nls,
        'Optional tests': get_option('tests'),
    },
    section: 'Features',
    bool_yn: true,
)
