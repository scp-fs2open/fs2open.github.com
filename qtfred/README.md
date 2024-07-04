qtFRED
=======

For a long time, the FReespace EDitor was only available to Windows users.

But no more!

Using the latest and greatest of software development arsenal,
a glimpse of hope appeared to users of other operating systems.

**THIS TOOL IS STILL UNDER HEAVY DEVELOPMENT AND IS _NOT_ TO BE CONSIDERED STABLE.**

Build Instructions
------------------

Additional Dependencies:

- Qt >= 5.7

qtFRED also needs all the dependencies that standard FSO needs.

### OS-specific notes
#### Ubuntu
`sudo aptitude install qttools5-dev libqt5opengl5-dev`
#### Windows
Download the Qt5 SDK available on their website.


### CMake configuration
Configure the project as usual with CMake but make sure that `FSO_BUILD_QTFRED` is enabled in the CMake configuration. The
variable `QT5_INSTALL_ROOT` is available for letting CMake know where it should look for the QT5 installation. This is
especially important on Windows where the standard CMake `find_package` calls will likely fail.

You need to set `QT5_INSTALL_ROOT` to the actual installation folder of a specific version. If you installed Qt to a path
like `/path/to/Qt` then you need to set `QT5_INSTALL_ROOT` to `/path/to/Qt/<version>/<variant>` where `<version>` might be
something like `5.9.2` and `<variant>` could be `msvc2017_64`. You need to make sure that the variant matches the compiler
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

### Front end - Back end separation
The original FRED freely mixed MFC (the UI framework it used) and mission editor logic code. This created a hard to maintain
code mess which used tight coupling wherever necessary.

To avoid this issue qtFRED tries to follow a front end - back end separation coding style where the code that handles the UI is
separate from the editor logic code. The original FRED also used global dialog and window instances for handling callbacks.
Since Qt has its own signal/slot functionality qtFRED uses that for handling event notification.

The logic of the mission editor has been extracted into multiple classes which do not use Qt functionality (except signals and
slots). These are `Editor` and `EditorViewport`.

- `Editor` is an instance of a mission editor and keeps all state related to that. It is similar to the `management.cpp` file
of the orignal FRED which used a lot of global variables for state keeping in the original FRED. The names of the fields and
functions are mostly the same so FRED code should be easy to port to this new class.
- `EditorViewport` is a single view into an editor instance. It encapsulates the state of a single main window and could be
used in the future to support multiple viewports (which is also the reason it exists). Dialogs use an instance of this for
keeping a reference to their parent editor viewport.

#### Dialog models
The dialogs follow a similar system. Each `QDialog` subclass should have a corresponding `AbstractDialogModel` subclass. Obviously
this is not required for non-editor dialogs like the About dialog.

This should be done for every dialog but if the original FRED code is too reliant on the old programming model and refactoring
would require too much work it is acceptable to break this system for the initial qtFRED port.

### Adding new files
New files need to be added to the appropriate folders in `source_groups.cmake`. If a new source folder is created it should be added
with an appropriate name.

UI form files need to be added to the `UI` folder. They will be automatically included in the build that way and they will be
recompiled when the form is changed at a later time.
