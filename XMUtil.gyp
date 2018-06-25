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
                                'SubSystem' : '1',
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
            './src/XMUtil.h',
            './src/XMUtil.cpp',
            './src/Model.h',
            './src/Model.cpp',
            './src/ContextInfo.h',
            './src/ContextInfo.cpp',

            './src/Xmile/XMILEGenerator.h',
            './src/Xmile/XMILEGenerator.cpp',

            './src/Vensim/VensimLex.h',
            './src/Vensim/VensimLex.cpp',
            './src/Vensim/VensimParse.h',
            './src/Vensim/VensimParse.cpp',
            './src/Vensim/VensimParseFunctions.h',
            './src/Vensim/VensimParseFunctions.cpp',
            './src/Vensim/VensimView.h',
            './src/Vensim/VensimView.cpp',
#            './src/Vensim/VLex.y',
            './src/Vensim/VYacc.tab.cpp',
            './src/Vensim/VYacc.tab.hpp',
            './src/Vensim/VYacc.y',

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
        'defines' : [
            
        ],
        'include_dirs': [
            'src',
        ]
    }]
}
