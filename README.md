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

## Generate Xcode Project

Open Terminal

~~~
cd ..
./configure.sh --use-xcode --with-ui
open XMUtil.xcodeproj
~~~

## Build the project

Run the Xcode project. It will produce a naked executable `xmutil` (no app bundle). To find where it is located, look in Xcode > Preferences > Locations. Click the arrow to the right of the pathname under Derived Data. This will show the folder in the Finder.

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

Once you have the /third_party directory setup you are ready to generate the Visual Studio project and build

To generate the Visual Studio project run

Run:

~~~
environment.bat
~~~

In the msys terminal run

~~~
./configure.bash --use-msvs --with-ui
~~~

Open the `XMUtil.vcxproj` project in Visual Studio. The project settings will be upgraded if necessary.

Choose Build Solution from the Build menu. The build result is `XMUtil.exe` in the Debug directory.

## Convert a Vensim model to XMILE

Switch to the directory where your `.mdl` file is located. Run XMUtil on the `.mdl` file to generate an `.xmile` file in the same directory. Press any key after the converter runs to end the process.
~~~
{/path/to/xmutil/debug}/XMUtil.exe {mdl-file}
~~~

# WebAssembly Build

XMUtil can be compiled to WebAssembly for use in web browsers.

## Prerequisites

- Emscripten SDK installed at `~/tools/emsdk`

## Building for WebAssembly

```bash
./configure.sh --use-wasm
make XMUtil_wasm BUILDTYPE=Debug|Release
```

The build outputs will be created in `out/BUILDTYPE/`:
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
    <script src="out/Debug/xmutil.js"></script>
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
