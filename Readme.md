FreeSpace2 *S*ource *C*ode *P*roject
==
[![Travis-CI Build Status](https://travis-ci.org/scp-fs2open/fs2open.github.com.svg?branch=master)](https://travis-ci.org/scp-fs2open/fs2open.github.com)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/scp-fs2open/fs2open.github.com?branch=master&svg=true)](https://ci.appveyor.com/project/SirKnightly/fs2open-github-com/branch/master)
[![Coverity](https://img.shields.io/coverity/scan/870.svg)](https://scan.coverity.com/projects/870)

Building
--
Before you do anything, make sure you have updated your git submodules, either by running git submodule update --init --recursive or by cloning the repository with the --recursive flag.<br/>
<br/>
For building you will need [CMake](http://www.cmake.org/cmake/resources/software.html). Version 3.4 is required. Once you have installed CMake you should create a build directory where the project/make files should be created, **do not create them inside the source tree!**<br>
<br>
On Windows you can use the `cmake-gui` executable to get a nice GUI, on Unix just use `cmake`. Use the newly created directory as build directory (change to this directory on Unix) and run cmake. On Windows this can be done by clicking the `Configure` button. Choose your compiler version and hit `Generate` when configuring is done. <br>
Depending on the makefile generator used build the `all` or `ALL_BUILD` target to compile the project.<br>

### Available variables to control the build
* `FSO_BUILD_WXFRED2`: Build the wxfred2 project, requires wxWidgets
* `FSO_FREESPACE_PATH`: Sets the path of your FreeSpace install, can be used to automatically run the generated executables with the correct working directory, this is an optional variable. By default this will use the value of the `FS2PATH` environment variable.
* `FSO_BUILD_TOOLS`: Build some tools related to FSO
* `CMAKE_BUILD_TYPE`: Sets the binary build type between `Debug` and `Release` (default)

#### Windows only variables
* `FSO_BUILD_FRED2`: Build FRED2, requires a Visual Studio version that ships with MFC
* `FSO_USE_SPEECH`: Build a binary with text-to-speech support
* `FSO_USE_VOICEREC`: Build a binary with voice recognition support

#### Advanced variables
You should only use these variables if you know what you're doing
* `FSO_CMAKE_DEBUG`: Print CMake debugging informations
* `FSO_BUILD_INCLUDED_LIBS`: Build libraries from the included source
* `FSO_USE_OPENALSOFT`: Use [OpenALSoft](http://kcat.strangesoft.net/openal.html) for OpenAL support
* `FSO_USE_LUAJIT`: Use [luajit](http://luajit.org/) as a replacement for lua
* `FSO_DEVELOPMENT_MODE`: Toggles some development behavior, only use if you really need it.
* `FSO_RUN_ARGUMENTS`: If you run an executable from within the project, these arguments will also be passed to the executable
* `FSO_BUILD_POSTFIX`: Sets a postfix to be added to the executable name, may be useful for release candidates.

#### Visual Studio specific variables
* `MSVC_USE_RUNTIME_DLL`: Use the DLL version of the runtime, experimental and not well tested.
* `MSVC_SIMD_INSTRUCTIONS`: The instruction set the executables will be optimized for.

### Generating an installation package
CMake can automatically generate installation packages of the project. To do this you will just need to run the `package` target of the build system and the package will be placed inside your build directory. On Windows you will have to install [NSIS](http://nsis.sourceforge.net/Main_Page) to use this.
