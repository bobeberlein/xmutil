#-*- mode: python -*-
{
    'target_defaults': {
        'include_dirs': [
            '<(cwd)/third_party/include',
            '<(cwd)/out/generated',
        ],
        'default_configuration': 'Debug',
        'configurations': {
            'Debug': {
		        'defines': [ '_DEBUG_XMUTIL', ],
                'conditions': [
                    ['OS=="mac"', {
                       'xcode_settings': {
                            'ONLY_ACTIVE_ARCH': 'YES',
                            'GCC_OPTIMIZATION_LEVEL': '0',
                            'SKIP_INSTALL': 'NO',
                        },
                    }],
                    ['OS=="win"', {
                        'msvs_configuration_platform': 'x64',
                        'msvs_settings': {
                            'VCCLCompilerTool': {
                                'Optimization': 0,
                            },
                            'VCLinkerTool': {
                                'AssemblyDebug': '1', #true
                                'GenerateDebugInformation': 'true',  
                            }
                        },
                    }],
                    ['OS=="linux"', {
                    }],
                ],
            },
            'Release': {
		        'defines': [ 'NDEBUG', ],
				# this has to be defined or the release builds will contain asserts on Windows
                'conditions': [
                    ['OS=="mac"', {
                       'xcode_settings': {
                            'ONLY_ACTIVE_ARCH': 'NO',
                            'GCC_OPTIMIZATION_LEVEL': 's',
                            'SKIP_INSTALL': 'YES',
                        },
                    }],
                    ['OS=="win"', {
                        'msvs_configuration_platform': 'x64',
                        'msvs_settings': {
                            'VCCLCompilerTool': {
                                'Optimization': 3,
                            },
                            'VCLinkerTool': {
                                'AssemblyDebug': '0', #false
                                'GenerateDebugInformation': 'false',  
                            }
                        },
                    }],
                ],
            },
        },
        'conditions': [
            ['OS=="win"', {
                'defines': [
                    'WIN32=1'
                ],
                'msvs_settings': {
                    'VCCLCompilerTool': {
                        'ExceptionHandling': '2',  # /EHsc
                        'AdditionalOptions': [
                            '/MP',
                        ],
                    },
                    'VCLinkerTool': {
                        'AdditionalLibraryDirectories': [
                            '<(cwd)/third_party/win/lib',
                            '<(qtdir)/lib'

                        ],
                        'SubSystem' : '1',
                    },
                },
                'include_dirs': [
                    '<(qtdir)/include/QtCore', 
                    '<(qtdir)/include/QtGui',
                    '<(qtdir)/include', 
                    '<(qtdir)/include/QtWidgets', 
                    '-L<(cwd)/third_party/include',
                ],
                'link_settings': {
                    'ldflags': [
                        '-L<(cwd)/third_party/win/lib',
                    ],
                    'libraries': [
                        '<(qtdir)/lib/Qt5Core.lib',
                        '<(qtdir)/lib/Qt5Widgets.lib',
                        '<(qtdir)/lib/Qt5Gui.lib',
                        '<(cwd)/third_party/win/lib/tinyxml2.lib',
                        '<(cwd)/third_party/win/lib/icudt.lib',
                        '<(cwd)/third_party/win/lib/icuin.lib',
                        '<(cwd)/third_party/win/lib/icuio.lib',
                        '<(cwd)/third_party/win/lib/icutu.lib',
                        '<(cwd)/third_party/win/lib/icuuc.lib',
                    ]
                }, 
                'copies': [{
                    'files': [
                        '$(SolutionDir)third_party/win/lib/dlls/icudt59.dll',
                        '$(SolutionDir)third_party/win/lib/dlls/icuin59.dll',
                        '$(SolutionDir)third_party/win/lib/dlls/icudt59.dll',
                        '$(SolutionDir)third_party/win/lib/dlls/icuio59.dll',
                        '$(SolutionDir)third_party/win/lib/dlls/icutu59.dll',
                        '$(SolutionDir)third_party/win/lib/dlls/icuuc59.dll',
                    ],
                    'destination': '$(SolutionDir)$(CONFIGURATION)',
                }]
            }],
            ['OS=="mac"', {
                'defines': [
                    'HAVE_SYS_MALLOC_H'
                ],
                'xcode_settings': {
                    'MACOSX_DEPLOYMENT_TARGET':'10.7',
                    'CLANG_CXX_LIBRARY': 'libc++',
                    'CLANG_CXX_LANGUAGE_STANDARD': 'gnu++11',
                    'SDKROOT': 'macosx',
                    'GCC_OPTIMIZATION_LEVEL': '0',
                    'OTHER_CFLAGS': [
                        '-Wno-tautological-compare',
                        '-Wno-parentheses-equality',
                        '-Werror=return-type',
                        '-Winconsistent-missing-override',
                        '-Wwrite-strings',
                        '-Wno-conversion-null',
                        '-Wno-attributes',
                        '-Wno-unused-variable',
                        '-Wno-reorder',
                        '-Wno-sign-compare',
                        '-Wno-unused-local-typedefs',
                        '-Wno-unused-parameter',
                        '-Wno-unused-function',
                        '-Wno-unknown-pragmas',
                        '-Wno-parentheses',
                        '-Wno-invalid-offsetof', # FIXME - fix
                        '-Wno-switch', # FIXME - fix
                        '-Wno-invalid-source-encoding',
                    ],
                    'OTHER_LDFLAGS': [
                        '-F<(qtdir)/lib',
                        '-F/System/Library/Frameworks',
                        '-L$SDKROOT/usr/lib',
                        '-L/usr/local/lib',
                        '-lz',
                        '-framework CoreFoundation',
                        '-framework ApplicationServices',
                        '-framework Cocoa',
                        '-framework IOKit'
                    ],
                    'LD_RUNPATH_SEARCH_PATHS':'<(qtdir)/lib',
                },
                'include_dirs': [
                    '<(qtdir)/lib/QtCore.framework/Headers',
                    '<(qtdir)/lib/QtWidgets.framework/Headers',
                    '<(qtdir)/lib/QtGui.framework/Headers'
                ],
                'link_settings': {
                    'ldflags': [
                        '-L<(cwd)/third_party/mac/lib',
                    ],
                    'libraries': [
                        '<(qtdir)/lib/QtCore.framework',
                        '<(qtdir)/lib/QtWidgets.framework',
                        '<(qtdir)/lib/QtGui.framework',
                        '<(cwd)/third_party/mac/lib/libboost_filesystem.a',
                        '<(cwd)/third_party/mac/lib/libboost_date_time.a',
                        '<(cwd)/third_party/mac/lib/libboost_chrono.a',
                        '<(cwd)/third_party/mac/lib/libboost_iostreams.a',
                        '<(cwd)/third_party/mac/lib/libboost_system.a',
                        '<(cwd)/third_party/mac/lib/libboost_thread.a',
                        '<(cwd)/third_party/mac/lib/libboost_random.a',
                        '<(cwd)/third_party/mac/lib/libtinyxml2.a',
                        '<(cwd)/third_party/mac/lib/libicudata.a',
                        '<(cwd)/third_party/mac/lib/libicui18n.a',
                        '<(cwd)/third_party/mac/lib/libicuio.a',
                        '<(cwd)/third_party/mac/lib/libicule.a',
                        '<(cwd)/third_party/mac/lib/libiculx.a',
                        '<(cwd)/third_party/mac/lib/libicutu.a',
                        '<(cwd)/third_party/mac/lib/libicuuc.a',
                    ]
                }

            }],
            ['OS=="linux"', {
                'defines':[
                    'linux',
                ],
                'include_dirs': [
                    '<(qtdir)/include/QtCore', 
                    '<(qtdir)/include/QtGui', 
                    '<(qtdir)/include/QtWidgets',
                    './third_party/include',
                    './third_party/linux/include',
                ],
                'link_settings': {
                    'ldflags': [
                        '-L./third_party/linux/lib',
                    ],
                    'libraries': [
                        '<(qtdir)/lib/libQt5Core.so',
                        '<(qtdir)/lib/libQt5Gui.so',
                        '<(qtdir)/lib/libQt5Widgets.so',
                        '<(cwd)/third_party/linux/lib/libboost_filesystem.a',
                        '<(cwd)/third_party/linux/lib/libboost_system.a',
                        '<(cwd)/third_party/linux/lib/libboost_chrono.a',
                        '<(cwd)/third_party/linux/lib/libboost_thread.a',
                        '<(cwd)/third_party/linux/lib/libboost_date_time.a',
                        '<(cwd)/third_party/linux/lib/libboost_random.a',
                        '-lpthread',
                        '-ldl',
                    ],
                },
                'cflags': [
                     '-O3',
                    '-Wall',
                    '-Wextra',
                    '-Wwrite-strings',
                    '-Wno-conversion-null',
                    '-Wno-attributes',
                    '-Wno-unused-variable',
                    '-Wno-reorder',
                    '-Wno-unused-but-set-variable',
                    '-Wno-sign-compare',
                    '-Wno-unused-local-typedefs',
                    '-Wno-unused-parameter',
                    '-Wno-unused-function',
                    '-Wno-unknown-pragmas',
                    '-Wno-parentheses',
                    '-std=c++11',
                    '-Wno-invalid-offsetof', # FIXME - fix
                    '-Wno-switch', # FIXME - fix
                    '-Wno-ignored-qualifiers', # fix
                    '-Wno-deprecated-declarations',
                    '-fPIC',
                ],
                'libraries': [

                ],
            }]
        ],

    },
}
