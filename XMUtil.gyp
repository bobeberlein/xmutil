#-*- mode: python -*-
{
    'includes': [
        './build/common.gypi',
    ],
    'targets': [{
        'target_name': 'XMUtil',
        'type': 'executable',
        'product_name': 'XMUtil',
        'mac_bundle': 0,
        'conditions': [
            ['with_ui==1', {
                'mac_bundle': 1,
                'sources': [
                    './src/UI/Main_Window.h',
                    './src/UI/Main_Window.cpp',
                    './src/UI/resources/main_window.ui',
                    './out/generated/moc_Main_Window.cpp',
                    './out/generated/ui_main_window.h',
                ],
                'defines': [ 'WITH_UI' ],
                'include_dirs': [
                    '<(cwd)/out/generated',
                ],
                'conditions': [
                    ['OS=="win"', {
                        'msvs_settings': {
                            'VCLinkerTool': {
                                'AdditionalLibraryDirectories': [
                                    '<(qtdir)/lib'
                                ],
                                'SubSystem' : '2',
                            },
                        },
                        'include_dirs': [
                            '<(qtdir)/include/QtCore', 
                            '<(qtdir)/include/QtGui',
                            '<(qtdir)/include', 
                            '<(qtdir)/include/QtWidgets', 
                        ],
                        'link_settings': {
                            'libraries': [
                                'shell32.lib',
                                '<(qtdir)/lib/qtmain.lib',
                                '<(qtdir)/lib/Qt5Core.lib',
                                '<(qtdir)/lib/Qt5Widgets.lib',
                                '<(qtdir)/lib/Qt5Gui.lib',
                            ]
                        }
                    }],
                    ['OS=="mac"', {
                        'xcode_settings': {
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
                            'libraries': [
                                '<(qtdir)/lib/QtCore.framework',
                                '<(qtdir)/lib/QtWidgets.framework',
                                '<(qtdir)/lib/QtGui.framework'
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
                            '<(qtdir)/include/QtWidgets'
                        ],
                        'link_settings': {
                            'libraries': [
                                '<(qtdir)/lib/libQt5Core.so',
                                '<(qtdir)/lib/libQt5Gui.so',
                                '<(qtdir)/lib/libQt5Widgets.so',
                                '-lpthread',
                                '-ldl',
                            ],
                        }
                    }]
                ],

            }]
        ],
        'sources': [
            './src/Main.cpp',
            # CLI-only: the wasm and library entry points never derive an
            # output filename, so this stays out of common_sources.
            './src/OutputPath.h',
            './src/OutputPath.cpp',
            '<@(common_sources)',
            '<@(platform_sources)',
        ],
        'defines' : [

        ],
        'include_dirs': [
            'src',
        ],
    }, {
        # The custom test harness links the same engine sources as XMUtil but
        # swaps Main.cpp for the test entry point.
        'target_name': 'xmutil_test',
        'type': 'executable',
        'product_name': 'xmutil_test',
        'mac_bundle': 0,
        'sources': [
            '<@(common_sources)',
            '<@(platform_sources)',
            # Main.cpp is replaced by the harness, so the CLI-only sources it
            # depends on have to be listed here too.
            './src/OutputPath.h',
            './src/OutputPath.cpp',
            './test/TestHarness.cpp',
            './test/UnicodeTest.cpp',
            './test/OutputPathTest.cpp',
            './test/mdl/ModelComparator.cpp',
            './test/mdl/RoundTrip.cpp',
            './test/mdl/MDLGeneratorTest.cpp',
            './test/mdl/MDLFormatTest.cpp',
            './test/mdl/WalkerTest.cpp',
            './test/mdl/ModelComparatorTest.cpp',
            './test/mdl/EquationRoundTripTest.cpp',
            './test/mdl/ControlRoundTripTest.cpp',
            './test/mdl/SketchRoundTripTest.cpp',
            './test/mdl/MacroRoundTripTest.cpp',
            './test/mdl/WriterBugRegressionTest.cpp',
            './test/mdl/GroupNestingTest.cpp',
            './test/mdl/CorpusRoundTripTest.cpp',
            './test/mdl/CEntryTest.cpp',
            './test/mdl/DynamoToMdlTest.cpp',
            './test/mdl/CrossViewFlowTest.cpp',
            './test/xmile/RoundTrip.cpp',
            './test/xmile/BasicSmokeTest.cpp',
            './test/xmile/XmileFunctionsTest.cpp',
            './test/xmile/EquationParseTest.cpp',
            './test/xmile/AuxRoundTripTest.cpp',
            './test/xmile/SimSpecsRoundTripTest.cpp',
            './test/xmile/StockFlowRoundTripTest.cpp',
            './test/xmile/ArrayRoundTripTest.cpp',
            './test/xmile/LookupRoundTripTest.cpp',
            './test/xmile/FreeTextSanitizeRoundTripTest.cpp',
            './test/xmile/ModelUnitsRoundTripTest.cpp',
            './test/xmile/ExtrapolateRoundTripTest.cpp',
            './test/xmile/ViewRoundTripTest.cpp',
            './test/xmile/GroupRoundTripTest.cpp',
            './test/xmile/SingleModelNormalizationTest.cpp',
            './test/xmile/CorpusRoundTripTest.cpp',
            './test/xmile/XmileCorpusTest.cpp',
            './test/xmile/FixpointTest.cpp',
            './test/xmile/ErrorRejectTest.cpp',
            './test/xmile/BuiltinNameCollisionTest.cpp',
            './test/xmile/PiKeywordTest.cpp',
            './test/xmile/ReaderLifetimeTest.cpp',
            './test/xmile/DiagnosticsTest.cpp',
            './test/xmile/CEntryTest.cpp',
            './test/xmile/MdlXmileByteIdentityTest.cpp',
        ],
        'defines' : [
            # <(cwd) is the repo root: configure.sh passes -Dcwd=`pwd` and the
            # same variable resolves third_party paths elsewhere in this build.
            # The corpus test joins it with a relative fixture path so it can
            # locate on-disk fixtures regardless of the test binary's CWD.
            'XMUTIL_SRC_ROOT="<(cwd)"',
        ],
        'include_dirs': [
            'src',
        ],
    }],
    'conditions': [
        # The wasm target drives emcc through a shell script, so it only exists
        # on the platforms that can run it -- keeping it out of the Visual
        # Studio solution leaves the CLI (and its tests) as the only projects.
        ['OS!="win"', {
            'targets': [{
                'target_name': 'XMUtil_wasm',
                'type': 'none',
                'dependencies': [],
                'sources': [
                    './src/emscripten_wrapper.cpp',
                    '<@(common_sources)',
                ],
                'actions': [{
                    'action_name': 'build_wasm',
                    'inputs': [
                        '<@(_sources)',
                        '<(cwd)/build_wasm_action.sh',
                    ],
                    'outputs': [
                        '<(PRODUCT_DIR)/xmutil.js',
                        '<(PRODUCT_DIR)/xmutil.wasm',
                    ],
                    'action': [
                        'bash',
                        '<(cwd)/build_wasm_action.sh',
                        '<(PRODUCT_DIR)',
                        '<@(_sources)',
                    ],
                }],
            }],
        }],
    ],
    'variables': {
        'conditions': [
            # Linux links the system tinyxml2 and macOS has a prebuilt static
            # lib, but nothing provisions a tinyxml2.lib for Windows, so there
            # the amalgamated source next to the header is compiled in.
            ['OS=="win"', {
                'platform_sources': [
                    './third_party/include/tinyxml2.cpp',
                ],
            }, {
                'platform_sources': [],
            }],
        ],
        'common_sources': [
            './src/XMUtil.h',
            './src/XMUtil.cpp',
            './src/Log.h',
            './src/Log.cpp',
            './src/Unicode.h',
            './src/Unicode.cpp',
            './src/Model.h',
            './src/Model.cpp',
            './src/ContextInfo.h',
            './src/ContextInfo.cpp',

            './src/Xmile/XMILEGenerator.h',
            './src/Xmile/XMILEGenerator.cpp',
            './src/Xmile/XmileReader.h',
            './src/Xmile/XmileReader.cpp',
            './src/Xmile/XmileView.h',
            './src/Xmile/XmileView.cpp',
            './src/Xmile/XmileFunctions.h',
            './src/Xmile/XmileFunctions.cpp',
            './src/Xmile/XmileEqLex.h',
            './src/Xmile/XmileEqLex.cpp',
            './src/Xmile/XmileEqYacc.tab.cpp',
            './src/Xmile/XmileEqYacc.tab.hpp',
            './src/Xmile/XmileEqYacc.y',
            './src/Xmile/XmileParseFunctions.h',
            './src/Xmile/XmileParseFunctions.cpp',

            './src/Mdl/MDLFormat.h',
            './src/Mdl/MDLFormat.cpp',
            './src/Mdl/MDLGenerator.h',
            './src/Mdl/MDLGenerator.cpp',

            './src/Vensim/VensimLex.h',
            './src/Vensim/VensimLex.cpp',
            './src/Vensim/VensimParse.h',
            './src/Vensim/VensimParse.cpp',
            './src/Vensim/VensimParseFunctions.h',
            './src/Vensim/VensimParseFunctions.cpp',
            './src/Vensim/VensimView.h',
            './src/Vensim/VensimView.cpp',
            './src/Vensim/VYacc.tab.cpp',
            './src/Vensim/VYacc.tab.hpp',
            './src/Vensim/VYacc.y',

            './src/Dynamo/DynamoLex.h',
            './src/Dynamo/DynamoLex.cpp',
            './src/Dynamo/DynamoFunction.h',
            './src/Dynamo/DynamoFunction.cpp',
            './src/Dynamo/DynamoParse.h',
            './src/Dynamo/DynamoParse.cpp',
            './src/Dynamo/DynamoParseFunctions.h',
            './src/Dynamo/DynamoParseFunctions.cpp',
            './src/Dynamo/DYacc.tab.cpp',
            './src/Dynamo/DYacc.tab.hpp',
            './src/Dynamo/DYacc.y',

            './src/Symbol/Variable.h',
            './src/Symbol/Variable.cpp',
            './src/Symbol/Units.h',
            './src/Symbol/Units.cpp',
            './src/Symbol/UnitExpression.h',
            './src/Symbol/UnitExpression.cpp',
            './src/Symbol/SymbolTableBase.h',
            './src/Symbol/SymbolTableBase.cpp',
            './src/Symbol/SymbolNameSpace.h',
            './src/Symbol/SymbolNameSpace.cpp',
            './src/Symbol/SymbolListList.h',
            './src/Symbol/SymbolListList.cpp',
            './src/Symbol/SymbolList.h',
            './src/Symbol/SymbolList.cpp',
            './src/Symbol/Symbol.h',
            './src/Symbol/Symbol.cpp',
            './src/Symbol/Parse.h',
            './src/Symbol/NotUsed_SymAllocList.h',
            './src/Symbol/NotUsed_SymAllocList.cpp',
            './src/Symbol/LeftHandSide.h',
            './src/Symbol/LeftHandSide.cpp',
            './src/Symbol/ExpressionList.h',
            './src/Symbol/ExpressionList.cpp',
            './src/Symbol/Expression.h',
            './src/Symbol/Expression.cpp',
            './src/Symbol/Equation.h',
            './src/Symbol/Equation.cpp',

            './src/Function/Function.h',
            './src/Function/Function.cpp',
            './src/Function/Level.h',
            './src/Function/Level.cpp',
            './src/Function/State.h',
            './src/Function/State.cpp',
            './src/Function/TableFunction.h',
            './src/Function/TableFunction.cpp',
        ],
    },
}
