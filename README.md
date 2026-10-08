# Building the XMUtil XMILE utilities on OS X

## Download and install Qt 5.11.0
You can find this verion here:
https://download.qt.io/official_releases/qt/5.11/5.11.0/

On OS X install in $HOME\Qt5.11.0 (default install directory)
On Windows install in C:/Qt/x64/Qt5.11.0 (on Windows you will have to put the Qt DLLs into the build directory to make the UI open)

## Build third_party
Open Terminal

~~~
cd third_party
./all_mac.sh
~~~

## Build with CMake

XMUtil builds with CMake (3.21+ for the presets) and Ninja. From the repository root:

~~~
cmake --preset debug          # configure into out/Debug (use "release" for out/Release)
cmake --build --preset debug  # builds XMUtil and xmutil_test
ctest --preset debug          # runs the unit tests and the CLI round-trip scripts
~~~

The CLI lands at `out/Debug/XMUtil`. macOS builds run on macOS 10.15 or later (11.0 on Apple silicon).

### Intel Mac build (macOS 10.15+)

~~~
cmake --preset mac-intel
cmake --build --preset mac-intel   # out/mac-intel/XMUtil
~~~

This links whatever is in `third_party/mac/lib`, which must then hold x86_64 libraries built with `-mmacosx-version-min=10.15` or lower (the default `OSX_VERSION` in `third_party/build/icu_mac.bash`). To cross-build on Apple silicon without replacing the arm64 libraries, keep the x86_64 ones in a separate directory and add `-DXMUTIL_MAC_LIB_DIR=/path/to/x86_64/lib` to the configure step. The tests run under Rosetta (`arch -x86_64 out/mac-intel/xmutil_test`).

To build the Qt UI, enable `XMUTIL_WITH_UI` and point CMake at the Qt install:

~~~
cmake --preset debug -DXMUTIL_WITH_UI=ON -DCMAKE_PREFIX_PATH=$HOME/Qt5.15.4/5.15.4/clang_arm64
~~~

## Generate Xcode Project (optional)

~~~
cmake --preset xcode
open out/xcode/XMUtil.xcodeproj
~~~

To run the `xmutil` command line app:
~~~
{/path/to/xmutil}/xmutil {mdl-file}
~~~

To make this command more convenient, add an alias to your `~/.bash_profile`.
~~~
alias xmutil='{/path/to/xmutil}/xmutil'
~~~

# Building the XMUtil XMILE utilities on Windows

These instructions were developed on Windows 10.

Microsoft Visual Studio 2017 is required for the C++ compiler. These instructions use Visual Studio Community 2017.

Use the "VS2017 x64 Native Tools Command Prompt" for all commands. `XMUtil` is built as 64-bit software.

## Install MSYS

Download [MSYS](http://www.mingw.org/wiki/MSYS)
In MinGW Installation Manager make sure you've installed:
    msys-base msys-bash msys-core msys-coreutils

## Install dependencies

### ICU - International Components for Unicode

Download [ICU4C](http://site.icu-project.org/download/59#TOC-ICU4C-Download)version 59.1 binaries for Win64.

Copy the \include folder contents (\unicode) into \third_party\include
Copy the \lib64 folder contents into \third_party\win\lib
Copy the \dll files from /bin into \third_party\win\lib\dlls

### Win flex-bison - Flex (the fast lexical analyser) and Bison (GNU parser generator)

Download and extract [Win flex-bison](http://sourceforge.net/projects/winflexbison/).

### TinyXML - XML Parser

Download and extract [TinyXML-2](https://github.com/leethomason/tinyxml2).

Compile TinyXML-2:

~~~
MSBuild.exe tinyxml2/tinyxml2.sln /t:tinyxml2:rebuild /property:Configuration=Release-Lib
~~~

Move lib into third_party/win/lib
tinyxml2/bin/tinyxml2/x64-Release-Lib/tinyxml2.lib /third_party/win/lib

Move header to include director
tinyxml2.h to /third_party/include

- OR -

From environment.bat run

~~~
third_party/build/tinyxml_win.bash
~~~

## Build XMUtil

Once you have the /third_party directory set up, generate the Visual Studio solution with CMake (CMake ships with Visual Studio's C++ workload):

~~~
cmake --preset msvs
cmake --build out/msvs --config Debug
~~~

Or open `out/msvs/XMUtil.sln` in Visual Studio, or open the repository folder directly (Visual Studio reads `CMakePresets.json`). The build result is `XMUtil.exe` in `out/msvs/Debug`. A post-build step copies the ICU DLLs from `third_party/win/lib/dlls` next to it. Add `-DXMUTIL_WITH_UI=ON -DCMAKE_PREFIX_PATH=C:/Qt/...` to the configure step to build the UI.

## Convert a Vensim model to XMILE

Switch to the directory where your `.mdl` file is located. Run XMUtil on the `.mdl` file to generate an `.xmile` file in the same directory. Press any key after the converter runs to end the process.
~~~
{/path/to/xmutil/debug}/XMUtil.exe {mdl-file}
~~~

# WebAssembly Build

XMUtil can be compiled to WebAssembly for use in web browsers.

## Prerequisites

- Emscripten SDK (activate it with `source <emsdk>/emsdk_env.sh`)
- The tinyxml2 source checkout at `third_party/build/tinyxml2` (created by `third_party/all_mac.sh`)

## Building for WebAssembly

```bash
emcmake cmake --preset wasm
cmake --build --preset wasm
```

The build outputs will be created in `out/wasm/`:
- `xmutil.js` - JavaScript loader
- `xmutil.wasm` - WebAssembly binary

## WebAssembly API

The WASM module exposes a single function:

```javascript
convertMdlToXmile(mdlContent, isCompact=false, isLongName=1, isAsSectors=false)
```

**Parameters:**
- `mdlContent` (string): The MDL model content to convert
- `isCompact` (boolean, optional): Whether to generate compact XMILE output (default: false)
- `isLongName` (number, optional): Use long names (1) or short names (0) (default: 1)
- `isAsSectors` (boolean, optional): Generate as sectors (default: false)

**Returns:** XMILE XML content as a string, or empty string on error

## Usage Example

```html
<!DOCTYPE html>
<html>
<head>
    <title>XMUtil WASM Example</title>
    <script src="out/wasm/xmutil.js"></script>
</head>
<body>
    <script>
        createXMUtilModule().then(function(Module) {
            const mdl = `{UTF-8}
Population = 100
~  Person
~  |`;

            const xmile = Module.convertMdlToXmile(mdl);
            console.log(xmile);
        });
    </script>
</body>
</html>
```

## Testing WebAssembly Build

A test HTML file is provided at `test_wasm.html`:

```bash
python3 -m http.server 8000
```

Then open http://localhost:8000/test_wasm.html in your browser.

## Implementation Notes

The WASM build uses a simplified Unicode implementation (`src/Unicode_stub.cpp`) that provides basic ASCII lowercase conversion instead of full ICU Unicode support. This avoids the complexity of linking ICU libraries in the WASM build.
