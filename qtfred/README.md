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
qtFRED currently always initializes the OpenGL renderer, even if `-vulkan` is passed on the command line
(`Fred_running` forces `mode = GraphicsAPI::OpenGL` in `gr_init()`, `code/graphics/2d.cpp`). This is intentional:
qtFRED's windowing (`QtGraphicsOperations`/`QtViewport` in `qtfred/src/ui/QtGraphicsOperations.cpp`) only knows how
to create a Qt-native (`QOpenGLWidget`-backed) render surface. Unlike the retail engine's `SDLGraphicsOperations`,
it never creates a real `SDL_Window`, and `QtViewport::toSDLWindow()` unconditionally returns `nullptr`.

The Vulkan backend (`code/graphics/vulkan/`), however, was written exclusively against `SDLGraphicsOperations`'s
windowing:
- `VulkanRenderer::initializeInstance()` (`VulkanRendererSetup.cpp`) obtains `vkGetInstanceProcAddr` via
  `SDL_Vulkan_GetVkGetInstanceProcAddr()`, which only returns a valid pointer once SDL has loaded the Vulkan
  loader — which normally happens automatically when a window is created with the `SDL_WINDOW_VULKAN` flag
  (see `freespace2/SDLGraphicsOperations.cpp`).
- `VulkanRenderer::initializeSurface()` creates the `VkSurfaceKHR` via `SDL_Vulkan_CreateSurface(window, ...)`,
  which likewise needs a real `SDL_Window*`.

Since qtFRED never creates an SDL window, both calls fail: the first call aborts immediately
(`VULKAN_HPP_DEFAULT_DISPATCHER.init()` is handed a null function pointer), and even if that were papered over,
surface creation would fail right after.

If you removed the `Fred_running` OpenGL override in `gr_init()` to experiment with Vulkan in the editor, this is
the crash you'll hit. To make Vulkan actually work in qtFRED, `QtGraphicsOperations` needs a real Vulkan surface
path, e.g. one of:

1. Create a genuine (possibly hidden/embedded) `SDL_Window` with `SDL_WINDOW_VULKAN` set purely so the existing
   SDL-based Vulkan plumbing (loader + instance extensions + surface creation) keeps working unmodified, while
   still presenting through Qt.
2. Bypass SDL for Vulkan entirely: load the Vulkan loader directly (`SDL_Vulkan_LoadLibrary(nullptr)` works
   without any window), and create the `VkSurfaceKHR` from the Qt window's native handle
   (`QWindow::winId()`/native handle APIs) using the appropriate platform extension
   (`VK_KHR_win32_surface`, `VK_KHR_xcb_surface`, `VK_KHR_wayland_surface`, etc.) instead of
   `SDL_Vulkan_CreateSurface`.

Either approach is a real chunk of implementation work, not a quick fix — plan for it as its own task rather than
bundling it with unrelated changes.

Known issues
------------

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
