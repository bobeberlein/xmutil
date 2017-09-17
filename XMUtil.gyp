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
            './src/Vensim/VYacc.tab.c',
            './src/Vensim/VYacc.tab.h',
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
