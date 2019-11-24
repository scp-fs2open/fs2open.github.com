# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [19.0.0-RC1] - 2019-08-29 [Thread](https://www.hard-light.net/forums/index.php?topic=95868.0)
### Meta
- With this release we decided to drop the "`3.Major_revision.Minor_revision`" versioning scheme in favor of a year based scheme since the Major and Minor versions did not have much meaning anymore. Instead the scheme will now be "`<year>.<number that is incremented every release>.0`". The last 0 is still there because some of our systems expect that. It will be gone at some point.
### Added
- Full Unicode text support: You can now use non-ASCII characters without having to worry about special fonts and special characters. This is an opt-in mod flag.
- Translation features for making it easier to only distribute one version of a mod which includes all languages.
- OpenAL Soft is included by default in binary distributions of FSO now.
- System for dynamically adding new SEXPs. With this a Lua script can expose functionality to the mission which can be used exactly the same as a standard SEXP with all the usual editing features in FRED.   
- Support for displaying decals on the surface of an object.
- Integrated support for the [Discord Rich Presence API](https://discordapp.com/rich-presence). 
- New markup based user interface system using [libRocket](https://github.com/libRocket/libRocket).
### Changed
- Exposed the movie player to the scripting API for advanced display features
- Various OpenGL optimizations for better graphics performance:
  - Animations now use texture arrays
  - Model uniforms get sent to the GPU using uniform buffers for less overhead
  - Various other minor changes
- Converted pilot files from custom binary format to JSON
### Fixed
- Refactored bitmap slot handling and removed the fixed upper limit on the number of bitmaps. No more bmpman corruption!

## [3.8.0] - 2017-08-22 [Release Thread](https://www.hard-light.net/forums/index.php?topic=93812.0)
### Added
- Support for APNG animations: [APNG](https://en.wikipedia.org/wiki/APNG) is a variant of the established PNG format which allows to store animations instead of single frames in a PNG file. This change allows to use these animation files in FSO which significantly improves usability for modders since they no longer have to store all individual frames as separate images.
- Physically-Based Rendering (PBR): This upgrade of our rendering engine allows to use [PBR](https://en.wikipedia.org/wiki/Physically_based_rendering) assets to make models look more realistic and allow more artistic freedom for modders since they have more control over how a model looks. This also added support for HDR lighting which should improve the overall graphics experience.
- Native particle systems: Particles have always been supported by FSO but the effects that could be created by them were very limited. There were some attempts to fix this by using Lua scripting for more advanced features but that suffered from performance issues. With these new particle systems that feature has been integrated directly into the engine which should improve performance and allow for better effects in the future.
- TrueType Font support: [TrueType](https://en.wikipedia.org/wiki/TrueType) fonts improve the text rendering capabilities of FSO by allowing to use freely scaleable font faces instead of the previous bitmap fonts.
### Changed
- We now use SDL2 on all platforms: SDL is a library for abstracting away the differences between different platforms. We now use SDL2 for all platforms which reduces the amount of platform-specific code tremendously and should result in better usability on all platforms. This was a major change that has been in development for several years. Here are a few key features that were added:
  - Pilot and configuration data is now stored in the correct location across all platforms (this allows to run FSO on Windows from the Program Files directory without administrator rights)
  - All platforms now use the ini files for storing settings. This fixes a lot of issues with the registry on Windows.
  - Better support for input devices. Since SDL handles keyboard, mouse and joystick input we now have better support on newer OS versions. Note: This does not mean that we support multiple joysticks (yet). There is ongoing development effort to support this but this release does not have that yet.
- CMake build system generation: This isn't relevant for players but we are now using [CMake](https://en.wikipedia.org/wiki/CMake) for handling compiling our builds. This improves cross-platform support and allows to implement advanced compilation features across multiple platforms. Modders will like the new "FastDebug" builds which are like the previous "Debug" builds but are compiled with all the optimizations of normal Release builds. That should make modding a lot easier since you can now debug your mod with almost the same performance as a Release build.
- Improved shield effects: Rendering of the shields is now handled by special shaders which improves the overall quality of the effects and allows more freedom for future effects.
- Use OpenGL Core Profile for rendering: This is another major graphical upgrade which adds support for the OpenGL Core profile across all platforms (this was also made possible by the SDL2 integration). This upgrade allows us to use more modern rendering techniques and is especially useful for our Linux users who use the open-source Mesa drivers since our shaders failed to compile with those drivers. Now everyone will be able to enjoy the new graphical features added in this and previous releases. This also made some internal changes to how we handle rendering which improves the usability of our rendering engine within our code.
- Use FFmpeg for video & audio decoding: [FFmpeg](https://www.ffmpeg.org/) is a multi-media library which exposes functionality for decoding video and audio files to their raw form so that we can use that data. Thanks to this library we can now play 1080p cutscenes without any stuttering or frame-timing issues. It also allows to use more advanced audio and video codecs such as H.264 for video or Opus for audio.


## [3.7.4] - 2016-07-04 [Release thread](http://www.hard-light.net/forums/index.php?topic=92181.0)
### Changed
- Major graphics update including deferred lighting and shadows
- Enhanced sound (up to 128 channels)
### Fixed
- "Tons" of bugfixes

## [3.7.2] - 2015-04-23 [Release thread](http://www.hard-light.net/forums/index.php?topic=89597.0)
### Added
- MediaVPS 2014 Support
- Autospread on model point shields
- WebUI based multiplayer standalone server (for non-Windows platforms)
- Toggle subsytem scanning, scannable, cargo revealed, hidden from sensors, stealth and friendly stealth added to the `alter-ship-flag` SEXP
- Support added to add more languages besides English, French, German and Polish, leading the way to better localization!
- `player-is-cheating` SEXP
- New ship flag to enable smarter AI use of afterburners. Now it can use them while flying waypoints or attacking capital ships (use the `free-afterburner-use` flag with `alter-ship-flag`)
- Experimental configurations to make AVX builds (see [this post](http://www.hard-light.net/forums/index.php?topic=85566.0) for more explanations)
- Frame profiling code, see what exactly is slowing down FreeSpace. (Mostly for devs or coders)
### Changed
- Unstretched interface screens
- Automatically non stretched HUD gauges and simpler modder options for placing HUD gauges
- `show-subtitle-text` can now display messages as well as raw text
- Allow a user-specified number of credit images
- event.log can now snapshot events checked in a frame and record the status of an event the previous time it was checked as well as the current state
- Allow the specification of the positions of medals, callsigns and labels on the medal screen. 
- The number of medals is now dynamic!
- `is-facing` now works for things that aren't ships (like waypoints!) 
- allowing `fade-in` and `fade-out` sexps to use any color, not just black, white and red.


## [3.7.0] - 2013-08-31 [Release thread](http://www.hard-light.net/forums/index.php?topic=85435.0)
### Major Change
- New Flexible Pilot Save File Code (combines single/multi pilots & allows safe switching between unfinished campaigns)
### Changed
- Raised the per-model debris limit to 48 (previously 32).
- Increased the per-frame debris limit to 96 (previously 64)

## [3.6.18] - 2013-03-01 [Release thread](http://www.hard-light.net/forums/index.php?topic=83889.0)
### Fixed
- Critical damage bug which caused heavy balance issues in the game

## [3.6.16] - 2013-01-31 [Release thread](http://www.hard-light.net/forums/index.php?topic=83577.0)
### Added
- Diaspora TC support
### Changed
- Performance Improvements

## [3.6.14] - 2012-10-23 - [Release thread](https://www.hard-light.net/forums/index.php?topic=82648.0)
### Changed
- Major HUD gauges overhaul
- Split the Locked flag so that you can lock Ships or Weapons on the game loadout screen independently
- Mixed ammo for primary and secondary weapons - [`$Substitute`](http://www.hard-light.net/wiki/index.php/Weapons.tbl#.24Substitute:) - Allows you to have a weapon that fires other weapon ammo in a specific pattern (say a Gatling gun with tracers).
- [`show-subtitle`](https://wiki.hard-light.net/index.php/SCP_SEXPs#show-subtitle) SEXP supports custom fonts
- The active support ship limit per side is now dynamic.  See [`set-support-ship`](https://wiki.hard-light.net/index.php/SCP_SEXPs#set-support-ship) for more info.
### Added
- Objecttype tables support for deciding how enemies respond to ship classes when disabled
- Allow modders to set a [mood](http://www.hard-light.net/forums/index.php?topic=81991.msg1639770#msg1639770) for builtin messages and then switch the mood for the mission on the fly using a SEXP
- Allow modders to set the chance of builtin messages being sent, how many will be sent per mission and how long is the delay between two messages
- Allow campaigns to be hidden in the campaign and tech rooms ([Game settings.tbl](https://wiki.hard-light.net/index.php/Game_settings.tbl#.23Ignored_Campaign_File_Names))
- Allow mods to set a default campaign ([Game settings.tbl](https://wiki.hard-light.net/index.php/Game_settings.tbl#.24Default_Campaign_File_Name:))
- New SEXPs
  - `set-player-orders`
  - `is-event\goal-true\false-msecs-delay`
  - `get\set-num-countermeasures`
  - `directive-value`
  - `invalidate-all-arguments`
  - `validate-all-arguments`
  - `has-primary-weapon`
  - `has-secondary-weapon`
  - [`hud-display-gauge`](http://www.hard-light.net/forums/index.php?topic=68837.msg1361711#msg1361711)
  - `get-object-speed-x`
  - `get-object-speed-y`
  - `get-object-speed-z`
### Fixed
- Various multiplayer SEXP fixes
- `rotating-subsys-set-time` now accelerates properly
- [Fixed a whole heap of memory, logic and coding errors](http://www.hard-light.net/forums/index.php?topic=78044.0) -- using PVS-Studio: C/C++ source code analysis and checking tool

