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

Rendering backend (OpenGL / Vulkan)
------------------------------------
qtFRED can render through either backend. OpenGL is still the default; pass `-vulkan` on the
command line, or choose it in Preferences > Graphics, to use Vulkan instead. If the windowing
implementation can't present through Vulkan (a Qt build without `QT_CONFIG(vulkan)`, or a platform
plugin with no known surface extension), `gr_init()` logs it and falls back to OpenGL rather than
failing (`code/graphics/2d.cpp`).

Vulkan presents through Qt's own Vulkan integration (`QWindow::setSurfaceType(QSurface::VulkanSurface)`
plus `QVulkanInstance`, adopting the engine's `VkInstance`) rather than through SDL.
`QtGraphicsOperations` implements `os::VulkanSurfaceProvider` (`code/osapi/vulkan_surface.h`), the
interface `VulkanRenderer` uses for everything that depends on the windowing toolkit — loading the
Vulkan loader, the required instance extensions, and `VkSurfaceKHR` creation for a given viewport.
`SDLGraphicsOperations` implements the same interface for the game. `fred2`'s `MFCGraphicsOperations`
does not, so `getVulkanSupport()` falls back to the `os::GraphicsOperations` base class default of
`nullptr` there, and `-vulkan` always falls back to OpenGL in the MFC editor.

qtFRED presents to two independent surfaces: the main viewport, and the briefing map's own window,
which renders on its own timer and switches between them with `gr_use_viewport()`. `VulkanRenderer`
keeps a `VulkanPresentTarget` (surface, swap chain, framebuffers, sync objects) per viewport it has
been asked to present to, created the first time that viewport is used and torn down when the
viewport goes away — e.g. when the briefing editor dialog is closed, which happens against a live
device since `BriefingEditorDialog` is built with `Qt::WA_DeleteOnClose`.

**Known limitation:** the main viewport does not present while the briefing map's timer is driving
the render loop (it sits on an already-acquired swap chain image the whole time). Not a regression
and doesn't block using the backend; see that document's *Follow-up work* section.

`fred2/` (the Windows MFC editor) is out of scope and stays OpenGL-only.

Known issues
------------

### The Gamma preference does nothing on OpenGL
Preferences > Graphics > Gamma sets `Gr_gamma`, but OpenGL only applies it inside the
`if (Cmdline_window_res)` branch of `gr_opengl_flip()`, and `gr_init()` deliberately does not set
`Cmdline_window_res` when `Fred_running` — so in the editor the value is stored and then ignored.
The Vulkan backend has no such branch and does apply it.

This was invisible for as long as nothing looked at the value: qtFRED inherited
`gr_set_gamma(3.0f)` verbatim from retail FRED2 (`fred2/management.cpp` still has it), where it was
equally dead. Once the Vulkan backend started honouring it, that 3.0 became a whole-frame
`pow(colour, 1/3)` and the viewport was visibly wrong. The default is now 1.0, which is both the
engine's own default and what the editor has always actually rendered with.

Note that a default is only a default: anyone who ran an earlier build of this branch has
`view_graphics_gamma=3` written into `qtFRED.conf` (or `%APPDATA%` on Windows) and needs to change
it in Preferences — the stored value wins.

Fixing the OpenGL side means giving FRED the intermediate buffer the gamma pass reads from, which
is exactly the `Cmdline_window_res` FRED integration work `gr_init()` already flags as unfinished.

### Blank/empty render viewport under Wayland
On Linux, qtFRED's main 3D viewport can render as a completely blank panel — no starfield, no
grid, no models, nothing — when running under a native Wayland session (confirmed on Arch Linux
with a KDE Plasma/Wayland session). The rest of the UI works fine, and the underlying FSO/OpenGL
init succeeds without errors; only the embedded render surface fails to display anything.

**Cause**: `RenderWidget` (`qtfred/src/ui/widgets/renderwidget.cpp`) creates the OpenGL render
surface as a raw `QWindow` and embeds it into the widget hierarchy via
`QWidget::createWindowContainer()`. This relies on native child-window embedding, which X11
supports directly (`XReparentWindow`) but Wayland has no equivalent for — Qt's Wayland QPA plugin
emulates it with `wl_subsurface`, and that emulation is a known source of bugs across
Qt/compositor version combinations, up to and including the embedded surface never compositing
into the visible window at all (i.e. exactly this symptom). qtFRED has no Wayland-specific
handling anywhere in its code, so it hits this unmodified.

**Workaround**: force Qt to run through XWayland (the X11 compatibility layer nearly all Wayland
compositors, including KWin, provide) instead of the native Wayland backend:

```
QT_QPA_PLATFORM=xcb ./qtfred
```

This has been confirmed to fix the blank viewport. There is no code-level fix for this yet — a
real fix would mean replacing the `createWindowContainer`-based render surface with something
Wayland-native (e.g. a top-level `QWindow`/`QOpenGLWindow` instead of an embedded child window,
or a `QOpenGLWidget`-based surface), which is a larger architectural change to qtFRED's rendering
widget, not a quick patch.

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
