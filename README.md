# Building the XMUtil XMILE utilities on OS X

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
./configure.sh --use-xcode
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

### Boost C++ Libraries

Download and extract the latest version of [Boost](http://www.boost.org/users/download/).

Follow the instructions in [Boost Getting Started on Windows](http://www.boost.org/doc/libs/1_64_0/more/getting_started/windows.html).

~~~
cd boost
bootstrap.bat
bjam -j4 toolset=msvc-14.1 architecture=x86 address-model=64 --build-type=complete --with-filesystem --with-iostreams --with-system stage
~~~

The products end up in stage\lib
The include directory is \boost

Copy the headers into \third_party\include\boost
Copy the libs into \third_party\win\lib

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
./configure.bash --use-msvs
~~~

Open the `XMUtil.vcxproj` project in Visual Studio. The project settings will be upgraded if necessary.

Choose Build Solution from the Build menu. The build result is `XMUtil.exe` in the Debug directory.

## Convert a Vensim model to XMILE

Switch to the directory where your `.mdl` file is located. Run XMUtil on the `.mdl` file to generate an `.xmile` file in the same directory. Press any key after the converter runs to end the process.
~~~
{/path/to/xmutil/debug}/XMUtil.exe {mdl-file}
~~~
