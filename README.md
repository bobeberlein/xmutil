# Building the XMUtil XMILE utilities

Windows is required to build and run XMUtil. These instructions were developed on Windows 10.

Microsoft Visual Studio is required for the C++ compiler. These instructions use Visual Studio Community 2015.

Use the "VS2015 x86 Native Tools Command Prompt" for all commands. `XMUtil` is built as 32-bit software.

## Install dependencies

### Boost C++ Libraries

Download and extract the latest version of [Boost](http://www.boost.org/users/download/).

Follow the instructions in [Boost Getting Started on Windows](http://www.boost.org/doc/libs/1_63_0/more/getting_started/windows.html).
~~~
cd boost
bootstrap
.\b2
~~~

Note the include and library paths emitted at the end of the build process.

### ICU - International Components for Unicode

Download [ICU4C](http://site.icu-project.org/download/57#TOC-ICU4C-Download)version 57.1 binaries for Win32. This is the last version with 32-bit binaries.

### Win flex-bison - Flex (the fast lexical analyser) and Bison (GNU parser generator)

Download and extract [Win flex-bison](http://sourceforge.net/projects/winflexbison/).

### TinyXML - XML Parser

Download and extract [TinyXML-2](https://github.com/leethomason/tinyxml2).

## Build XMUtil

Set dependency directories in the `setup.bat` file in the `third_party` directory. You may also need to change the Boost library path according to the output of the Boost build process.

Open the "VS2015 x86 Native Tools Command Prompt" as administrator and run `setup.bat`.

Open the `XMUtil.vcxproj` project in Visual Studio. The project settings will be upgraded if necessary.

Choose Build Solution from the Build menu. Bison will generate code. The build result is `XMUtil.exe` in the Debug directory.

## Convert a Vensim model to XMILE

Switch to the directory where your `.mdl` file is located. Run XMUtil on the `.mdl` file to generate an `.xmile` file in the same directory. Press any key after the converter runs to end the process.
~~~
{/path/to/xmutil/debug}/XMUtil.exe {mdl-file}
~~~
