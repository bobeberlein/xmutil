#-*- mode: python -*-
{
    'variables': {
        'with_ui%': '0',
    },
    'target_defaults': {
        'include_dirs': [
            '<(cwd)/third_party/include',
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
                        ],
                        'SubSystem' : '1',
                    },
                },
                'include_dirs': [
                    '-L<(cwd)/third_party/include',
                ],
                'link_settings': {
                    'ldflags': [
                        '-L<(cwd)/third_party/win/lib',
                    ],
                    'libraries': [
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
                        '$(SolutionDir)third_party/win/lib/dlls/icudt67.dll',
                        '$(SolutionDir)third_party/win/lib/dlls/icuin67.dll',
                        '$(SolutionDir)third_party/win/lib/dlls/icudt67.dll',
                        '$(SolutionDir)third_party/win/lib/dlls/icuio67.dll',
                        '$(SolutionDir)third_party/win/lib/dlls/icutu67.dll',
                        '$(SolutionDir)third_party/win/lib/dlls/icuuc67.dll',
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
                        '-Wno-invalid-source-encoding',
                    ],
                    'OTHER_LDFLAGS': [

                    ],
                },
                'link_settings': {
                    'ldflags': [
                        '-L<(cwd)/third_party/mac/lib',
                    ],
                    'libraries': [
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
                    './third_party/include',
                    './third_party/linux/include',
                ],
                'link_settings': {
                    'ldflags': [
                        '-L./third_party/linux/lib',
                    ],
                    'libraries': [
                        '-lboost_chrono',
                        '-lboost_date_time',
                        '-lboost_filesystem',
                        '-lboost_iostreams',
                        '-lboost_random',
                        '-lboost_system',
                        '-lboost_thread',
                        '-ldl',
                        '-licuuc',
                        '-lpthread',
                        '-ltinyxml2',
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
                    '-fPIC',
                ],
                'libraries': [

                ],
            }]
        ],
    },
}
