# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


## [3.8.0] - 2017-08-22
### Major Changes
* Support for APNG animations: [APNG](https://en.wikipedia.org/wiki/APNG) is a variant of the established PNG format which allows to store animations instead of single frames in a PNG file. This change allows to use these animation files in FSO which significantly improves usability for modders since they no longer have to store all individual frames as separate images.
* Physically-Based Rendering (PBR): This upgrade of our rendering engine allows to use [PBR](https://en.wikipedia.org/wiki/Physically_based_rendering) assets to make models look more realistic and allow more artistic freedom for modders since they have more control over how a model looks. This also added support for HDR lighting which should improve the overall graphics experience.
* SDL2 usage on all platforms: SDL is a library for abstracting away the differences between different platforms. We now use SDL2 for all platforms which reduces the amount of platform-specific code tremendously and should result in better usability on all platforms. This was a major change that has been in development for several years. Here are a few key features that were added:
* * Pilot and configuration data is now stored in the correct location across all platforms (this allows to run FSO on Windows from the Program Files directory without administrator rights)
* * All platforms now use the ini files for storing settings. This fixes a lot of issues with the registry on Windows.
* * Better support for input devices. Since SDL handles keyboard, mouse and joystick input we now have better support on newer OS versions. Note: This does not mean that we support multiple joysticks (yet). There is ongoing development effort to support this but this release does not have that yet.
* CMake build system generation: This isn't relevant for players but we are now using [CMake](https://en.wikipedia.org/wiki/CMake) for handling compiling our builds. This improves cross-platform support and allows to implement advanced compilation features across multiple platforms. Modders will like the new "FastDebug" builds which are like the previous "Debug" builds but are compiled with all the optimizations of normal Release builds. That should make modding a lot easier since you can now debug your mod with almost the same performance as a Release build.
* Improved shield effects: Rendering of the shields is now handled by special shaders which improves the overall quality of the effects and allows more freedom for future effects.
* Native particle systems: Particles have always been supported by FSO but the effects that could be created by them were very limited. There were some attempts to fix this by using Lua scripting for more advanced features but that suffered from performance issues. With these new particle systems that feature has been integrated directly into the engine which should improve performance and allow for better effects in the future.
* TrueType Font support: [TrueType](https://en.wikipedia.org/wiki/TrueType) fonts improve the text rendering capabilities of FSO by allowing to use freely scaleable font faces instead of the previous bitmap fonts.
* Use OpenGL Core Profile for rendering: This is another major graphical upgrade which adds support for the OpenGL Core profile across all platforms (this was also made possible by the SDL2 integration). This upgrade allows us to use more modern rendering techniques and is especially useful for our Linux users who use the open-source Mesa drivers since our shaders failed to compile with those drivers. Now everyone will be able to enjoy the new graphical features added in this and previous releases. This also made some internal changes to how we handle rendering which improves the usability of our rendering engine within our code.
* Use FFmpeg for video & audio decoding: [FFmpeg](https://www.ffmpeg.org/) is a multi-media library which exposes functionality for decoding video and audio files to their raw form so that we can use that data. Thanks to this library we can now play 1080p cutscenes without any stuttering or frame-timing issues. It also allows to use more advanced audio and video codecs such as H.264 for video or Opus for audio.

### Other Changes


## [3.7.4] - 2016-07-04