qtFRED
=======

For a long time, the FReespace EDitor was only available to Windows users.

But no more!

Using the latest and greatest of software development arsenal,
a glimpse of hope appeared to users of other operating systems.

**THIS TOOL IS IN ALPHA AND NOT TO BE CONSIDERED STABLE**

Build Instructions
------------------

Additional Dependencies:

- Qt >= 6.8

qtFRED also needs all the dependencies that standard FSO needs.

### OS-specific notes
#### Ubuntu
`sudo aptitude install qttools5-dev libqt5opengl5-dev`
#### Windows
Download the Qt6 SDK available on their website.


### CMake configuration
Configure the project as usual with CMake but make sure that `FSO_BUILD_QTFRED` is enabled in the CMake configuration. The
variable `QT6_INSTALL_ROOT` is available for letting CMake know where it should look for the QT6 installation. This is
especially important on Windows where the standard CMake `find_package` calls will likely fail.

You need to set `QT6_INSTALL_ROOT` to the actual installation folder of a specific version. If you installed Qt to a path
like `/path/to/Qt` then you need to set `QT6_INSTALL_ROOT` to `/path/to/Qt/<version>/<variant>` where `<version>` might be
something like `6.8.3` and `<variant>` could be `msvc2017_64`. You need to make sure that the variant matches the compiler
and bitness you are currently compiling FSO/qtFRED for. In the example above it was Visual Studio 2017 and FSO was built
in 64-bit mode.

Our CMake setup will automatically handle copying the relevant Qt DLLs to the build folder and if you use the CMake install
functionality the DLLs will also be copied to the correct paths in the destination folder.

Directories
-----------

- **resources**: Contains the Qt resources needed for qtFRED
    - **images**: The image files used by qtFRED. If you add a new file make sure that it is also added to resources.qrc
    - **win**: Contains Windows specific resources. Do not edit unless absolutely necessary.
- **src**: Contains the main source files of qtFRED
    - **mission**: Contains the mission management code. This should be UI framework agnostic (see Coding style for more information)
        - **dialogs**: Contains the dialog models used for keeping the mission editor logic separate from the UI code
    - **ui**: Contains the Qt specific code for qtFRED. `FredView` is the main UI class of qtFRED.
        - **dialogs**: The `QDialog` subclasses for the individual editor dialogs are located here
        - **util**: Used for organizing various utility classes and functions
        - **widgets**: qtFRED specific Qt widgets are kept here
- **ui**: Contains the Qt Designer files used for creating the UI of qtFRED

Coding style
------------
qtFRED has some special coding style requirements that should be observed when writing code for qtFRED.

For a more detailed, practical breakdown of the design preferences for new and updated code — dialog interaction
patterns, signal/slot conventions, dirty-state handling, and help-doc requirements — see the
[QtFRED Design Guide](DESIGN_GUIDE.md).
