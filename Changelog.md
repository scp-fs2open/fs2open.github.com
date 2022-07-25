# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [22.2.0] - 2022-07-XX ([Thread](https://www.hard-light.net/forums/index.php?topic=XX))

```
Changelog note: 
As of this version, in an attempt to make the changelog easier to navigate and read,
each section will have collapsible subsections, ordered alphabetically by topic. 
Headlining features will always be on top.  
```
### Added

<details><summary> <b>**Big Feature: SEXP Containers**</b> </summary>

> As opposed to a SEXP variable, which stores only one value, a SEXP container stores a collection of values.  
> Consequently, containers allow for handling data in much more sophisticated ways than variables. 
> Containers come in two types:
> * List container - an ordered sequence of values, sort of a cross between Python lists and deques
> * Map container - pairs of keys associated with data, like Lua tables or Python dictionaries
> Further details in the [SEXP Containers announcement thread](https://www.hard-light.net/forums/index.php?topic=98209.0)
</details>

<details><summary> <b>Big Feature: LuaAI</b> </summary>

> LuaAI allows scripters to add a new AI Goal (like ai-chase) that will run script instead of internal logic.
> Adding a Lua AI section in a -sexp.tbm and a few functions to a corresponding -sct.tbm to define the AI logic in certain conditions. More documentation on these functions is available in the LuaAISEXP-Object of the Scripting Docs.
</details>

<details><summary>AI:</summary>

- `$AI ignores aspect lock for leading:` Flag, which lets AI continue to lead their target while performing an Aspect Lock
</details>

<details><summary>Animation: </summary>

- `pause on reverse` Flag
</details>

<details><summary>Controls: </summary>

- Handle international keyboard layouts
- Re-Add `-joy-info` command line flag, for slightly quicker Joystick GUID retrieval
- Add `-controlconfig_tbl` command line flag to print out a template controlconfigdefaults.tbl
</details>

<details><summary>FRED: </summary> 

- Support the full number of nebula poofs using a multi-select list
- Add the ability to edit nebula fog colors, since that previously was not part of the FRED UI
- Flags `same-arrival-warp-when-docked` and `same-departure-warp-when-docked` to allow docked ships to still use custom warp parameters
- Allow highlighting custom-colored events by drawing a border on them
- Can now rotate objects relative to other objects in the Object Editor
- Dialog to calculate relative distance and orientation between ships
- Flag "Point using uvec" to point ships while incorporating the ship's uvec
- `ai-attackable-if-no-collide` flag, which prevents turrets from ignoring a target with `no_collide`
</details>

<details><summary>QtFRED: </summary>

- Reinforcements Dialog
- Implemented Replace Data for Number and String
- Texture Replacement Dialog
- TBL Viewer in the Ship Editor
</details>

<details><summary>Graphics: </summary>

- Allow particles to use a random chance for spawning through the `+Chance` parameter
- Command-line parameter `-fov_cockpit` which allows setting an FoV for the cockpit model separately from the viewport FoV. This allows using custom FoVs without distorting the cockpit
- Allow user to export environment map to a PNG from the F3 Lab by pressing M
- Weapon lights: Configurable lighting data for weapons, both on a per-weapon basis and globally via lighting profiles
- Implemented debug sphere queuing and rendering
</details>

<details><summary>Localization: </summary>

- Make strings for Match Speed Indicator, Glide Indicator, and Afterburner Indicator translateable
</details>

<details><summary>Modding: </summary>

- `game_settings.tbl` setting `$Supernova hits at zero:`, which causes Supernovae to hit when the timer gets to 0, instead of the default of hitting 5 seconds before the time is up
- `game_settings.tbl` settings `$Show-subtitle uses pixels:` and `$Show-subtitle base resolution:`, which can be used in conjunction to scale subtitles
- `game_settings.tbl` setting `$Always warn player about unbound keys used in Directives Gauge:`, to show such warning to players when needed (without it these warnings only apply to Training Missions)
- Ship flags `fail-sound-locked-primary` and `fail-sound-locked-secondary` to allow the firing fail sound to play if the player tries to fire weapons that are locked
- Subsystem Flag "hide turret from loadout stats"
- Allow weapon to override turret name with `$Turret Name:`
</details>

<details><summary>Multiplayer: </summary>

- Save PXO stats to local pilot file
</details>

<details><summary>Performance: </summary>

- Preload subspace tunnel if the `mission-set-subspace` SEXP is used in the mission
</details>

<details><summary>SEXPs: </summary>

- `cancel-future-waves`, for stopping wings that still had waves queued from spawning
- Added optional flag to `show-subtitle-text` and `show-subtitle-image` to not change the subtitle aspect ratio
- `set-nav-color` and `set-nav-visited-color`, which allows setting custom colors to Navpoints in a mission
</details>

<details><summary>Scripting: </summary>

- Support for decals (see `decaldefinition` in the Scripting documentation)
- Missile list iterator: `mn.getMissileList()`
- Campaign Start hook: `On Campaign Begin`
- Player Loaded hook: `On Player Loaded`
- Added a few functions to get information on HUD gauges (check `HudGauge` in the Scripting documentation)
- Function `ship.checkVisibility` which checks if a ship can appear on the viewer's radar
- Function `hu.flashTargetBox` which causes the target box to flash
- Function `hu.getTargetDistance` to return the distance to target as reported by the HUD
- Function `mn.hasLineOfSight` which checks if one point has line of sight to the other
- Function `mn.getLineOfSightFirstIntersect` which checks for line of sight and returns the distance of the first interruption if not
- Variable `gr.CurrentResizeMode`, which can get or set the scaling mode to be used by the `gr.*` drawing methods
- Function `gr.draw3dLine`, which draws a line in space between two points
- Allow SCPUI to run `$On Briefing Stage:` hooks
- New bindings to allow proper hooking into control actions based on the control's enumeration name
- Allow scripted controls to override hardcoded functionality
- Fennel script support
- Lua BitOp 1.0.2 extension library, allowing the usage of bit operations
</details>

<details><summary>Technical: </summary> 

- Class encapsulation for Volition's linked list
- Improve a debug console message to be more informative
- Type-safe `TIMESTAMP` class
- Add `timestamp_since()` function
- Add methods to compare timestamps, calculate deltas from an existing timestamp and a number of milliseconds, and to calculate deltas between timestamps
- Create a command line option for logging data packets
- Add more information to Multi-related debug logging 
- Enable Doxygen for QTFred 
- Set up Mac builds on CI, allowing builds to be published automatically for Mac
- Add some logging to `waitAsync`
</details>

### Changed

<details><summary>Game: </summary>

- Only display one popup when the player gets to the main hall
- Allow an unlimited number of ship flags to be parsed at once
</details>

<details><summary>AI: </summary>

- Improve Player Orders to allow for a nearly unlimited number of player orders, as opposed to the 32 before
- Update `ai_turn_towards_vector` to use a target velocity for banking
</details>

<details><summary>Audio: </summary>

- Allow unlimited soundtracks and spooled music (Previously limited to 30 and 50 respectively)
- Allow unlimited sound effects
- Dynamically set the maximum number of audio channels based on how many the user's system can support
</details>

<details><summary>FRED: </summary>

- CTRL+V is now add-paste (used to be CTRL+P)
- CTRL+SHIFT+V is now overwrite-paste (used to be CTRL+V)
- Replace `Always_show_goals` with `Toggle_showing_goals` which toggles whether the goals are shown, and works in any mission (instead of only Training Missions)
- No longer disable icon dropdowns for models that have their own icons 
- No longer change the ship class when the icon type is a special popup
- Make the number of player orders FRED can handle dynamic
- Restore original Error Checking behavior where the function would exit after one check (for consistency)
- Exclude player ships from Error Checking for ships in a wing sharing the same orders
- Split "No subsystems found" and "Not all subsystems have a record in ships.tbl" warnings into two different warning flags
</details>

<details><summary>Graphics: </summary>

- Improved light attenuation to look more real
- Increase POF vertex limit, allowing up to `2^32 - 1` vertices to be used by a single subobject
</details>

<details><summary>Performance: </summary> 

- Delay `ai_new_maybe_reposition_attack_subsys` to only run once a second. Before this change, it could take around 13% of runtime in crowded missions
- Cached subsystem indexing, speeding up subsystem lookups, which run for multiple ships, multiple times per frame
</details>

<details><summary>Scripting: </summary> 

- Enhance help text for Lua SEXPs
- Converted the following Lua Hooks to the new format to add documentation: `On Death`, `On Game Init`, `On Ship Collision`
</details>

<details><summary>Technical: </summary>

- Turn `insertion_sort` into a templated function that uses assignment
- CMake: Update `PREBUILT_VERSION_NAME` to latest (mac libs)
- Optimized space used in the new POF version
- Cleanup model code
- Cleanup beam light functions by consolidating some duplicated code and removing unreachable logic
- Cleanup wing wave code
- Move special parsing functions into new `parsehi.cpp` and `parsehi.h` files
- Actually clear delayed SSM parsing data when it's no longer needed
- Allow C++ to interact more with LuaTables, allowing it to extract userdata like objects or ships from them
- Use `SCP_string` for constructing multi/event log lines 
- Replace `+Orders Accepted:` Bitfield with flag list
- Handle deprecated command line flags programmatically, avoiding lots of code duplication
- Set up debug_filter.cfg to have default filters that can be turned off
</details>

### Removed

<details><summary>FRED: </summary> 

- Remove the `Allow Daisy-Chained Docking` mission flag
- Remove "Delete Item" from the right-click menu in the Event Editor
</details>

<details><summary>Technical: </summary>

- Remove light filtering data which didn't work and increased code complexity
- Remove a pointless assert that checked if `SEXP_NODE_INCREMENT` (a define, set at 250) was greater than zero
</details>

### Fixed 

<details><summary>Game: </summary>

- Missing whitespace in credits 
- Stopped destroyed submodels from showing in Techroom
- Prevent a crash when an object dies in the same frame as their goal
- Always record primary bank ammo capacity during ship creation, preventing situations where a ballistic weapon would accidentally be set to zero ammo
- Properly remove ships from the Techroom when they're not in the Tech Database
- Load the correct Techroom data after a Tech Database Reset
- Don't tick Supernova timer while the game is paused
- Make `set-camera-facing` use default identity orientation when the `$Use host orientation` flag is active and no host is present
- When a hotkey text replacement string refers to an unbound control, use the name of the binding instead of "none"
- Fix usage of default tech database flags
- Prevent a crash in the loadout screen with a weapons-locked ship
- Fix two crashes in the F3 lab
- End the campaign properly when a mission is skipped, preventing crashing in some situations
- Prevent a crash when a message is sent from an invalid source
- Prevent a broken campaign state if the player quits the game from the options menu during the debriefing
- Add some checks before converting a message to Command
- Prevent accidentally skipping cutscenes caused by hitting the spacebar during a loading screen
- Fixed an issue where ships would start with the wrong hull values in Red Alert missions
</details>

<details><summary>AI: </summary> 

- Fix turret swarm weapons by removing a few checks that are no longer valid
- Fix a few issues with maneuvers
</details>

<details><summary>Animations: </summary> 

- Fix broken turret animations in the Techroom
</details>

<details><summary>Audio: </summary>

- Fix cockpit engine sound volume changing unexpectedly
- Fix a timing bug that would sometimes prevent music from being initialized when re-playing a mission
- Check all possible extensions for audio files
</details>

<details><summary>Controls: </summary>

- Flush the mouse state after toggling between `Use_mouse_to_fly` states, preventing situations where a stale mouse state would be used
- Fix a conflict between Free Look View controls and directional thrust controls
</details>

<details><summary>FRED: </summary>

- Prevent Ship Select Dialog from crashing if there are too many IFFs
- Support saving nebula fog colors
- Properly save `nav-carry-status` flag
- Don't treat a bare `<argument>` as an error
- Prevent adding duplicated variables and display an error to the user when they try
- Clean up some parse problems
- Fix assignment of alt-name when using the hash
- Guard against off-by-one errors in `replace_one` and `replace_all`
- Improve error checking of campaign loop branches
- Improve error reporting for bad SEXP nodes
- Prevent a crash with paste/add-paste when the clipboard was empty
- Prevent rounding/conversion issues that caused FRED to think an angle or position was modified when it wasn't
- Skip absent ships or wings when running `clear-goals`, instead of stopping the whole operation
</details>

<details><summary>Graphics: </summary>

- Disable Lightshafts while the Supernova glare effect is active. This partial fix ensures the glare shows up with the correct brightness
- Update and correctly load environment maps in F3 Lab
- Correctly set textures and Team Colors with `change-ship-class`
- Fixes Team Colors not working with `show-ship`
- Correct positioning and angles for background bitmaps and suns
- Clean up ship arrival and departure effects
- Fix clearing a skybox model when it isn't the mission default
- Fix an assertion with Fireball LODs which would wrongly trigger if the number of LODs was the same as the maximum
</details>

<details><summary>HUD: </summary>

- Make HUD Target distance consistent
- Fix a crash with scripted HUDs
- Correctly update RTT Cockpit Gauges with `change-ship-class`
- Ignore HUD brackets with a size of zero or less (instead of just crashing)
- Squad menu wasn't scrolling properly with PageDown in some cases
- Fix issues with loading of hud config presets
- Fix parsing issues with HUD color presets
  - Continue parsing remaining gauges if one isn't found
  - Use translated strings both when saving and loading
- Fix an issue where a target's orders would be displayed wrongly in the target box 
- Fix asteroid brackets disappearing or displaying wrong values
</details>

<details><summary>Modding: </summary>

- Delay processing `$Player Weapon Precedence:` until after parsing, avoiding ships starting with the wrong weapons in some situations
- Make the `$Player Weapon Precedence:` list actually give precedence
- Empty `+Orders Accepted List:` entries were causing the ship to accept the default orders, instead of no orders as would be correct
- Show a Warning if a turret submodel is not an immediate child of the base object and handle it as a single-part object to prevent a crash
</details>

<details><summary>Multiplayer: </summary>

- Limit client collisions to prevent bugs, such as flinging players with high pings who collided multiple times between packet updates
- Fix multiple timestamp issues
  - Multi messaging and multi pause screen
  - Make sure maintenance stuff (pings, voice, file xfer, etc.) isn't affected by pausing or time compression
  - Sync issues caused by options menu and screenshots
- Prevent an error with long messages in PXO Chat by using proper buffer size
- Fix some issues with Multiplayer SEXPs
  - Try to handle incompatible sexp packets better
  - Fix operator name issue with debug messages
  - Enable the new show-subtitle-text options
- Improve interpolation, fixing the jerkiness that used to occur when the interpolation code basically guessed the timing on when packets were coming in
- Fix rollback primary collisions, vastly improving accuracy on moving targets even at high pings
- Fix the standalone ship being added to ship list
- Stop non-printable keys adding text to multi message buffer
- Prevent the standalone from crashing if a player tried to switch secondaries in a ship that doesn't have secondaries
- Fix a standalone crash from disabled uniform buffer manager
- Prevent a crash related to Observer-type players
- Fix network issues on MacOS
</details>

<details><summary>Scripting: </summary>

- Make hook conditions consistent and handle combinations that didn't work properly before
- Fix `maybePlayCutscene` arguments
- Add `isActive` checks for new-style script hooks
- Sync the two `VIRTVAR`s for Orientation in subsystems and submodel instances, which behaved in different ways
- Fixed a few timing and framerate related issues for playing movies via script
- Ensure the Lua interpreter has enough space in its stack, and allow reserving more space if necessary
</details>

<details><summary>SEXPs: </summary>

- Avoid setting .rest for `Locked_sexp_true/false` with malformed SEXPs
- Fix some memory management problems in the `for-*` operators, avoiding a memory leak
- Fix crash when removing form-on-wing order with `remove-goal`
- Fix `show-subtitle` SEXP which was missing the `width` argument
- Reset volumes changed via `adjust-audio-volume` when the mission closes
- Prevent too many dynamic SEXPs from breaking the SEXP system
</details>

<details><summary>Technical: </summary>

- Check if index is valid before accessing `Weapon_info`, preventing CTDs under certain conditions
- When `weapon_create` was called while `MAX_WEAPONS` was reached, the previous algorithm didn't achieve desired results, and simply locked up the game. Instead, we now simply won't allow a new weapon to be created in such condition
- Refactored Supernova code and fixed some state transition issues in it
- Prevent crashes in the event of a subsystem mismatch during `change_ship_type`
- Change include order to fix compilation of unit tests on Mac
- Patch freeing of SEXP nodes, preventing a crash in certain conditions
- Fix a memory leak in model code
- Fix a crash in the Ship Creation code if a ship's name was exactly the maximum allowed length
- Removed vestigial bit-depth asserts for aabitmaps, preventing crashes with certain image types
- Prevent a crash when Escort Priority was parsed for a ship not present in the Escort List
- Fix a bunch of graphics functions to respect GR_STUB, preventing problems in standalone server
- Clean up line endings in several files (replaced CRLF with LF basically everywhere)
- Fixed a timestamp issue which helps lay the groundwork to add In-Game Joining to multiplayer
- Prevent pilot and campaign files from having invalid values, by clamping them within the expected ranges when saving and loading
  - Also warn the player if such an invalid value was detected, telling them which setting had the wrong value and what it was reset to
  - Try to avoid assertions by preventing invalid types from being handled by `obj_team()`
- Shut down the SEXP system before the Scripting system, otherwise a crash could happen if the SEXP system held references to Lua
</details>

----
	
## [22.0.0] - 2022-04-01 ([Thread](https://www.hard-light.net/forums/index.php?topic=98125.0))
### Changed
- Controls5 PR (aka multi-joy)
- Timing system upgrade
- Model animation code upgrades


## [21.4.0] - 2021-09-27 ([Thread](https://www.hard-light.net/forums/index.php?topic=97802.0))
### Changed
- Multiple features; see thread
- Multiple bugfixes; see thread
### Deprecations
- `script-eval` has been deprecated in favor of `script-eval-block`
- renamed `hud-set-retail-gauge-active` to `hud-set-builtin-gauge-active`


## [21.2.0] - 2021-05-02 ([Thread](https://www.hard-light.net/forums/index.php?topic=97561.0))
### Changed
- Added a dynamic action system for more modder control of effects
- Removed fireball limit
- Fixed bomb intercept in multiplayer
- Changed nebula fog distance from a linear to exponential model
- Turrets on moving subobjects enabled
- LZ4 Compression added for game files


## [21.0.0] - 2021-01-27 ([Thread](https://www.hard-light.net/forums/index.php?topic=97293.0))
### Changed
- More OpenGL optimizations
- Full type information output for Lua documentation. Very useful for writing Lua scripts.
- A new -weaponspew command-line option for printing MediaVP-style statistics, plus a weapon comparison spreadsheet, to the debug log.
- Improved ship lookup behavior in SEXPs for improved performance.
- BPTC/BC7 texture compression support
- Refactoring and new features for the ship lab.
- Missile multi-lock. It's finally in an official build!
- Enhancements to the model code in preparation for cool features in 21.2.
- IPv6 support for multiplayer
- A lot of general fixes for multiplayer.
### Deprecated
- Dropped support for Windows XP
- Scripting:
  - `ba.getFrametime()`: The parameter value was used incorrectly inside the implementation. To avoid breaking existing scripts and to improve readability this has been split into `ba.getMissionFrametime()` and `ba.getRealFrametime()`.
  - `gr.drawMonochromeImage()`: `gr.drawImage()` got a new parameter for drawing monochrome images and is more flexible in general so the monochrome variant is no longer needed.


## [19.0.0] - 2020-01-25 ([Thread](https://www.hard-light.net/forums/index.php?topic=96226.0))
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
- Replaced Blinn-Phong BRDF with GGX BRDF
### Fixed
- Refactored bitmap slot handling and removed the fixed upper limit on the number of bitmaps. No more bmpman corruption!


## [3.8.0] - 2017-08-22 ([Release Thread](https://www.hard-light.net/forums/index.php?topic=93812.0))
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


## [3.7.4] - 2016-07-04 ([Release thread](http://www.hard-light.net/forums/index.php?topic=92181.0))
### Changed
- Major graphics update including deferred lighting and shadows
- Enhanced sound (up to 128 channels)
### Fixed
- "Tons" of bugfixes


## [3.7.2] - 2015-04-23 ([Release thread](http://www.hard-light.net/forums/index.php?topic=89597.0))
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


## [3.7.0] - 2013-08-31 ([Release thread](http://www.hard-light.net/forums/index.php?topic=85435.0))
### Major Change
- New Flexible Pilot Save File Code (combines single/multi pilots & allows safe switching between unfinished campaigns)
### Changed
- Raised the per-model debris limit to 48 (previously 32).
- Increased the per-frame debris limit to 96 (previously 64)


## [3.6.18] - 2013-03-01 ([Release thread](http://www.hard-light.net/forums/index.php?topic=83889.0))
### Fixed
- Critical damage bug which caused heavy balance issues in the game


## [3.6.16] - 2013-01-31 ([Release thread](http://www.hard-light.net/forums/index.php?topic=83577.0))
### Added
- Diaspora TC support
### Changed
- Performance Improvements


## [3.6.14] - 2012-10-23 - ([Release thread](https://www.hard-light.net/forums/index.php?topic=82648.0))
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
### Changed
- Major HUD gauges overhaul
- Split the Locked flag so that you can lock Ships or Weapons on the game loadout screen independently
- Mixed ammo for primary and secondary weapons - [`$Substitute`](http://www.hard-light.net/wiki/index.php/Weapons.tbl#.24Substitute:) - Allows you to have a weapon that fires other weapon ammo in a specific pattern (say a Gatling gun with tracers).
- [`show-subtitle`](https://wiki.hard-light.net/index.php/SCP_SEXPs#show-subtitle) SEXP supports custom fonts
- The active support ship limit per side is now dynamic.  See [`set-support-ship`](https://wiki.hard-light.net/index.php/SCP_SEXPs#set-support-ship) for more info.
### Fixed
- Various multiplayer SEXP fixes
- `rotating-subsys-set-time` now accelerates properly
- [Fixed a whole heap of memory, logic and coding errors](http://www.hard-light.net/forums/index.php?topic=78044.0) -- using PVS-Studio: C/C++ source code analysis and checking tool

----

## [Older Changes] - Unformatted
```
ChangeLog for fs2_open.  All times are UTC.  

==================================================================
( massive gap in updates, maybe this will be filled in someday...)
==================================================================

2002-10-30 22:57  randomtiger

	* code/: code.dsp (1.6.2.8), graphics/grd3d.cpp (2.2.2.18),
	graphics/grd3dinternal.h (2.1.2.7), graphics/grd3dtexture.cpp
	(2.1.2.16), model/modelinterp.cpp (2.3.2.1):
	
	Changed DX8 code to not use set render and texture states if they
	are already set to that value.	Disabled buffer saving and
	restoring code when windowed to stop DX8 debug runs from crashing.
	- RT

2002-10-30 20:22  inquisitor

	* code/cmdline/cmdline.h (2.10):
	
	Bad Committ, Rolling back

2002-10-30 06:29  DTP

	* code/cfile/: cfilesystem.cpp (2.5), cfile.cpp (2.5):
	
	doh!, used upper case in include, dont know how much it matters for
	*nix systems, but here it is

2002-10-30 06:26  DTP

	* code/cfile/cfilesystem.cpp (2.4):
	
	DTP Implemented basic VP files handling. mission and campaign files
	inside VP files found in mod dir still not supported, cheking /
	creating directories not implented either

2002-10-29 22:41  sesquipedalian

	* code/parse/: sexp.cpp (2.3), sexp.h (2.3):
	
	no message

2002-10-28 02:28  DTP

	* code/cmdline/cmdline.h (2.9):
	
	Crap, somebody may restore cmdline.h to 2 version before this.

2002-10-28 00:40  randomtiger

	* code/graphics/grd3d.cpp (2.2.2.17):
	
	Implemented screen saving code for restoring when drawing popups, a
	bit slow but works. - RT

2002-10-27 23:59  DTP

	* code/cfile/: cfile.cpp (2.4), cfile.h (2.3):
	
	DTP; started basic implementation of mod-support plain files only
	for now. fs2_open.exe -mod X will look for files in fs2/ X
	/all-legal-subdirectories. no checking/creating dirs yet.
	directories must be there.

2002-10-27 23:55  DTP

	* code/cmdline/: cmdline.cpp (2.9), cmdline.h (2.8):
	
	DTP; started basic implementation of mod-support plain files only
	for now. fs2_open.exe -mod X will look for files in fs2/ X
	/all-legal-subdirectories. no checking/creating dirs yet.
	directories must be there.

2002-10-26 09:55  unknownplayer

	* code/: freespace2/freespace.cpp (2.6.2.5), nebula/neb.cpp
	(2.1.2.1):
	
	Fixed the nebula flicker bug. Check the NO_DIRECT3D conditional
	compile sections in the future for bits of code which may cause
	problems due to hacks correcting for DirectX5 that no longer apply.

2002-10-26 01:24  randomtiger

	* code/: bmpman/bmpman.cpp (2.2.2.10), bmpman/bmpman.h (2.0.2.7),
	graphics/grd3d.cpp (2.2.2.16), graphics/grd3drender.cpp (2.3.2.15),
	graphics/grd3dtexture.cpp (2.1.2.15):
	
	Fixed debug bitmap compiling bug.  Fixed tga bug. - RT

2002-10-22 23:02  randomtiger

	* code/: cmdline/cmdline.cpp (2.8), cmdline/cmdline.h (2.7),
	debugconsole/timerbar.h (1.2), hud/hudtargetbox.cpp (2.4),
	model/modelinterp.cpp (2.5):
	
	Made Phreaks alternative scanning style optional under the command
	line tag "-phreak" Fixed bug that changes HUD colour when
	targetting debris in a full nebula. - RT

2002-10-22 17:46  randomtiger

	* Freespace2.dsp (1.7.2.4), code/bmpman/bmpman.cpp (2.2.2.9),
	code/bmpman/bmpman.h (2.0.2.6), code/graphics/grd3dtexture.cpp
	(2.1.2.14):
	
	Fixed new TGA code texturing bug. - RT

2002-10-22 17:42  randomtiger

	* code/: freespace2/freespace.cpp (2.13),
	missionui/missionpause.cpp (2.3):
	
	Fixed lighting bug that caused special pause to crash on debug
	build.	Also added TAB functionality for special pause that toggles
	HUD. - RT

2002-10-21 16:33  randomtiger

	* code/: bmpman/bmpman.cpp (2.2.2.8), bmpman/bmpman.h (2.0.2.5),
	graphics/grd3d.cpp (2.2.2.15), graphics/grd3dinternal.h (2.1.2.6),
	graphics/grd3drender.cpp (2.3.2.14), graphics/grd3dtexture.cpp
	(2.1.2.13):
	
	Added D3D only 32 bit TGA functionality. Will load a texture as big
	as your graphics card allows. Code not finished yet and will forge
	the beginnings of the new texture system. - RT

2002-10-20 22:21  randomtiger

	* code/: bmpman/bmpman.cpp (2.2.2.7), graphics/grd3d.cpp
	(2.2.2.14), graphics/grd3drender.cpp (2.3.2.13), palman/palman.cpp
	(2.1.2.1):
	
	Some incomplete code to handle background drawing when message
	boxes are drawn.  It doesnt work but its a good base for someone to
	start from. - RT

2002-10-19 23:56  randomtiger

	* code/: bmpman/bmpman.cpp (2.2.2.6), bmpman/bmpman.h (2.0.2.4),
	globalincs/pstypes.h (2.4.2.3), graphics/grd3d.cpp (2.2.2.13),
	graphics/grd3dtexture.cpp (2.1.2.12):
	
	Changed generic bitmap code to allow maximum dimensions to be
	determined by 3D's engines maximum texture size query.	Defaults to
	256 as it was before. Also added base code for reworking the
	texture code to be more efficient. - RT

2002-10-19 19:29  bobboau

	* code/: controlconfig/controlsconfig.h (2.2),
	controlconfig/controlsconfigcommon.cpp (2.3), model/model.h (2.5),
	model/modelinterp.cpp (2.4), model/modelread.cpp (2.4),
	model/modelsinc.h (2.4), object/object.cpp (2.2), osapi/outwnd.cpp
	(2.2), physics/physics.cpp (2.2), physics/physics.h (2.2),
	playerman/playercontrol.cpp (2.2), ship/ai.h (2.2), ship/aibig.cpp
	(2.2), ship/aicode.cpp (2.8), ship/ship.cpp (2.7), ship/ship.h
	(2.4), ship/shipcontrails.cpp (2.3), ship/shipcontrails.h (2.1),
	ship/shiphit.cpp (2.6), starfield/starfield.cpp (2.3),
	weapon/beam.cpp (2.4), weapon/beam.h (2.3), weapon/weapon.h (2.2),
	weapon/weapons.cpp (2.2):
	
	inital commit, trying to get most of my stuff into FSO, there
	should be most of my fighter beam, beam rendering, beam sheild hit,
	ABtrails, and ssm stuff. one thing you should be happy to know is
	the beam texture tileing is now set in the beam section section of
	the weapon table entry

2002-10-19 03:50  randomtiger

	* code/: cmdline/cmdline.cpp (2.7), cmdline/cmdline.h (2.6),
	freespace2/freespace.cpp (2.12), io/keycontrol.cpp (2.4),
	menuui/readyroom.cpp (2.2), missionui/missionpause.cpp (2.2),
	missionui/missionpause.h (2.2):
	
	Added special pause mode for easier action screenshots.  Added new
	command line parameter for accessing all single missions in tech
	room. - RT

2002-10-17 20:40  randomtiger

	* code/: controlconfig/controlsconfig.h (2.1),
	controlconfig/controlsconfigcommon.cpp (2.2), hud/hud.cpp (2.2),
	hud/hud.h (2.2), io/keycontrol.cpp (2.3),
	mission/missionmessage.cpp (2.2):
	
	Added ability to remove HUD ingame on keypress shift O So I've
	added a new key to the bind list and made use of already existing
	hud removal code.

2002-10-16 00:41  randomtiger

	* code/graphics/: 2d.cpp (2.3.2.8), 2d.h (2.1.2.2), grd3d.cpp
	(2.2.2.12), grd3d.h (2.0.2.4), grd3drender.cpp (2.3.2.12):
	
	Fixed small bug that was stopping unactive text from displaying
	greyed out Also added ability to run FS2 DX8 in 640x480, however I
	needed to make a small change to 2d.cpp which invloved calling the
	resolution processing code after initialising the device for D3D
	only.  This is because D3D8 for the moment has its own internal
	launcher.  Also I added a fair bit of documentation and tidied some
	stuff up. - RT

2002-10-14 21:52  randomtiger

	* code/: bmpman/bmpman.cpp (2.2.2.5), graphics/grd3d.cpp
	(2.2.2.11), graphics/grd3drender.cpp (2.3.2.11),
	graphics/grd3dtexture.cpp (2.1.2.11):
	
	Finally tracked down and killed off that 8 bit alpha bug.  So the
	font, HUD and 8 bit ani's now work fine. - RT

2002-10-14 19:49  phreak

	* code/graphics/gropenglw32x.cpp (1.4):
	
	screenshots, yay!

2002-10-13 21:43  phreak

	* code/graphics/gropenglw32x.cpp (1.3):
	
	further optimizations

2002-10-12 17:48  phreak

	* code/graphics/gropenglw32x.cpp (1.2):
	
	fixed text

2002-10-11 21:35  phreak

	* code/graphics/gropenglw32.cpp (1.2):
	
	dont use this one

2002-10-11 21:30  phreak

	* code/graphics/: gropenglw32.cpp (1.1), gropenglw32x.cpp (1.1):
	
	first run at opengl for w32, only useful in main hall, barracks and
	campaign room

2002-10-11 18:50  randomtiger

	* code/graphics/: 2d.cpp (2.3.2.7), 2d.h (2.1.2.1), grd3d.cpp
	(2.2.2.10), grd3d.h (2.0.2.3), grd3dinternal.h (2.1.2.5),
	grd3drender.cpp (2.3.2.10), grd3dtexture.cpp (2.1.2.10):
	
	Checked in fix for 16 bit problem, thanks to Righteous1 Removed a
	fair bit of code that was used by the 16 bit code path which no
	longer exists.	32 bit and 16 bit should now work in exactly the
	same way. - RT

2002-10-08 14:33  randomtiger

	* code/: graphics/grd3d.cpp (2.2.2.9), graphics/grd3drender.cpp
	(2.3.2.9), graphics/grd3dtexture.cpp (2.1.2.9), osapi/outwnd.cpp
	(2.1.2.1):
	
	OK, I've fixed the z-buffer problem.  However I have abandoned
	using w-buffer for now because of problems.  I think I know how to
	solve it but Im not sure it would make much difference given the
	way FS2 engine works.  I have left code in there of use if anyone
	wants to turn their hand to it. However for now we just need to
	crack of the alpha problem then we will have FS2 fully wokring in
	DX8 on GF4 in 32 bit.

2002-10-05 16:46  randomtiger

	* code/: code.dsp (1.10), debugconsole/timerbar.cpp (1.1),
	debugconsole/timerbar.h (1.1), freespace2/freespace.cpp (2.11),
	globalincs/systemvars.h (2.2), graphics/grd3d.cpp (2.3),
	graphics/grd3dinternal.h (2.2), graphics/grd3drender.cpp (2.4),
	menuui/credits.cpp (2.3):
	
	Added us fs2_open people to the credits. Worth looking at just for
	that.  Added timer bar code, by default its not compiled in.  Use
	TIMEBAR_ACTIVE in project and dependancy code settings to activate.
	 Added the new timebar files with the new code.

2002-10-04 00:48  randomtiger

	* code/graphics/: grd3d.cpp (2.2.2.8), grd3dinternal.h (2.1.2.4),
	grd3drender.cpp (2.3.2.8), grd3dtexture.cpp (2.1.2.8):
	
	Fixed video memory leaks Added code to cope with lost device, not
	tested Got rid of some DX5 stuff we definately dont need Moved some
	enum's into internal,h because gr_d3d_set_state should be able to
	be called from any dx file Cleaned up some stuff - RT

2002-10-03 08:32  unknownplayer

	* code/: graphics/grd3d.cpp (2.2.2.7), graphics/grd3dtexture.cpp
	(2.1.2.7), osapi/osapi.cpp (2.2.2.2):
	
	Hacked in a windowed mode so we can properly debug this without
	using monitors (although I drool at the concept of having that!)

2002-10-02 17:52  randomtiger

	* code/: debugconsole/dbugfile.cpp (2.1.2.3),
	debugconsole/dbugfile.h (2.1.2.3), graphics/grd3d.cpp (2.2.2.6),
	graphics/grd3drender.cpp (2.3.2.7), graphics/grd3dtexture.cpp
	(2.1.2.6):
	
	Fixed blue lighting bug.  Put filtering flag set back in that I
	accidentally removed Added some new functionality to my debuging
	system - RT

2002-10-02 11:40  randomtiger

	* code/: code.dsp (1.6.2.7), bmpman/bmpman.cpp (2.2.2.4),
	bmpman/bmpman.h (2.0.2.3), graphics/2d.cpp (2.3.2.6),
	graphics/grd3d.cpp (2.2.2.5), graphics/grd3d.h (2.0.2.2),
	graphics/grd3dinternal.h (2.1.2.3), graphics/grd3drender.cpp
	(2.3.2.6), graphics/grd3dtexture.cpp (2.1.2.5),
	pcxutils/pcxutils.cpp (2.1.2.2):
	
	Bmpmap has been reverted to an old non d3d8 version.  All d3d8 code
	is now in the proper place.  PCX code is now working to an extent.
	Problems with alpha though.  Ani's work slowly with alpha problems.
	 Also I have done a bit of tidying - RT

2002-09-28 22:13  randomtiger

	* code/: anim/animplay.cpp (2.1.2.2), bmpman/bmpman.cpp (2.2.2.3),
	graphics/grd3d.cpp (2.2.2.4), graphics/grd3dinternal.h (2.1.2.2),
	graphics/grd3drender.cpp (2.3.2.5), graphics/grd3dtexture.cpp
	(2.1.2.4), weapon/weapons.cpp (2.1.2.2):
	
	Sorted out some bits and pieces. The background nebula blends now
	which is nice. – RT

2002-09-28 12:20  randomtiger

	* code/graphics/: grd3d.cpp (2.2.2.3), grd3drender.cpp (2.3.2.4),
	grd3dtexture.cpp (2.1.2.3):
	
	Just a tiny code change that lets stuff work in 16 bit.  For some
	reason 16 bit code was taking a different code path for displaying
	textures.  So until I unstand why, Im cutting off that codepath
	because it isnt easy to convert into DX8.

2002-09-28 00:18  randomtiger

	* code/: bmpman/bmpman.cpp (2.2.2.2), bmpman/bmpman.h (2.0.2.2),
	graphics/grd3drender.cpp (2.3.2.3), graphics/grd3dtexture.cpp
	(2.1.2.2):
	
	Did some work on trying to get textures to load from pcx, define
	TX_ATTEMPT for access to it.  Converted some DX7 blending calls to
	DX8 which a fair difference to ingame visuals.
	
	- RT

2002-09-24 18:56  randomtiger

	* Freespace2.dsp (1.7.2.3), code/code.dsp (1.6.2.6),
	code/anim/animplay.cpp (2.1.2.1), code/bmpman/bmpman.cpp (2.2.2.1),
	code/bmpman/bmpman.h (2.0.2.1), code/cfile/cfile.cpp (2.3.2.1),
	code/cmeasure/cmeasure.cpp (2.1.2.1),
	code/controlconfig/controlsconfig.cpp (2.1.2.1),
	code/cutscene/cutscenes.cpp (2.2.2.4), code/cutscene/movie.cpp
	(2.1.2.4), code/cutscene/movie.h (2.1.2.2),
	code/debugconsole/console.cpp (2.1.2.1),
	code/debugconsole/dbugfile.cpp (2.1.2.2),
	code/debugconsole/dbugfile.h (2.1.2.2),
	code/freespace2/freespace.cpp (2.6.2.4),
	code/freespace2/freespace.rc (2.0.2.1),
	code/freespace2/freespaceresource.h (2.0.2.1),
	code/gamehelp/gameplayhelp.cpp (2.1.2.1), code/globalincs/pstypes.h
	(2.4.2.2), code/graphics/2d.cpp (2.3.2.5), code/graphics/bitblt.cpp
	(2.1.2.1), code/graphics/grd3d.cpp (2.2.2.2), code/graphics/grd3d.h
	(2.0.2.1), code/graphics/grd3dinternal.h (2.1.2.1),
	code/graphics/grd3drender.cpp (2.3.2.2),
	code/graphics/grd3dtexture.cpp (2.1.2.1),
	code/graphics/grdirectdraw.cpp (2.1.2.2), code/hud/hudconfig.cpp
	(2.2.2.1), code/hud/hudmessage.cpp (2.1.2.1),
	code/menuui/barracks.cpp (2.2.2.1), code/menuui/credits.cpp
	(2.2.2.1), code/menuui/mainhallmenu.cpp (2.3.2.4),
	code/menuui/mainhalltemp.cpp (2.1.2.1), code/menuui/optionsmenu.cpp
	(2.1.2.1), code/menuui/playermenu.cpp (2.2.2.1),
	code/menuui/readyroom.cpp (2.1.2.1), code/menuui/techmenu.cpp
	(2.1.2.1), code/menuui/trainingmenu.cpp (2.1.2.1),
	code/mission/missioncampaign.cpp (2.3.2.3),
	code/mission/missiongoals.cpp (2.1.2.1),
	code/mission/missionhotkey.cpp (2.1.2.1),
	code/mission/missionload.cpp (2.1.2.1),
	code/missionui/missionbrief.cpp (2.1.2.1),
	code/missionui/missioncmdbrief.cpp (2.1.2.1),
	code/missionui/missiondebrief.cpp (2.2.2.1),
	code/missionui/missionloopbrief.cpp (2.1.2.1),
	code/missionui/missionpause.cpp (2.1.2.1),
	code/missionui/missionshipchoice.cpp (2.1.2.1),
	code/missionui/missionweaponchoice.cpp (2.1.2.1),
	code/missionui/redalert.cpp (2.1.2.1),
	code/network/multi_dogfight.cpp (2.1.2.1),
	code/network/multi_ingame.cpp (2.1.2.1),
	code/network/multi_pinfo.cpp (2.1.2.1),
	code/network/multiteamselect.cpp (2.2.2.1),
	code/network/multiui.cpp (2.4.2.1), code/pcxutils/pcxutils.cpp
	(2.1.2.1), code/popup/popup.cpp (2.1.2.1), code/render/3ddraw.cpp
	(2.2.2.1), code/ship/ship.cpp (2.6.2.1), code/stats/medals.cpp
	(2.2.2.1), code/ui/gadget.cpp (2.1.2.1), code/weapon/weapons.cpp
	(2.1.2.1):
	
	DX8 branch commit
	
	This is the scub of UP's previous code with the more up to date RT
	code.  For full details check previous dev e-mails

2002-09-20 20:09  phreak

	* code/freespace2/freespace.cpp (2.10):
	
	did glare stuff in game_sunspot_process()

2002-09-20 20:05  phreak

	* code/starfield/starfield.cpp (2.2):
	
	glare parser stuff in stars_init()

2002-09-20 20:04  phreak

	* code/starfield/starfield.h (2.2):
	
	added glare variable for ambient suns if glare is 0 then the sun
	glare whiteout is not shown when looking at the sun

2002-09-20 20:02  phreak

	* code/hud/hudtarget.cpp (2.2):
	
	lead indicator for dumbfire missiles

2002-09-20 20:01  phreak

	* code/hud/hudtargetbox.cpp (2.3):
	
	extra effects during cargo scan

2002-09-10 21:58  unknownplayer

	* code/graphics/directx8/GrD3d81Stubs.cpp (1.1):
	
	file GrD3d81Stubs.cpp was initially added on branch directx8.

2002-09-10 21:58  unknownplayer

	* code/: freespace2/freespace.cpp (2.6.2.3), graphics/2d.cpp
	(2.3.2.4), graphics/grdirectdraw.cpp (2.1.2.1),
	graphics/directx8/GrD3D81.cpp (1.2.2.4),
	graphics/directx8/GrD3D81.h (1.1.2.4),
	graphics/directx8/GrD3d81Stubs.cpp (1.1.2.1):
	
	Added the DX8 stub file, plus new CODE!

2002-09-08 02:36  unknownplayer

	* code/: code.dsp (1.6.2.5), graphics/2d.cpp (2.3.2.3),
	graphics/grd3d.cpp (2.2.2.1), graphics/grd3drender.cpp (2.3.2.1),
	graphics/directx8/GrD3D81.cpp (1.2.2.3),
	graphics/directx8/GrD3D81.h (1.1.2.3):
	
	Framework for the new D3DRENDER class is in place, and gr_screen
	function structures are now there too. The way it's done is
	probably kind of inefficient, but until we're further along
	actually drawing polygons we should keep it that way for
	consistency's sake.

2002-09-01 08:18  unknownplayer

	* Freespace2.dsp (1.7.2.2), code/code.dsp (1.6.2.4),
	code/globalincs/pstypes.h (2.4.2.1), code/graphics/2d.cpp
	(2.3.2.2), code/graphics/directx8/GrD3D81.cpp (1.2.2.2),
	code/graphics/directx8/GrD3D81.h (1.1.2.2),
	code/menuui/mainhallmenu.cpp (2.3.2.3):
	
	Started writing some code for the new DX8.1 class and hit a serious
	snag to do with window handling and DirectDraw (and possibly my
	unfamiliarity with DX). It seems we may need to make some changes
	further up the code because we no longer use DX through DD in 8.1
	and the V code did that a lot (to go fullscreen and such check out
	the device creation code)

2002-08-31 04:50  unknownplayer

	* code/: code.dsp (1.6.2.3), graphics/2d.cpp (2.3.2.1),
	graphics/directx8/GrD3D81.cpp (1.2.2.1),
	graphics/directx8/GrD3D81.h (1.1.2.1):
	
	Removed some older DX81 stuff I added before and changed some other
	things.  I'm currently thinking that I'm going to run with putting
	everything into a specialized class with the d3d_init and
	d3d_cleanup functions as gods.	(Creation / Destruction functions -
	i.e. gods :)

2002-08-31 04:48  unknownplayer

	* code/graphics/directx8/grd3d81internal.h (1.2.2.1):
	
	These are all officially now out of FS2 unless you have a specific
	reason to need them. I'm running with the idea that we make a C++
	class and use that as an effective way to control the code.
	Rewriting V's stuff is just insanely difficult.

2002-08-28 21:47  randomtiger

	* Freespace2.dsp (1.10), code/cutscene/cutscenes.cpp (2.2.2.3),
	code/cutscene/movie.cpp (2.1.2.3):
	
	Tiny change to DX branch to keep movie code optional (default off)

2002-08-28 12:39  randomtiger

	* code/: code.dsp (1.6.2.2), cutscene/cutscenes.cpp (2.2.2.2),
	cutscene/movie.cpp (2.1.2.2), debugconsole/dbugfile.cpp (2.1.2.1),
	debugconsole/dbugfile.h (2.1.2.1), directx/dx8show.cpp (1.1.2.2),
	directx/dx8show.h (1.1.2.2), freespace2/freespace.cpp (2.6.2.2),
	menuui/mainhallmenu.cpp (2.3.2.2), mission/missioncampaign.cpp
	(2.3.2.2), osapi/osapi.cpp (2.2.2.1), osapi/osapi.h (2.2.2.1):
	
	OK, this should be a commit to the DX branch or Im going to be in a
	lot of trouble.  The movie and dx8show files have been cleaned up
	big time.  My debug system is in but has NO EFFECT at all unless a
	compiler flag is turned on, check h file for details.  Aside from
	that a few changes to help the movie code work properly.  Works on
	most things including GF4 and Voodoo 3. However may not work
	properly on a voodoo 2.  Im going to leave this as a bug for now,
	serves you right for buying voodoo!

2002-08-28 10:51  randomtiger

	* code/: code.dsp (1.9), debugconsole/dbugfile.cpp (2.1),
	debugconsole/dbugfile.h (2.1), freespace2/freespace.cpp (2.9):
	
	Woh! I sure as hell didnt modify all these files it says I did.  I
	will put this down to the branch! Note: I did start from a fresh
	checkout!

2002-08-27 13:38  penguin

	* Freespace2.dsp (1.9), code/code.dsp (1.8),
	code/cmdline/cmdline.cpp (2.6), code/cmdline/cmdline.h (2.5),
	code/cutscene/cutscenes.cpp (2.4), code/cutscene/movie.cpp (2.2),
	code/cutscene/movie.h (2.2), code/directx/DShow.h (1.2),
	code/directx/ddraw.lib (1.2), code/directx/dx8show.cpp (1.2),
	code/directx/dx8show.h (1.2), code/directx/dxguid.lib (1.2),
	code/directx/strmiids.lib (1.2), code/freespace2/freespace.cpp
	(2.8), code/menuui/mainhallmenu.cpp (2.5),
	code/mission/missioncampaign.cpp (2.5):
	
	Moved DirectX8 stuff to directx8 branch; reverted to previous

2002-08-27 13:25  penguin

	* code/directx/: DShow.h (1.1.2.1), ddraw.lib (1.1.2.1),
	dx8show.cpp (1.1.2.1), dx8show.h (1.1.2.1), dxguid.lib (1.1.2.1),
	strmiids.lib (1.1.2.1):
	
	Moved to directx8 branch

2002-08-27 13:21  penguin

	* code/: freespace2/freespace.cpp (2.6.2.1),
	menuui/mainhallmenu.cpp (2.3.2.1), mission/missioncampaign.cpp
	(2.3.2.1), code.dsp (1.6.2.1), cmdline/cmdline.cpp (2.4.2.1),
	cmdline/cmdline.h (2.3.2.1), cutscene/cutscenes.cpp (2.2.2.1),
	cutscene/movie.cpp (2.1.2.1), cutscene/movie.h (2.1.2.1),
	Freespace2.dsp (1.7.2.1):
	
	Moved to directx8 branch

2002-08-18 19:48  randomtiger

	* Freespace2.dsp (1.8), code/code.dsp (1.7),
	code/cmdline/cmdline.cpp (2.5), code/cmdline/cmdline.h (2.4),
	code/cutscene/cutscenes.cpp (2.3), code/cutscene/movie.cpp (2.1),
	code/cutscene/movie.h (2.1), code/directx/DShow.h (1.1),
	code/directx/ddraw.lib (1.1), code/directx/dx8show.cpp (1.1),
	code/directx/dx8show.h (1.1), code/directx/dxguid.lib (1.1),
	code/directx/strmiids.lib (1.1), code/freespace2/freespace.cpp
	(2.7), code/menuui/mainhallmenu.cpp (2.4),
	code/mission/missioncampaign.cpp (2.4):
	
	Added new lib files: strmiids and ddraw to get dshow working Added
	new command line parameter to active direct show movie play:
	-dshowvid Uncommented movie_play calls and includes

2002-08-15 04:41  penguin

	* code/globalincs/systemvars.h (2.1):
	
	Added #include, needed for FRED

2002-08-13 03:40  penguin

	RELEASED: fs2_open 3.2 (tag: fs2_open3_2)

2002-08-13 03:34  penguin

	* code/freespace2/freespace.cpp (2.6):
	
	1. Disable CD checking
	2. Add CVS tag to version string

2002-08-13 03:32  penguin

	* code/globalincs/version.h (2.2):
	
	Bumped to version 3.2

2002-08-07 00:45  DTP

	* code/graphics/grd3drender.cpp (2.3):
	
	Implented -GF4FIX commandline switch & #include "cmdline/cmdline.h"

2002-08-07 00:44  DTP

	* code/cmdline/: cmdline.cpp (2.4), cmdline.h (2.3):
	
	Implented -GF4FIX commandline switch

2002-08-06 16:50  phreak

	* code/hud/hudtargetbox.cpp (2.2):
	
	added wireframe targetbox feature

2002-08-06 16:50  phreak

	* code/hud/hudtargetbox.h (2.1):
	
	added Targetbox_wire variable to check what mode the hud targetbox
	uses

2002-08-06 16:49  phreak

	* code/io/keycontrol.cpp (2.2):
	
	added keybing for wireframe hud, fixed previous missile cheat

2002-08-06 04:39  penguin

	* code/hud/hudwingmanstatus.cpp (2.2):
	
	Use text strings for wingmen names instead of ANI

2002-08-06 03:50  penguin

	* code/ui/ui.h (2.2):
	
	inserted "class" keyword on "friend" definitions (ANSI C++ )

2002-08-06 02:42  penguin

	* Freespace2.dsp (1.7), code/code.dsp (1.6):
	
	Fixed problem w/ compile options for Debug mode

2002-08-06 01:49  penguin

	* code/Makefile (2.4):
	
	Removed a couple more include dirs I missed

2002-08-06 01:49  penguin

	* code/: globalincs/pstypes.h (2.4), render/3dclipper.cpp (2.2),
	render/3ddraw.cpp (2.2):
	
	Renamed ccode members to cc_or and cc_and

2002-08-04 05:43  penguin

	* ChangeLog (1.1), code/ChangeLog (1.3):
	
	Moved ChangeLog to top-level directory

2002-08-04 05:13  penguin

	RELEASED: fs2_open 3.1 (tag: fs2_open3_1)

2002-08-04 05:13  penguin

	* code/network/stand_gui.cpp (2.3):
	
	Change version display

	* code/menuui/playermenu.cpp (2.2):
	
	Display fs2_open version instead of "Freespace 2"

	* code/menuui/mainhallmenu.cpp (2.3):
	
	Change version display location

	* code/globalincs/version.h (2.1):
	
	Update version number to 3.1

	* code/freespace2/freespace.cpp (2.5):
	
	Don't write version to registry; change way version string is
	formatted

	* code/ChangeLog (1.1), ChangeLog (1.2):

	Moved ChangeLog to top-level directory

2002-08-03 19:42  randomtiger

	* code/graphics/grd3drender.cpp (2.2):
	
	Fixed Geforce 4 bug that caused font and hall video distortion. 
	Very small change in 'gr_d3d_aabitmap_ex_internal'
	
	Tested and works on the following systems
	
	OUTSIDER Voodoo 3 win98 OUTSIDER Geforce 2 win2000 Me Geforce 4 PNY
	4600 XP JBX-Phoenix Geforce 4 PNY XP Mehrunes GeForce 3 XP
	WMCoolmon nVidia TNT2 M64 win2000 Orange GeForce 4 4200 XP
	ShadowWolf_IH Monster2 win98 ShadowWolf_IH voodoo 2 win98

2002-08-03 18:34  wmcoolmon

	* code/code.dsp (1.5), Freespace2.dsp (1.6):
	
	Fixed Win32 debug configuration

2002-08-03 17:42  wmcoolmon

	* Freespace2.dsp (1.5), code/code.dsp (1.4):
	
	Sync with makefile 1.4

2002-08-01 04:04  penguin

	* code/ChangeLog (1.2):
	
	updated

2002-08-01 02:00  penguin

	* fs2_open.w32.mak (1.4):
	
	The big include file move

====
NOTE:
====
	below this point, all files are relative to the 'code' directory


2002-08-01 01:41  penguin

	* Makefile (2.3), anim/animplay.cpp (2.1), anim/animplay.h (2.1),
	anim/packunpack.cpp (2.1), anim/packunpack.h (2.1),
	asteroid/asteroid.cpp (2.1), asteroid/asteroid.h (2.1),
	bmpman/bmpman.cpp (2.2), cfile/cfile.cpp (2.3), cfile/cfile.h
	(2.2), cfile/cfilearchive.cpp (2.2), cfile/cfilelist.cpp (2.2),
	cfile/cfilesystem.cpp (2.3), cmdline/cmdline.cpp (2.3),
	cmeasure/cmeasure.cpp (2.1), cmeasure/cmeasure.h (2.1),
	controlconfig/controlsconfig.cpp (2.1),
	controlconfig/controlsconfigcommon.cpp (2.1),
	cryptstring/cryptstring.cpp (2.1), cutscene/cutscenes.cpp (2.2),
	cutscene/cutscenes.h (2.1), debris/debris.cpp (2.1),
	debugconsole/console.cpp (2.1), demo/demo.cpp (2.2), directx/vd3d.h
	(2.1), directx/vd3dcaps.h (2.1), directx/vd3di.h (2.2),
	directx/vd3drm.h (2.1), directx/vd3drmdef.h (2.2),
	directx/vd3drmobj.h (2.1), directx/vd3drmwin.h (2.2),
	directx/vd3dtypes.h (2.2), directx/vdplobby.h (2.1),
	directx/vdsound.h (2.1), fireball/fireballs.cpp (2.1),
	fireball/fireballs.h (2.1), fireball/warpineffect.cpp (2.1),
	freespace2/freespace.cpp (2.4), freespace2/freespace.h (2.1),
	freespace2/levelpaging.cpp (2.1), gamehelp/contexthelp.cpp (2.1),
	gamehelp/gameplayhelp.cpp (2.1), gamesequence/gamesequence.cpp
	(2.1), gamesequence/gamesequence.h (2.1), gamesnd/eventmusic.cpp
	(2.1), gamesnd/eventmusic.h (2.1), gamesnd/gamesnd.cpp (2.1),
	gamesnd/gamesnd.h (2.1), glide/glide.cpp (2.2), glide/glide.h
	(2.2), globalincs/alphacolors.cpp (2.1), globalincs/crypt.cpp
	(2.1), globalincs/pstypes.h (2.3), globalincs/systemvars.cpp (2.1),
	globalincs/version.cpp (2.1), globalincs/windebug.cpp (2.1),
	graphics/2d.cpp (2.3), graphics/2d.h (2.1), graphics/aaline.cpp
	(2.2), graphics/bitblt.cpp (2.1), graphics/circle.cpp (2.1),
	graphics/colors.cpp (2.1), graphics/font.cpp (2.3), graphics/font.h
	(2.2), graphics/gradient.cpp (2.2), graphics/grd3d.cpp (2.2),
	graphics/grd3dinternal.h (2.1), graphics/grd3drender.cpp (2.1),
	graphics/grd3dtexture.cpp (2.1), graphics/grdirectdraw.cpp (2.1),
	graphics/grglide.cpp (2.1), graphics/grglideinternal.h (2.1),
	graphics/grglidetexture.cpp (2.1), graphics/grinternal.h (2.1),
	graphics/gropengl.cpp (2.3), graphics/grsoft.cpp (2.2),
	graphics/grzbuffer.cpp (2.1), graphics/line.cpp (2.2),
	graphics/pixel.cpp (2.1), graphics/rect.cpp (2.1),
	graphics/scaler.cpp (2.2), graphics/scaler.h (2.1),
	graphics/shade.cpp (2.2), graphics/tmapgenericscans.cpp (2.1),
	graphics/tmapper.cpp (2.2), graphics/tmapscanline.cpp (2.1),
	graphics/tmapscantiled128x128.cpp (2.1),
	graphics/tmapscantiled16x16.cpp (2.1),
	graphics/tmapscantiled256x256.cpp (2.1),
	graphics/tmapscantiled32x32.cpp (2.1),
	graphics/tmapscantiled64x64.cpp (2.1),
	graphics/directx8/GrD3D81.cpp (1.2),
	graphics/directx8/grd3d81internal.h (1.2), hud/hud.cpp (2.1),
	hud/hud.h (2.1), hud/hudartillery.cpp (2.1), hud/hudartillery.h
	(2.1), hud/hudbrackets.cpp (2.1), hud/hudbrackets.h (2.1),
	hud/hudconfig.cpp (2.2), hud/hudconfig.h (2.1), hud/hudescort.cpp
	(2.1), hud/hudets.cpp (2.1), hud/hudets.h (2.1), hud/hudlock.cpp
	(2.1), hud/hudmessage.cpp (2.1), hud/hudobserver.cpp (2.1),
	hud/hudobserver.h (2.1), hud/hudreticle.cpp (2.1), hud/hudreticle.h
	(2.1), hud/hudshield.cpp (2.2), hud/hudsquadmsg.cpp (2.1),
	hud/hudsquadmsg.h (2.1), hud/hudtarget.cpp (2.1), hud/hudtarget.h
	(2.1), hud/hudtargetbox.cpp (2.1), hud/hudwingmanstatus.cpp (2.1),
	inetfile/cftp.cpp (2.1), inetfile/chttpget.cpp (2.2),
	inetfile/inetgetfile.cpp (2.1), inetfile/inetgetfile.h (2.1),
	io/joy.cpp (2.1), io/joy_ff.cpp (2.2), io/key.cpp (2.3), io/key.h
	(2.1), io/keycontrol.cpp (2.1), io/keycontrol.h (2.1), io/mouse.cpp
	(2.3), io/mouse.h (2.2), io/sw_force.h (2.1), io/swff_lib.cpp
	(2.1), io/timer.cpp (2.2), io/timer.h (2.2), io/xmouse.cpp (2.1),
	jumpnode/jumpnode.cpp (2.1), jumpnode/jumpnode.h (2.1),
	lighting/lighting.cpp (2.1), localization/fhash.cpp (2.1),
	localization/localize.cpp (2.1), math/fix.cpp (2.2),
	math/floating.cpp (2.1), math/fvi.cpp (2.1), math/fvi.h (2.1),
	math/spline.cpp (2.1), math/spline.h (2.1), math/staticrand.cpp
	(2.1), math/vecmat.cpp (2.1), math/vecmat.h (2.1),
	menuui/barracks.cpp (2.2), menuui/credits.cpp (2.2),
	menuui/fishtank.cpp (2.1), menuui/mainhallmenu.cpp (2.2),
	menuui/mainhalltemp.cpp (2.1), menuui/optionsmenu.cpp (2.1),
	menuui/optionsmenu.h (2.1), menuui/optionsmenumulti.cpp (2.1),
	menuui/playermenu.cpp (2.1), menuui/readyroom.cpp (2.1),
	menuui/snazzyui.cpp (2.1), menuui/snazzyui.h (2.1),
	menuui/techmenu.cpp (2.1), menuui/trainingmenu.cpp (2.1),
	mission/missionbriefcommon.cpp (2.1), mission/missionbriefcommon.h
	(2.1), mission/missioncampaign.cpp (2.3), mission/missioncampaign.h
	(2.1), mission/missiongoals.cpp (2.1), mission/missiongoals.h
	(2.1), mission/missiongrid.cpp (2.1), mission/missionhotkey.cpp
	(2.1), mission/missionload.cpp (2.1), mission/missionload.h (2.2),
	mission/missionlog.cpp (2.1), mission/missionmessage.cpp (2.1),
	mission/missionmessage.h (2.1), mission/missionparse.cpp (2.1),
	mission/missionparse.h (2.3), mission/missiontraining.cpp (2.1),
	missionui/chatbox.cpp (2.1), missionui/missionbrief.cpp (2.1),
	missionui/missionbrief.h (2.1), missionui/missioncmdbrief.cpp
	(2.1), missionui/missiondebrief.cpp (2.2),
	missionui/missionloopbrief.cpp (2.1), missionui/missionpause.cpp
	(2.1), missionui/missionpause.h (2.1),
	missionui/missionscreencommon.cpp (2.1),
	missionui/missionscreencommon.h (2.1),
	missionui/missionshipchoice.cpp (2.1),
	missionui/missionshipchoice.h (2.1),
	missionui/missionweaponchoice.cpp (2.1),
	missionui/missionweaponchoice.h (2.1), missionui/redalert.cpp
	(2.1), missionui/redalert.h (2.1), model/model.h (2.4),
	model/modelcollide.cpp (2.1), model/modelinterp.cpp (2.3),
	model/modeloctant.cpp (2.1), model/modelread.cpp (2.3),
	model/modelsinc.h (2.3), nebula/neb.cpp (2.1),
	nebula/neblightning.cpp (2.1), network/fs2ox.cpp (1.2),
	network/multi.cpp (2.2), network/multi.h (2.2),
	network/multi_campaign.cpp (2.2), network/multi_data.cpp (2.3),
	network/multi_dogfight.cpp (2.1), network/multi_endgame.cpp (2.2),
	network/multi_ingame.cpp (2.1), network/multi_kick.cpp (2.1),
	network/multi_log.cpp (2.2), network/multi_obj.cpp (2.2),
	network/multi_observer.cpp (2.2), network/multi_oo.cpp (2.1),
	network/multi_oo.h (2.1), network/multi_options.cpp (2.2),
	network/multi_pause.cpp (2.2), network/multi_pinfo.cpp (2.1),
	network/multi_ping.cpp (2.1), network/multi_pmsg.cpp (2.3),
	network/multi_rate.cpp (2.1), network/multi_respawn.cpp (2.2),
	network/multi_team.cpp (2.1), network/multi_update.cpp (2.1),
	network/multi_voice.cpp (2.2), network/multi_xfer.cpp (2.2),
	network/multilag.cpp (2.1), network/multimsgs.cpp (2.2),
	network/multiteamselect.cpp (2.2), network/multiui.cpp (2.4),
	network/multiui.h (2.1), network/multiutil.cpp (2.3),
	network/multiutil.h (2.1), network/psnet.cpp (2.2), network/psnet.h
	(2.1), network/psnet2.cpp (2.3), network/psnet2.h (2.4),
	network/stand_gui.cpp (2.2), object/collidedebrisship.cpp (2.1),
	object/collidedebrisweapon.cpp (2.1), object/collideshipship.cpp
	(2.2), object/collideshipweapon.cpp (2.1),
	object/collideweaponweapon.cpp (2.1), object/objcollide.cpp (2.1),
	object/objcollide.h (2.1), object/object.cpp (2.1), object/object.h
	(2.1), object/objectsnd.cpp (2.2), object/objectsort.cpp (2.1),
	observer/observer.cpp (2.1), observer/observer.h (2.1),
	osapi/osapi.cpp (2.2), osapi/osapi.h (2.2), osapi/osapi_unix.cpp
	(2.2), osapi/osregistry.cpp (2.1), osapi/osregistry_unix.cpp (2.2),
	osapi/outwnd.cpp (2.1), osapi/outwnd_unix.cpp (2.2),
	palman/palman.cpp (2.1), parse/encrypt.cpp (2.1), parse/parselo.cpp
	(2.1), parse/parselo.h (2.2), parse/sexp.cpp (2.2), parse/sexp.h
	(2.2), particle/particle.cpp (2.1), pcxutils/pcxutils.cpp (2.1),
	pcxutils/pcxutils.h (2.1), physics/physics.cpp (2.1),
	physics/physics.h (2.1), playerman/managepilot.cpp (2.2),
	playerman/managepilot.h (2.1), playerman/player.h (2.1),
	playerman/playercontrol.cpp (2.1), popup/popup.cpp (2.1),
	popup/popupdead.cpp (2.1), radar/radar.cpp (2.1), render/3d.h
	(2.1), render/3dclipper.cpp (2.1), render/3ddraw.cpp (2.1),
	render/3dinternal.h (2.1), render/3dlaser.cpp (2.1),
	render/3dmath.cpp (2.1), render/3dsetup.cpp (2.1),
	scramble/scramble.cpp (2.1), ship/afterburner.cpp (2.1),
	ship/afterburner.h (2.1), ship/ai.cpp (2.1), ship/ai.h (2.1),
	ship/aibig.cpp (2.1), ship/aicode.cpp (2.7), ship/aigoals.cpp
	(2.1), ship/aigoals.h (2.1), ship/awacs.cpp (2.1), ship/shield.cpp
	(2.3), ship/ship.cpp (2.6), ship/ship.h (2.3),
	ship/shipcontrails.cpp (2.2), ship/shipfx.cpp (2.1),
	ship/shiphit.cpp (2.5), sound/acm.cpp (2.1), sound/acm.h (2.1),
	sound/audiostr.cpp (2.1), sound/channel.h (2.1), sound/ds.cpp
	(2.1), sound/ds.h (2.1), sound/ds3d.cpp (2.2), sound/dscap.cpp
	(2.1), sound/rbaudio.cpp (2.1), sound/rtvoice.cpp (2.1),
	sound/sound.cpp (2.2), starfield/nebula.cpp (2.1),
	starfield/starfield.cpp (2.1), starfield/starfield.h (2.1),
	starfield/supernova.cpp (2.1), stats/medals.cpp (2.2),
	stats/medals.h (2.1), stats/scoring.cpp (2.1), stats/scoring.h
	(2.1), stats/stats.cpp (2.2), stats/stats.h (2.1),
	tgautils/tgautils.cpp (2.1), ui/button.cpp (2.1), ui/checkbox.cpp
	(2.1), ui/gadget.cpp (2.1), ui/icon.cpp (2.1), ui/inputbox.cpp
	(2.1), ui/keytrap.cpp (2.1), ui/listbox.cpp (2.1), ui/radio.cpp
	(2.1), ui/scroll.cpp (2.1), ui/slider.cpp (2.1), ui/slider2.cpp
	(2.1), ui/ui.h (2.1), ui/uidefs.h (2.1), ui/uidraw.cpp (2.1),
	ui/uimouse.cpp (2.1), ui/window.cpp (2.1), vcodec/codec1.cpp (2.3),
	weapon/beam.cpp (2.3), weapon/beam.h (2.2), weapon/corkscrew.cpp
	(2.1), weapon/corkscrew.h (2.1), weapon/emp.cpp (2.1),
	weapon/flak.cpp (2.1), weapon/muzzleflash.cpp (2.1),
	weapon/shockwave.cpp (2.1), weapon/shockwave.h (2.1),
	weapon/swarm.cpp (2.1), weapon/swarm.h (2.1), weapon/trails.cpp
	(2.1), weapon/trails.h (2.1), weapon/weapon.h (2.1),
	weapon/weapons.cpp (2.1), windows_stub/stubs.cpp (2.4):
	
	The big include file move

2002-07-30 18:11  wmcoolmon

	* ship/ship.cpp (2.5):
	
	Fixed a bug I added in ship_do_rearm_frame

2002-07-30 17:57  wmcoolmon

	* ship/ship.cpp (2.4):
	
	Modified to add hull repair capabilities and toggling of them via a
	mission flag, MISSION_FLAG_SUPPORT_REPAIRS_HULL

2002-07-30 17:35  wmcoolmon

	* mission/missionparse.h (2.2):
	
	Added mission flag "MISSION_FLAG_SUPPORT_REPAIRS_HULL" for toggling
	Support Ship hull repair on and off

2002-07-30 14:29  unknownplayer

	* code.dsp (1.3), graphics/2d.cpp (2.2), graphics/grd3d.cpp (2.1),
	graphics/directx8/GrD3D81.cpp (1.1), graphics/directx8/GrD3D81.h
	(1.1), graphics/directx8/GrD3D81Render.cpp (1.1),
	graphics/directx8/GrD3D81Texture.cpp (1.1),
	graphics/directx8/grd3d81internal.h (1.1), network/psnet2.cpp
	(2.2):
		
	Started work on DX8.1 implementation. Updated the project files to
	encompass the new files. Disable the compiler tag to use old DX
	code (THERE IS NO NEW CODE YET!)

2002-07-29 22:24  penguin

	* network/: fs2ox.cpp (1.1), fs2ox.h (1.1):
	
	First attempt at "fs2_open exchange" (PXO substitute) screen

2002-07-29 22:14  penguin

	* ChangeLog (1.1):
	
	We finally have a ChangeLog

2002-07-29 20:48  penguin

	* Ship/shiphit.cpp (2.4):
	
	Moved extern declaration of ssm_create outside of block (it
	wouldn't compile w/ gcc)

2002-07-29 20:12  penguin

	* FREESPACE2/freespace.cpp (2.3), Graphics/font.cpp (2.2),
	Graphics/gradient.cpp (2.1), Graphics/line.cpp (2.1), Io/key.cpp
	(2.2), Io/mouse.cpp (2.2), Math/fix.cpp (2.1),
	Mission/missioncampaign.cpp (2.2), Stats/stats.cpp (2.1),
	VCodec/codec1.cpp (2.2):
	
	added #ifdef _WIN32 around windows-specific system headers

2002-07-29 20:12  penguin

	* OsApi/: osregistry_unix.cpp (2.1), outwnd_unix.cpp (2.1):
	
	removed bogus #include windows.h

2002-07-29 19:52  penguin

	* CFile/cfilesystem.cpp (2.2):
	
	added #ifdef _WIN32 around windows-specific system headers

2002-07-29 19:37  penguin

	* CFile/cfilelist.cpp (2.1):
	
	added #ifdef _WIN32 around windows-specific system headers

2002-07-29 19:17  penguin

	* CFile/cfilearchive.cpp (2.1):
	
	added #ifdef _WIN32 around windows-specific system headers

2002-07-29 19:04  penguin

	* CFile/cfile.cpp (2.2):
	
	added #ifdef _WIN32 around windows-specific system headers

2002-07-29 08:31  DTP

	* Model/model.h (2.3):
	
	Forgot to Bump MAX_MODEL_SUBSYSTEMS from 128 to 200

2002-07-29 08:28  DTP

	* Model/model.h (2.2):
	
	BUMPED MAX_POLYGON_MODELS TO 198 , MAX_SHIP_TYPES - 2 = 198

2002-07-29 08:24  DTP

	* Ship/ship.h (2.2):
	
	Bumped all MAX_SHIPS, and SHIP_LIMIT to 400.(let the mission
	designers decide what is good, and what is bad

2002-07-29 08:22  DTP

	* Ship/ship.cpp (2.3):
	
	Bumped MAX_SHIP_SUBOBJECTS to 2100(2100 /(average fighter has 7
	subsystems) = 400 fighters

2002-07-29 08:19  DTP

	* Ship/shiphit.cpp (2.3):
	
	Bumped MAX_SUBSYS_LIST from 32 to 200

2002-07-29 08:05  DTP

	* Network/multiui.cpp (2.3):
	
	-almission(autoload mission) commandline arguement handling
	implemented

2002-07-29 07:56  DTP

	* MenuUI/mainhallmenu.cpp (2.1):
	
	FIX; Startgame arguement dont go ahead when a singleplayer pilot is
	selected

2002-07-29 06:35  DTP

	* Cmdline/: cmdline.cpp (2.2), cmdline.h (2.2):
	
	added -almission commandline arguement, will autoload mission i.e
	fs2_open.exe -almission kickass will autoload kickass.fs2 which
	should be a multiplayer mission.

2002-07-28 15:07  DTP

	* Network/multiui.cpp (2.2):
	
	FIX -stargame commandline arguement will now let you quit, a thing
	not possible before when using the startgame arguement

2002-07-26 16:17  penguin

	* Object/collideshipship.cpp (2.1):
	
	renamed 'big' and 'small' (conflict w/ MS include file)

2002-07-26 16:12  penguin

	* Network/psnet2.h (2.3):
	
	fixed bug where winsock.h wasn't defined

2002-07-26 03:11  wmcoolmon

	* Weapon/beam.cpp (2.2):
	
	Added Bobboau's beam tiling code

2002-07-25 04:50  wmcoolmon

	* Ship/shield.cpp (2.2), Ship/ship.cpp (2.2), Weapon/beam.cpp
	(2.1), Weapon/beam.h (2.1):
	
	Added Bobboau's fighter-beam code.

2002-07-22 01:39  penguin

	* FREESPACE2/freespace.cpp (2.2):
	
	Added ifndef NO_STANDALONE

2002-07-22 01:37  penguin

	* Sound/rtvoice.h (2.1):
	
	Stub defines for NO_SOUND

2002-07-22 01:22  penguin

	* Network/: multi.cpp (2.1), multi.h (2.1), multi_campaign.cpp
	(2.1), multi_data.cpp (2.2), multi_endgame.cpp (2.1),
	multi_options.cpp (2.1), multi_pause.cpp (2.1), multi_pmsg.cpp
	(2.2), multi_voice.cpp (2.1), multimsgs.cpp (2.1), multiui.cpp
	(2.1), multiutil.cpp (2.2), psnet.cpp (2.1), psnet2.cpp (2.1),
	psnet2.h (2.2), stand_gui.cpp (2.1), stand_gui.h (2.1):
	
	Linux port -- added NO_STANDALONE ifdefs

2002-07-22 01:14  penguin

	* Makefile (2.2):
	
	Made more compatible w/ Win32; defines for NO_SOUND, NO_JOYSTICK,
	NO_NETWORK

2002-07-22 01:10  penguin

	* windows_stub/stubs.cpp (2.3):
	
	Moved the dummy "sound" definitions to main program (freespace.cpp)

2002-07-22 01:06  penguin

	* windows_stub/config.h (2.2):
	
	More defines for winsock compatibility

2002-07-22 01:04  penguin

	* GlobalIncs/pstypes.h (2.2):
	
	took out GAME_CD_CHECK define

2002-07-20 23:51  DTP

	* Hud/hudshield.cpp (2.1):
	
	Bumped Max Shield Icons to 80

2002-07-20 23:50  DTP

	* MissionUI/missiondebrief.cpp (2.1):
	
	Fixed multiplayer music. succes music on all Primary goals
	complete, fail music if otherwise

2002-07-20 23:49  DTP

	* Ship/ship.cpp (2.1):
	
	Fixed Secondary bank bug, where next valid secondary bank inherits
	current valid banks FULL fire delay

2002-07-20 19:21  DTP

	* Parse/parselo.h (2.1):
	
	bumped MAX_MISSION_TEXT to 1000000 in code/parse/parselo.h

2002-07-18 20:07  penguin

	* Hud/hudconfig.cpp (2.1):
	
	Fixed bug (potential access of unitialized pointer) in
	hud_config_render_gauges() -- thanks to DTP for finding it!

2002-07-18 14:36  unknownplayer

	* Ship/aicode.cpp (2.6):
	
	no message

2002-07-18 09:55  unknownplayer

	* Ship/aicode.cpp (2.5):
	
	
	Revised AI primary selection code again to be more intelligent.

2002-07-18 05:38  unknownplayer

	* Ship/aicode.cpp (2.4):
	
	
	Rewrote my original algorithm to search by proportional hull
	damage.  Does the same thing a little better.

2002-07-18 03:27  unknownplayer

	* Ship/aicode.cpp (2.3):
	
	
	Fixed AI problems with using the maxim on fighters.

2002-07-18 03:26  unknownplayer

	* Ship/shield.cpp (2.1):
	
	
	Added some commentry to the low detail shield function.

2002-07-18 03:25  unknownplayer

	* Ship/shiphit.cpp (2.2):
	
	no message

2002-07-17 23:59  unknownplayer

	* code.dsp (1.2):
	
	
	Altered project files to properly build debug info. Should save you
	all some time.

2002-07-17 23:58  unknownplayer

	* Ship/aicode.cpp (2.2):
	
	
	Minor commenting added to two functions.

2002-07-17 20:04  wmcoolmon

	* Ship/shiphit.cpp (2.1):
	
	Added SSM code for Tag C from Bobboau. Note that strings.tbl may
	need to be updated.

2002-07-16 14:39  unknownplayer

	* Graphics/gropengl.cpp (2.2):
	
	Updated to enable compilation under MSVC++ - all changes are marked
	with my name ##UnknownPlayer##

2002-07-16 14:37  unknownplayer

	* code.dsp (1.1):
	
	This is the code project file. Use the workspace file I included to
	get at this.

2002-07-15 02:09  wmcoolmon

	* Mission/missionparse.h (2.1), Ship/shipcontrails.cpp (2.1):
	
	Added support for toggling ship trails

2002-07-13 09:16  wmcoolmon

	* Parse/: sexp.cpp (2.1), sexp.h (2.1):
	
	Added initial code for "ship-lights-on" and "ship-lights-off" SEXPs

2002-07-12 16:59  penguin

	* Ship/ship.h (2.1):
	
	Added flags2 (ran out of bits in flags!) to ship struct; bit 0 will
	be used to toggle Bobboau's lights.

2002-07-12 02:56  penguin

	* windows_stub/: commctrl.h (2.1), conio.h (2.1), crtdbg.h (2.1),
	d3dcom.h (2.1), direct.h (2.1), initguid.h (2.1), io.h (2.1),
	mmreg.h (2.1), mmsystem.h (2.1), msacm.h (2.1), objbase.h (2.1),
	ole2.h (2.1), ole2ver.h (2.1), process.h (2.1), ras.h (2.1),
	raserror.h (2.1), subwtype.h (2.1), winbase.h (2.1), windows.h
	(2.1), windowsx.h (2.1), winerror.h (2.1), winioctl.h (2.1),
	winsock.h (2.1), wsipx.h (2.1):
	
	Removed dummy include file

2002-07-12 02:44  penguin

	* Model/modelsinc.h (2.2):
	
	Fixed endian-ness of new "GLOW" IDs

2002-07-10 18:42  wmcoolmon

	* Model/: modelsinc.h (2.1), model.h (2.1), modelinterp.cpp (2.2),
	modelread.cpp (2.2):
	
	Added  Bobboau's glow code; all comments include "-Bobboau"

2002-07-07 19:55  penguin

	* Makefile (2.1), Bmpman/bmpman.cpp (2.1), CFile/cfile.cpp (2.1),
	CFile/cfile.h (2.1), CFile/cfilearchive.h (2.1),
	CFile/cfilesystem.cpp (2.1), Cmdline/cmdline.cpp (2.1),
	Cmdline/cmdline.h (2.1), Cutscene/cutscenes.cpp (2.1),
	Demo/demo.cpp (2.1), DirectX/vd3di.h (2.1), DirectX/vd3drmdef.h
	(2.1), DirectX/vd3drmwin.h (2.1), DirectX/vd3dtypes.h (2.1),
	ExceptionHandler/exceptionhandler.h (2.1), FREESPACE2/freespace.cpp
	(2.1), Glide/3dfx.h (2.1), Glide/fxdll.h (2.1), Glide/fxos.h (2.1),
	Glide/glide.cpp (2.1), Glide/glide.h (2.1), GlobalIncs/pstypes.h
	(2.1), Graphics/2d.cpp (2.1), Graphics/aaline.cpp (2.1),
	Graphics/font.cpp (2.1), Graphics/font.h (2.1),
	Graphics/gropengl.cpp (2.1), Graphics/grsoft.cpp (2.1),
	Graphics/scaler.cpp (2.1), Graphics/shade.cpp (2.1),
	Graphics/tmapper.cpp (2.1), Inetfile/chttpget.cpp (2.1),
	Io/joy_ff.cpp (2.1), Io/key.cpp (2.1), Io/mouse.cpp (2.1),
	Io/mouse.h (2.1), Io/timer.cpp (2.1), Io/timer.h (2.1),
	MenuUI/barracks.cpp (2.1), MenuUI/credits.cpp (2.1),
	Mission/missioncampaign.cpp (2.1), Mission/missionload.h (2.1),
	Model/modelinterp.cpp (2.1), Model/modelread.cpp (2.1),
	Network/multi_data.cpp (2.1), Network/multi_log.cpp (2.1),
	Network/multi_obj.cpp (2.1), Network/multi_observer.cpp (2.1),
	Network/multi_pmsg.cpp (2.1), Network/multi_respawn.cpp (2.1),
	Network/multi_xfer.cpp (2.1), Network/multiteamselect.cpp (2.1),
	Network/multiutil.cpp (2.1), Network/psnet2.h (2.1),
	Object/objectsnd.cpp (2.1), OsApi/osapi.cpp (2.1), OsApi/osapi.h
	(2.1), OsApi/osapi_unix.cpp (2.1), Playerman/managepilot.cpp (2.1),
	Ship/aicode.cpp (2.1), Sound/ds3d.cpp (2.1), Sound/sound.cpp (2.1),
	Stats/medals.cpp (2.1), VCodec/codec1.cpp (2.1),
	windows_stub/config.h (2.1), windows_stub/stubs.cpp (2.2):
	
	Back-port to MSVC

2002-06-04 06:38  penguin

	* windows_stub/stubs.cpp (2.1):
	
	Added machine-independent version of MulDiv (in case we don't know
	the processor type)

2002-06-03 04:10  penguin

	* Io/xmouse.cpp (2.0), OsApi/osapi_unix.cpp (2.0),
	OsApi/osregistry_unix.cpp (2.0), OsApi/outwnd_unix.cpp (2.0)
	(utags: fs2_open_0_1):
	
	Warpcore CVS sync

2002-06-03 04:08  penguin

	* windows_stub/: commctrl.h (2.0), conio.h (2.0), crtdbg.h (2.0),
	d3dcom.h (2.0), direct.h (2.0), initguid.h (2.0), io.h (2.0),
	mmreg.h (2.0), mmsystem.h (2.0), msacm.h (2.0), objbase.h (2.0),
	ole2.h (2.0), ole2ver.h (2.0), process.h (2.0), ras.h (2.0),
	raserror.h (2.0), subwtype.h (2.0), winbase.h (2.0), windowsx.h
	(2.0), winerror.h (2.0), winioctl.h (2.0), winsock.h (2.0), wsipx.h
	(2.0) (utags: fs2_open_0_1):
	
	Dummy include file

2002-06-03 04:07  penguin

	* windows_stub/: config.h (2.0), stubs.cpp (2.0), windows.h (2.0)
	(utags: fs2_open_0_1):
	
	Warpcore CVS sync

2002-06-03 04:02  penguin

	* Makefile (2.0), Anim/animplay.cpp (2.0), Anim/animplay.h (2.0),
	Anim/packunpack.cpp (2.0), Anim/packunpack.h (2.0),
	Asteroid/asteroid.cpp (2.0), Asteroid/asteroid.h (2.0),
	Bmpman/bmpman.cpp (2.0), Bmpman/bmpman.h (2.0), CFile/cfile.cpp
	(2.0), CFile/cfile.h (2.0), CFile/cfilearchive.cpp (2.0),
	CFile/cfilearchive.h (2.0), CFile/cfilelist.cpp (2.0),
	CFile/cfilesystem.cpp (2.0), CFile/cfilesystem.h (2.0),
	CMeasure/cmeasure.cpp (2.0), CMeasure/cmeasure.h (2.0),
	Cfilearchiver/cfilearchiver.cpp (2.0), Cmdline/cmdline.cpp (2.0),
	Cmdline/cmdline.h (2.0), ControlConfig/controlsconfig.cpp (2.0),
	ControlConfig/controlsconfig.h (2.0),
	ControlConfig/controlsconfigcommon.cpp (2.0),
	Cryptstring/cryptstring.cpp (2.0), Cutscene/cutscenes.cpp (2.0),
	Cutscene/cutscenes.h (2.0), Debris/debris.cpp (2.0),
	Debris/debris.h (2.0), DebugConsole/console.cpp (2.0),
	Demo/demo.cpp (2.0), Demo/demo.h (2.0), DirectX/vasync.h (2.0),
	DirectX/vd3d.h (2.0), DirectX/vd3dcaps.h (2.0), DirectX/vd3di.h
	(2.0), DirectX/vd3drm.h (2.0), DirectX/vd3drmdef.h (2.0),
	DirectX/vd3drmobj.h (2.0), DirectX/vd3drmwin.h (2.0),
	DirectX/vd3dtypes.h (2.0), DirectX/vd3dvec.inl (2.0),
	DirectX/vddraw.h (2.0), DirectX/vddraw.lib (2.0), DirectX/vdinput.h
	(2.0), DirectX/vdinput.lib (2.0), DirectX/vdplay.h (2.0),
	DirectX/vdplobby.h (2.0), DirectX/vdsetup.h (2.0),
	DirectX/vdsound.h (2.0), DirectX/vdsound.lib (2.0), DirectX/vdvp.h
	(2.0), DirectX/vdxguid.lib (2.0),
	ExceptionHandler/exceptionhandler.cpp (2.0),
	ExceptionHandler/exceptionhandler.h (2.0), FREESPACE2/app_icon.ico
	(2.0), FREESPACE2/freespace.cpp (2.0), FREESPACE2/freespace.h
	(2.0), FREESPACE2/freespace.rc (2.0),
	FREESPACE2/freespaceresource.h (2.0), FREESPACE2/goal_com.bmp
	(2.0), FREESPACE2/goal_fail.bmp (2.0), FREESPACE2/goal_inc.bmp
	(2.0), FREESPACE2/goal_none.bmp (2.0), FREESPACE2/goal_ord.bmp
	(2.0), FREESPACE2/levelpaging.cpp (2.0), FREESPACE2/levelpaging.h
	(2.0), Fireball/fireballs.cpp (2.0), Fireball/fireballs.h (2.0),
	Fireball/warpineffect.cpp (2.0), GameHelp/contexthelp.cpp (2.0),
	GameHelp/contexthelp.h (2.0), GameHelp/gameplayhelp.cpp (2.0),
	GameHelp/gameplayhelp.h (2.0), GameSequence/gamesequence.cpp (2.0),
	GameSequence/gamesequence.h (2.0), Gamesnd/eventmusic.cpp (2.0),
	Gamesnd/eventmusic.h (2.0), Gamesnd/gamesnd.cpp (2.0),
	Gamesnd/gamesnd.h (2.0), Glide/3dfx.h (2.0), Glide/fxdll.h (2.0),
	Glide/fxglob.h (2.0), Glide/fxos.h (2.0), Glide/glide.cpp (2.0),
	Glide/glide.h (2.0), Glide/glidesys.h (2.0), Glide/glideutl.h
	(2.0), Glide/sst1vid.h (2.0), GlobalIncs/alphacolors.cpp (2.0),
	GlobalIncs/alphacolors.h (2.0), GlobalIncs/crypt.cpp (2.0),
	GlobalIncs/crypt.h (2.0), GlobalIncs/linklist.h (2.0),
	GlobalIncs/pstypes.h (2.0), GlobalIncs/systemvars.cpp (2.0),
	GlobalIncs/systemvars.h (2.0), GlobalIncs/version.cpp (2.0),
	GlobalIncs/version.h (2.0), GlobalIncs/windebug.cpp (2.0),
	Graphics/2d.cpp (2.0), Graphics/2d.h (2.0), Graphics/aaline.cpp
	(2.0), Graphics/bitblt.cpp (2.0), Graphics/bitblt.h (2.0),
	Graphics/circle.cpp (2.0), Graphics/circle.h (2.0),
	Graphics/colors.cpp (2.0), Graphics/colors.h (2.0),
	Graphics/font.cpp (2.0), Graphics/font.h (2.0),
	Graphics/gradient.cpp (2.0), Graphics/gradient.h (2.0),
	Graphics/grd3d.cpp (2.0), Graphics/grd3d.h (2.0),
	Graphics/grd3dinternal.h (2.0), Graphics/grd3drender.cpp (2.0),
	Graphics/grd3dtexture.cpp (2.0), Graphics/grdirectdraw.cpp (2.0),
	Graphics/grdirectdraw.h (2.0), Graphics/grglide.cpp (2.0),
	Graphics/grglide.h (2.0), Graphics/grglideinternal.h (2.0),
	Graphics/grglidetexture.cpp (2.0), Graphics/grinternal.h (2.0),
	Graphics/gropengl.h (2.0), Graphics/grsoft.cpp (2.0),
	Graphics/grsoft.h (2.0), Graphics/grzbuffer.cpp (2.0),
	Graphics/grzbuffer.h (2.0), Graphics/line.cpp (2.0),
	Graphics/line.h (2.0), Graphics/pixel.cpp (2.0), Graphics/pixel.h
	(2.0), Graphics/rect.cpp (2.0), Graphics/rect.h (2.0),
	Graphics/scaler.cpp (2.0), Graphics/scaler.h (2.0),
	Graphics/shade.cpp (2.0), Graphics/shade.h (2.0),
	Graphics/tmapgenericscans.cpp (2.0), Graphics/tmapper.cpp (2.0),
	Graphics/tmapper.h (2.0), Graphics/tmapscanline.cpp (2.0),
	Graphics/tmapscanline.h (2.0), Graphics/tmapscantiled128x128.cpp
	(2.0), Graphics/tmapscantiled16x16.cpp (2.0),
	Graphics/tmapscantiled256x256.cpp (2.0),
	Graphics/tmapscantiled32x32.cpp (2.0),
	Graphics/tmapscantiled64x64.cpp (2.0), Hud/hud.cpp (2.0), Hud/hud.h
	(2.0), Hud/hudartillery.cpp (2.0), Hud/hudartillery.h (2.0),
	Hud/hudbrackets.cpp (2.0), Hud/hudbrackets.h (2.0),
	Hud/hudconfig.cpp (2.0), Hud/hudconfig.h (2.0), Hud/hudescort.cpp
	(2.0), Hud/hudescort.h (2.0), Hud/hudets.cpp (2.0), Hud/hudets.h
	(2.0), Hud/hudgauges.h (2.0), Hud/hudlock.cpp (2.0), Hud/hudlock.h
	(2.0), Hud/hudmessage.cpp (2.0), Hud/hudmessage.h (2.0),
	Hud/hudobserver.cpp (2.0), Hud/hudobserver.h (2.0),
	Hud/hudresource.cpp (2.0), Hud/hudresource.h (2.0),
	Hud/hudreticle.cpp (2.0), Hud/hudreticle.h (2.0), Hud/hudshield.cpp
	(2.0), Hud/hudshield.h (2.0), Hud/hudsquadmsg.cpp (2.0),
	Hud/hudsquadmsg.h (2.0), Hud/hudtarget.cpp (2.0), Hud/hudtarget.h
	(2.0), Hud/hudtargetbox.cpp (2.0), Hud/hudtargetbox.h (2.0),
	Hud/hudwingmanstatus.cpp (2.0), Hud/hudwingmanstatus.h (2.0),
	Inetfile/cftp.cpp (2.0), Inetfile/cftp.h (2.0),
	Inetfile/chttpget.cpp (2.0), Inetfile/chttpget.h (2.0),
	Inetfile/inetgetfile.cpp (2.0), Inetfile/inetgetfile.h (2.0),
	Io/joy.cpp (2.0), Io/joy.h (2.0), Io/joy_ff.cpp (2.0), Io/joy_ff.h
	(2.0), Io/key.cpp (2.0), Io/key.h (2.0), Io/keycontrol.cpp (2.0),
	Io/keycontrol.h (2.0), Io/mouse.cpp (2.0), Io/mouse.h (2.0),
	Io/sw_error.hpp (2.0), Io/sw_force.h (2.0), Io/sw_guid.hpp (2.0),
	Io/swff_lib.cpp (2.0), Io/timer.cpp (2.0), Io/timer.h (2.0),
	JumpNode/jumpnode.cpp (2.0), JumpNode/jumpnode.h (2.0),
	Lighting/lighting.cpp (2.0), Lighting/lighting.h (2.0),
	Localization/fhash.cpp (2.0), Localization/fhash.h (2.0),
	Localization/localize.cpp (2.0), Localization/localize.h (2.0),
	Math/fix.cpp (2.0), Math/fix.h (2.0), Math/floating.cpp (2.0),
	Math/floating.h (2.0), Math/fvi.cpp (2.0), Math/fvi.h (2.0),
	Math/spline.cpp (2.0), Math/spline.h (2.0), Math/staticrand.cpp
	(2.0), Math/staticrand.h (2.0), Math/vecmat.cpp (2.0),
	Math/vecmat.h (2.0), MenuUI/barracks.cpp (2.0), MenuUI/barracks.h
	(2.0), MenuUI/credits.cpp (2.0), MenuUI/credits.h (2.0),
	MenuUI/fishtank.cpp (2.0), MenuUI/fishtank.h (2.0),
	MenuUI/mainhallmenu.cpp (2.0), MenuUI/mainhallmenu.h (2.0),
	MenuUI/mainhalltemp.cpp (2.0), MenuUI/mainhalltemp.h (2.0),
	MenuUI/optionsmenu.cpp (2.0), MenuUI/optionsmenu.h (2.0),
	MenuUI/optionsmenumulti.cpp (2.0), MenuUI/optionsmenumulti.h (2.0),
	MenuUI/playermenu.cpp (2.0), MenuUI/playermenu.h (2.0),
	MenuUI/readyroom.cpp (2.0), MenuUI/readyroom.h (2.0),
	MenuUI/snazzyui.cpp (2.0), MenuUI/snazzyui.h (2.0),
	MenuUI/techmenu.cpp (2.0), MenuUI/techmenu.h (2.0),
	MenuUI/trainingmenu.cpp (2.0), MenuUI/trainingmenu.h (2.0),
	Mission/missionbriefcommon.cpp (2.0), Mission/missionbriefcommon.h
	(2.0), Mission/missioncampaign.cpp (2.0), Mission/missioncampaign.h
	(2.0), Mission/missiongoals.cpp (2.0), Mission/missiongoals.h
	(2.0), Mission/missiongrid.cpp (2.0), Mission/missiongrid.h (2.0),
	Mission/missionhotkey.cpp (2.0), Mission/missionhotkey.h (2.0),
	Mission/missionload.cpp (2.0), Mission/missionload.h (2.0),
	Mission/missionlog.cpp (2.0), Mission/missionlog.h (2.0),
	Mission/missionmessage.cpp (2.0), Mission/missionmessage.h (2.0),
	Mission/missionparse.cpp (2.0), Mission/missionparse.h (2.0),
	Mission/missiontraining.cpp (2.0), Mission/missiontraining.h (2.0),
	MissionUI/chatbox.cpp (2.0), MissionUI/chatbox.h (2.0),
	MissionUI/missionbrief.cpp (2.0), MissionUI/missionbrief.h (2.0),
	MissionUI/missioncmdbrief.cpp (2.0), MissionUI/missioncmdbrief.h
	(2.0), MissionUI/missiondebrief.cpp (2.0),
	MissionUI/missiondebrief.h (2.0), MissionUI/missionloopbrief.cpp
	(2.0), MissionUI/missionloopbrief.h (2.0),
	MissionUI/missionpause.cpp (2.0), MissionUI/missionpause.h (2.0),
	MissionUI/missionrecommend.cpp (2.0), MissionUI/missionrecommend.h
	(2.0), MissionUI/missionscreencommon.cpp (2.0),
	MissionUI/missionscreencommon.h (2.0),
	MissionUI/missionshipchoice.cpp (2.0),
	MissionUI/missionshipchoice.h (2.0), MissionUI/missionstats.cpp
	(2.0), MissionUI/missionstats.h (2.0),
	MissionUI/missionweaponchoice.cpp (2.0),
	MissionUI/missionweaponchoice.h (2.0), MissionUI/redalert.cpp
	(2.0), MissionUI/redalert.h (2.0), Model/model.h (2.0),
	Model/modelcollide.cpp (2.0), Model/modelinterp.cpp (2.0),
	Model/modeloctant.cpp (2.0), Model/modelread.cpp (2.0),
	Model/modelsinc.h (2.0), Nebula/neb.cpp (2.0), Nebula/neb.h (2.0),
	Nebula/neblightning.cpp (2.0), Nebula/neblightning.h (2.0),
	Network/multi.cpp (2.0), Network/multi.h (2.0),
	Network/multi_campaign.cpp (2.0), Network/multi_campaign.h (2.0),
	Network/multi_data.cpp (2.0), Network/multi_data.h (2.0),
	Network/multi_dogfight.cpp (2.0), Network/multi_dogfight.h (2.0),
	Network/multi_endgame.cpp (2.0), Network/multi_endgame.h (2.0),
	Network/multi_ingame.cpp (2.0), Network/multi_ingame.h (2.0),
	Network/multi_kick.cpp (2.0), Network/multi_kick.h (2.0),
	Network/multi_log.cpp (2.0), Network/multi_log.h (2.0),
	Network/multi_obj.cpp (2.0), Network/multi_obj.h (2.0),
	Network/multi_observer.cpp (2.0), Network/multi_observer.h (2.0),
	Network/multi_oo.cpp (2.0), Network/multi_oo.h (2.0),
	Network/multi_options.cpp (2.0), Network/multi_options.h (2.0),
	Network/multi_pause.cpp (2.0), Network/multi_pause.h (2.0),
	Network/multi_pinfo.cpp (2.0), Network/multi_pinfo.h (2.0),
	Network/multi_ping.cpp (2.0), Network/multi_ping.h (2.0),
	Network/multi_pmsg.cpp (2.0), Network/multi_pmsg.h (2.0),
	Network/multi_rate.cpp (2.0), Network/multi_rate.h (2.0),
	Network/multi_respawn.cpp (2.0), Network/multi_respawn.h (2.0),
	Network/multi_team.cpp (2.0), Network/multi_team.h (2.0),
	Network/multi_update.cpp (2.0), Network/multi_update.h (2.0),
	Network/multi_voice.cpp (2.0), Network/multi_voice.h (2.0),
	Network/multi_xfer.cpp (2.0), Network/multi_xfer.h (2.0),
	Network/multilag.cpp (2.0), Network/multilag.h (2.0),
	Network/multimsgs.cpp (2.0), Network/multimsgs.h (2.0),
	Network/multiteamselect.cpp (2.0), Network/multiteamselect.h (2.0),
	Network/multiui.cpp (2.0), Network/multiui.h (2.0),
	Network/multiutil.cpp (2.0), Network/multiutil.h (2.0),
	Network/psnet.cpp (2.0), Network/psnet.h (2.0), Network/psnet2.cpp
	(2.0), Network/psnet2.h (2.0), Network/stand_gui.cpp (2.0),
	Network/stand_gui.h (2.0), Object/collidedebrisship.cpp (2.0),
	Object/collidedebrisweapon.cpp (2.0), Object/collideshipship.cpp
	(2.0), Object/collideshipweapon.cpp (2.0),
	Object/collideweaponweapon.cpp (2.0), Object/objcollide.cpp (2.0),
	Object/objcollide.h (2.0), Object/object.cpp (2.0), Object/object.h
	(2.0), Object/objectsnd.cpp (2.0), Object/objectsnd.h (2.0),
	Object/objectsort.cpp (2.0), Observer/observer.cpp (2.0),
	Observer/observer.h (2.0), OsApi/monopub.h (2.0), OsApi/osapi.cpp
	(2.0), OsApi/osapi.h (2.0), OsApi/osregistry.cpp (2.0),
	OsApi/osregistry.h (2.0), OsApi/outwnd.cpp (2.0), OsApi/outwnd.h
	(2.0), Palman/palman.cpp (2.0), Palman/palman.h (2.0),
	Parse/encrypt.cpp (2.0), Parse/encrypt.h (2.0), Parse/parselo.cpp
	(2.0), Parse/parselo.h (2.0), Parse/sexp.cpp (2.0), Parse/sexp.h
	(2.0), Particle/particle.cpp (2.0), Particle/particle.h (2.0),
	PcxUtils/pcxutils.cpp (2.0), PcxUtils/pcxutils.h (2.0),
	Physics/physics.cpp (2.0), Physics/physics.h (2.0),
	Playerman/managepilot.cpp (2.0), Playerman/managepilot.h (2.0),
	Playerman/player.h (2.0), Playerman/playercontrol.cpp (2.0),
	Popup/popup.cpp (2.0), Popup/popup.h (2.0), Popup/popupdead.cpp
	(2.0), Popup/popupdead.h (2.0), Radar/radar.cpp (2.0),
	Radar/radar.h (2.0), Render/3d.h (2.0), Render/3dclipper.cpp (2.0),
	Render/3ddraw.cpp (2.0), Render/3dinternal.h (2.0),
	Render/3dlaser.cpp (2.0), Render/3dmath.cpp (2.0),
	Render/3dsetup.cpp (2.0), Scramble/scramble.cpp (2.0),
	Scramble/scramble.h (2.0), Ship/afterburner.cpp (2.0),
	Ship/afterburner.h (2.0), Ship/ai.cpp (2.0), Ship/ai.h (2.0),
	Ship/aibig.cpp (2.0), Ship/aibig.h (2.0), Ship/aicode.cpp (2.0),
	Ship/aigoals.cpp (2.0), Ship/aigoals.h (2.0), Ship/ailocal.h (2.0),
	Ship/awacs.cpp (2.0), Ship/awacs.h (2.0), Ship/shield.cpp (2.0),
	Ship/ship.cpp (2.0), Ship/ship.h (2.0), Ship/shipcontrails.cpp
	(2.0), Ship/shipcontrails.h (2.0), Ship/shipfx.cpp (2.0),
	Ship/shipfx.h (2.0), Ship/shiphit.cpp (2.0), Ship/shiphit.h (2.0),
	Ship/subsysdamage.h (2.0), Sound/acm.cpp (2.0), Sound/acm.h (2.0),
	Sound/audiostr.cpp (2.0), Sound/audiostr.h (2.0), Sound/channel.h
	(2.0), Sound/ds.cpp (2.0), Sound/ds.h (2.0), Sound/ds3d.cpp (2.0),
	Sound/ds3d.h (2.0), Sound/dscap.cpp (2.0), Sound/dscap.h (2.0),
	Sound/midifile.cpp (2.0), Sound/midifile.h (2.0), Sound/midiseq.h
	(2.0), Sound/rbaudio.cpp (2.0), Sound/rbaudio.h (2.0),
	Sound/rtvoice.cpp (2.0), Sound/rtvoice.h (2.0), Sound/sound.cpp
	(2.0), Sound/sound.h (2.0), Sound/winmidi.cpp (2.0),
	Sound/winmidi.h (2.0), Sound/winmidi_base.cpp (2.0),
	Starfield/nebula.cpp (2.0), Starfield/nebula.h (2.0),
	Starfield/starfield.cpp (2.0), Starfield/starfield.h (2.0),
	Starfield/supernova.cpp (2.0), Starfield/supernova.h (2.0),
	Stats/medals.cpp (2.0), Stats/medals.h (2.0), Stats/scoring.cpp
	(2.0), Stats/scoring.h (2.0), Stats/stats.cpp (2.0), Stats/stats.h
	(2.0), TgaUtils/tgautils.cpp (2.0), TgaUtils/tgautils.h (2.0),
	UI/button.cpp (2.0), UI/checkbox.cpp (2.0), UI/gadget.cpp (2.0),
	UI/icon.cpp (2.0), UI/inputbox.cpp (2.0), UI/keytrap.cpp (2.0),
	UI/listbox.cpp (2.0), UI/radio.cpp (2.0), UI/scroll.cpp (2.0),
	UI/slider.cpp (2.0), UI/slider2.cpp (2.0), UI/ui.h (2.0),
	UI/uidefs.h (2.0), UI/uidraw.cpp (2.0), UI/uimouse.cpp (2.0),
	UI/window.cpp (2.0), VCodec/codec1.cpp (2.0), VCodec/codec1.h
	(2.0), Weapon/beam.cpp (2.0), Weapon/beam.h (2.0),
	Weapon/corkscrew.cpp (2.0), Weapon/corkscrew.h (2.0),
	Weapon/emp.cpp (2.0), Weapon/emp.h (2.0), Weapon/flak.cpp (2.0),
	Weapon/flak.h (2.0), Weapon/muzzleflash.cpp (2.0),
	Weapon/muzzleflash.h (2.0), Weapon/shockwave.cpp (2.0),
	Weapon/shockwave.h (2.0), Weapon/swarm.cpp (2.0), Weapon/swarm.h
	(2.0), Weapon/trails.cpp (2.0), Weapon/trails.h (2.0),
	Weapon/weapon.h (2.0), Weapon/weapons.cpp (2.0) (utags:
	fs2_open_0_1):
	
	Warpcore CVS sync

2002-06-03 03:55  penguin

	* Graphics/gropengl.cpp (2.0, fs2_open_0_1):
	
	Warpcore CVS sync

2002-06-03 03:25  penguin

	* Makefile (1.1), Anim/animplay.cpp (1.1), Anim/animplay.h (1.1),
	Anim/packunpack.cpp (1.1), Anim/packunpack.h (1.1),
	Asteroid/asteroid.cpp (1.1), Asteroid/asteroid.h (1.1),
	Bmpman/bmpman.cpp (1.1), Bmpman/bmpman.h (1.1), CFile/cfile.cpp
	(1.1), CFile/cfile.h (1.1), CFile/cfilearchive.cpp (1.1),
	CFile/cfilearchive.h (1.1), CFile/cfilelist.cpp (1.1),
	CFile/cfilesystem.cpp (1.1), CFile/cfilesystem.h (1.1),
	CMeasure/cmeasure.cpp (1.1), CMeasure/cmeasure.h (1.1),
	Cfilearchiver/cfilearchiver.cpp (1.1), Cmdline/cmdline.cpp (1.1),
	Cmdline/cmdline.h (1.1), ControlConfig/controlsconfig.cpp (1.1),
	ControlConfig/controlsconfig.h (1.1),
	ControlConfig/controlsconfigcommon.cpp (1.1),
	Cryptstring/cryptstring.cpp (1.1), Cutscene/cutscenes.cpp (1.1),
	Cutscene/cutscenes.h (1.1), Debris/debris.cpp (1.1),
	Debris/debris.h (1.1), DebugConsole/console.cpp (1.1),
	Demo/demo.cpp (1.1), Demo/demo.h (1.1), DirectX/vasync.h (1.1),
	DirectX/vd3d.h (1.1), DirectX/vd3dcaps.h (1.1), DirectX/vd3di.h
	(1.1), DirectX/vd3drm.h (1.1), DirectX/vd3drmdef.h (1.1),
	DirectX/vd3drmobj.h (1.1), DirectX/vd3drmwin.h (1.1),
	DirectX/vd3dtypes.h (1.1), DirectX/vd3dvec.inl (1.1),
	DirectX/vddraw.h (1.1), DirectX/vddraw.lib (1.1), DirectX/vdinput.h
	(1.1), DirectX/vdinput.lib (1.1), DirectX/vdplay.h (1.1),
	DirectX/vdplobby.h (1.1), DirectX/vdsetup.h (1.1),
	DirectX/vdsound.h (1.1), DirectX/vdsound.lib (1.1), DirectX/vdvp.h
	(1.1), DirectX/vdxguid.lib (1.1),
	ExceptionHandler/exceptionhandler.cpp (1.1),
	ExceptionHandler/exceptionhandler.h (1.1), FREESPACE2/app_icon.ico
	(1.1), FREESPACE2/freespace.cpp (1.1), FREESPACE2/freespace.h
	(1.1), FREESPACE2/freespace.rc (1.1),
	FREESPACE2/freespaceresource.h (1.1), FREESPACE2/goal_com.bmp
	(1.1), FREESPACE2/goal_fail.bmp (1.1), FREESPACE2/goal_inc.bmp
	(1.1), FREESPACE2/goal_none.bmp (1.1), FREESPACE2/goal_ord.bmp
	(1.1), FREESPACE2/levelpaging.cpp (1.1), FREESPACE2/levelpaging.h
	(1.1), Fireball/fireballs.cpp (1.1), Fireball/fireballs.h (1.1),
	Fireball/warpineffect.cpp (1.1), GameHelp/contexthelp.cpp (1.1),
	GameHelp/contexthelp.h (1.1), GameHelp/gameplayhelp.cpp (1.1),
	GameHelp/gameplayhelp.h (1.1), GameSequence/gamesequence.cpp (1.1),
	GameSequence/gamesequence.h (1.1), Gamesnd/eventmusic.cpp (1.1),
	Gamesnd/eventmusic.h (1.1), Gamesnd/gamesnd.cpp (1.1),
	Gamesnd/gamesnd.h (1.1), Glide/3dfx.h (1.1), Glide/fxdll.h (1.1),
	Glide/fxglob.h (1.1), Glide/fxos.h (1.1), Glide/glide.cpp (1.1),
	Glide/glide.h (1.1), Glide/glidesys.h (1.1), Glide/glideutl.h
	(1.1), Glide/sst1vid.h (1.1), GlobalIncs/alphacolors.cpp (1.1),
	GlobalIncs/alphacolors.h (1.1), GlobalIncs/crypt.cpp (1.1),
	GlobalIncs/crypt.h (1.1), GlobalIncs/linklist.h (1.1),
	GlobalIncs/pstypes.h (1.1), GlobalIncs/systemvars.cpp (1.1),
	GlobalIncs/systemvars.h (1.1), GlobalIncs/version.cpp (1.1),
	GlobalIncs/version.h (1.1), GlobalIncs/windebug.cpp (1.1),
	Graphics/2d.cpp (1.1), Graphics/2d.h (1.1), Graphics/aaline.cpp
	(1.1), Graphics/bitblt.cpp (1.1), Graphics/bitblt.h (1.1),
	Graphics/circle.cpp (1.1), Graphics/circle.h (1.1),
	Graphics/colors.cpp (1.1), Graphics/colors.h (1.1),
	Graphics/font.cpp (1.1), Graphics/font.h (1.1),
	Graphics/gradient.cpp (1.1), Graphics/gradient.h (1.1),
	Graphics/grd3d.cpp (1.1), Graphics/grd3d.h (1.1),
	Graphics/grd3dinternal.h (1.1), Graphics/grd3drender.cpp (1.1),
	Graphics/grd3dtexture.cpp (1.1), Graphics/grdirectdraw.cpp (1.1),
	Graphics/grdirectdraw.h (1.1), Graphics/grglide.cpp (1.1),
	Graphics/grglide.h (1.1), Graphics/grglideinternal.h (1.1),
	Graphics/grglidetexture.cpp (1.1), Graphics/grinternal.h (1.1),
	Graphics/gropengl.cpp (1.1), Graphics/gropengl.h (1.1),
	Graphics/grsoft.cpp (1.1), Graphics/grsoft.h (1.1),
	Graphics/grzbuffer.cpp (1.1), Graphics/grzbuffer.h (1.1),
	Graphics/line.cpp (1.1), Graphics/line.h (1.1), Graphics/pixel.cpp
	(1.1), Graphics/pixel.h (1.1), Graphics/rect.cpp (1.1),
	Graphics/rect.h (1.1), Graphics/scaler.cpp (1.1), Graphics/scaler.h
	(1.1), Graphics/shade.cpp (1.1), Graphics/shade.h (1.1),
	Graphics/tmapgenericscans.cpp (1.1), Graphics/tmapper.cpp (1.1),
	Graphics/tmapper.h (1.1), Graphics/tmapscanline.cpp (1.1),
	Graphics/tmapscanline.h (1.1), Graphics/tmapscantiled128x128.cpp
	(1.1), Graphics/tmapscantiled16x16.cpp (1.1),
	Graphics/tmapscantiled256x256.cpp (1.1),
	Graphics/tmapscantiled32x32.cpp (1.1),
	Graphics/tmapscantiled64x64.cpp (1.1), Hud/hud.cpp (1.1), Hud/hud.h
	(1.1), Hud/hudartillery.cpp (1.1), Hud/hudartillery.h (1.1),
	Hud/hudbrackets.cpp (1.1), Hud/hudbrackets.h (1.1),
	Hud/hudconfig.cpp (1.1), Hud/hudconfig.h (1.1), Hud/hudescort.cpp
	(1.1), Hud/hudescort.h (1.1), Hud/hudets.cpp (1.1), Hud/hudets.h
	(1.1), Hud/hudgauges.h (1.1), Hud/hudlock.cpp (1.1), Hud/hudlock.h
	(1.1), Hud/hudmessage.cpp (1.1), Hud/hudmessage.h (1.1),
	Hud/hudobserver.cpp (1.1), Hud/hudobserver.h (1.1),
	Hud/hudresource.cpp (1.1), Hud/hudresource.h (1.1),
	Hud/hudreticle.cpp (1.1), Hud/hudreticle.h (1.1), Hud/hudshield.cpp
	(1.1), Hud/hudshield.h (1.1), Hud/hudsquadmsg.cpp (1.1),
	Hud/hudsquadmsg.h (1.1), Hud/hudtarget.cpp (1.1), Hud/hudtarget.h
	(1.1), Hud/hudtargetbox.cpp (1.1), Hud/hudtargetbox.h (1.1),
	Hud/hudwingmanstatus.cpp (1.1), Hud/hudwingmanstatus.h (1.1),
	Inetfile/cftp.cpp (1.1), Inetfile/cftp.h (1.1),
	Inetfile/chttpget.cpp (1.1), Inetfile/chttpget.h (1.1),
	Inetfile/inetgetfile.cpp (1.1), Inetfile/inetgetfile.h (1.1),
	Io/joy.cpp (1.1), Io/joy.h (1.1), Io/joy_ff.cpp (1.1), Io/joy_ff.h
	(1.1), Io/key.cpp (1.1), Io/key.h (1.1), Io/keycontrol.cpp (1.1),
	Io/keycontrol.h (1.1), Io/mouse.cpp (1.1), Io/mouse.h (1.1),
	Io/sw_error.hpp (1.1), Io/sw_force.h (1.1), Io/sw_guid.hpp (1.1),
	Io/swff_lib.cpp (1.1), Io/timer.cpp (1.1), Io/timer.h (1.1),
	JumpNode/jumpnode.cpp (1.1), JumpNode/jumpnode.h (1.1),
	Lighting/lighting.cpp (1.1), Lighting/lighting.h (1.1),
	Localization/fhash.cpp (1.1), Localization/fhash.h (1.1),
	Localization/localize.cpp (1.1), Localization/localize.h (1.1),
	Math/fix.cpp (1.1), Math/fix.h (1.1), Math/floating.cpp (1.1),
	Math/floating.h (1.1), Math/fvi.cpp (1.1), Math/fvi.h (1.1),
	Math/spline.cpp (1.1), Math/spline.h (1.1), Math/staticrand.cpp
	(1.1), Math/staticrand.h (1.1), Math/vecmat.cpp (1.1),
	Math/vecmat.h (1.1), MenuUI/barracks.cpp (1.1), MenuUI/barracks.h
	(1.1), MenuUI/credits.cpp (1.1), MenuUI/credits.h (1.1),
	MenuUI/fishtank.cpp (1.1), MenuUI/fishtank.h (1.1),
	MenuUI/mainhallmenu.cpp (1.1), MenuUI/mainhallmenu.h (1.1),
	MenuUI/mainhalltemp.cpp (1.1), MenuUI/mainhalltemp.h (1.1),
	MenuUI/optionsmenu.cpp (1.1), MenuUI/optionsmenu.h (1.1),
	MenuUI/optionsmenumulti.cpp (1.1), MenuUI/optionsmenumulti.h (1.1),
	MenuUI/playermenu.cpp (1.1), MenuUI/playermenu.h (1.1),
	MenuUI/readyroom.cpp (1.1), MenuUI/readyroom.h (1.1),
	MenuUI/snazzyui.cpp (1.1), MenuUI/snazzyui.h (1.1),
	MenuUI/techmenu.cpp (1.1), MenuUI/techmenu.h (1.1),
	MenuUI/trainingmenu.cpp (1.1), MenuUI/trainingmenu.h (1.1),
	Mission/missionbriefcommon.cpp (1.1), Mission/missionbriefcommon.h
	(1.1), Mission/missioncampaign.cpp (1.1), Mission/missioncampaign.h
	(1.1), Mission/missiongoals.cpp (1.1), Mission/missiongoals.h
	(1.1), Mission/missiongrid.cpp (1.1), Mission/missiongrid.h (1.1),
	Mission/missionhotkey.cpp (1.1), Mission/missionhotkey.h (1.1),
	Mission/missionload.cpp (1.1), Mission/missionload.h (1.1),
	Mission/missionlog.cpp (1.1), Mission/missionlog.h (1.1),
	Mission/missionmessage.cpp (1.1), Mission/missionmessage.h (1.1),
	Mission/missionparse.cpp (1.1), Mission/missionparse.h (1.1),
	Mission/missiontraining.cpp (1.1), Mission/missiontraining.h (1.1),
	MissionUI/chatbox.cpp (1.1), MissionUI/chatbox.h (1.1),
	MissionUI/missionbrief.cpp (1.1), MissionUI/missionbrief.h (1.1),
	MissionUI/missioncmdbrief.cpp (1.1), MissionUI/missioncmdbrief.h
	(1.1), MissionUI/missiondebrief.cpp (1.1),
	MissionUI/missiondebrief.h (1.1), MissionUI/missionloopbrief.cpp
	(1.1), MissionUI/missionloopbrief.h (1.1),
	MissionUI/missionpause.cpp (1.1), MissionUI/missionpause.h (1.1),
	MissionUI/missionrecommend.cpp (1.1), MissionUI/missionrecommend.h
	(1.1), MissionUI/missionscreencommon.cpp (1.1),
	MissionUI/missionscreencommon.h (1.1),
	MissionUI/missionshipchoice.cpp (1.1),
	MissionUI/missionshipchoice.h (1.1), MissionUI/missionstats.cpp
	(1.1), MissionUI/missionstats.h (1.1),
	MissionUI/missionweaponchoice.cpp (1.1),
	MissionUI/missionweaponchoice.h (1.1), MissionUI/redalert.cpp
	(1.1), MissionUI/redalert.h (1.1), Model/model.h (1.1),
	Model/modelcollide.cpp (1.1), Model/modelinterp.cpp (1.1),
	Model/modeloctant.cpp (1.1), Model/modelread.cpp (1.1),
	Model/modelsinc.h (1.1), Nebula/neb.cpp (1.1), Nebula/neb.h (1.1),
	Nebula/neblightning.cpp (1.1), Nebula/neblightning.h (1.1),
	Network/multi.cpp (1.1), Network/multi.h (1.1),
	Network/multi_campaign.cpp (1.1), Network/multi_campaign.h (1.1),
	Network/multi_data.cpp (1.1), Network/multi_data.h (1.1),
	Network/multi_dogfight.cpp (1.1), Network/multi_dogfight.h (1.1),
	Network/multi_endgame.cpp (1.1), Network/multi_endgame.h (1.1),
	Network/multi_ingame.cpp (1.1), Network/multi_ingame.h (1.1),
	Network/multi_kick.cpp (1.1), Network/multi_kick.h (1.1),
	Network/multi_log.cpp (1.1), Network/multi_log.h (1.1),
	Network/multi_obj.cpp (1.1), Network/multi_obj.h (1.1),
	Network/multi_observer.cpp (1.1), Network/multi_observer.h (1.1),
	Network/multi_oo.cpp (1.1), Network/multi_oo.h (1.1),
	Network/multi_options.cpp (1.1), Network/multi_options.h (1.1),
	Network/multi_pause.cpp (1.1), Network/multi_pause.h (1.1),
	Network/multi_pinfo.cpp (1.1), Network/multi_pinfo.h (1.1),
	Network/multi_ping.cpp (1.1), Network/multi_ping.h (1.1),
	Network/multi_pmsg.cpp (1.1), Network/multi_pmsg.h (1.1),
	Network/multi_rate.cpp (1.1), Network/multi_rate.h (1.1),
	Network/multi_respawn.cpp (1.1), Network/multi_respawn.h (1.1),
	Network/multi_team.cpp (1.1), Network/multi_team.h (1.1),
	Network/multi_update.cpp (1.1), Network/multi_update.h (1.1),
	Network/multi_voice.cpp (1.1), Network/multi_voice.h (1.1),
	Network/multi_xfer.cpp (1.1), Network/multi_xfer.h (1.1),
	Network/multilag.cpp (1.1), Network/multilag.h (1.1),
	Network/multimsgs.cpp (1.1), Network/multimsgs.h (1.1),
	Network/multiteamselect.cpp (1.1), Network/multiteamselect.h (1.1),
	Network/multiui.cpp (1.1), Network/multiui.h (1.1),
	Network/multiutil.cpp (1.1), Network/multiutil.h (1.1),
	Network/psnet.cpp (1.1), Network/psnet.h (1.1), Network/psnet2.cpp
	(1.1), Network/psnet2.h (1.1), Network/stand_gui.cpp (1.1),
	Network/stand_gui.h (1.1), Object/collidedebrisship.cpp (1.1),
	Object/collidedebrisweapon.cpp (1.1), Object/collideshipship.cpp
	(1.1), Object/collideshipweapon.cpp (1.1),
	Object/collideweaponweapon.cpp (1.1), Object/objcollide.cpp (1.1),
	Object/objcollide.h (1.1), Object/object.cpp (1.1), Object/object.h
	(1.1), Object/objectsnd.cpp (1.1), Object/objectsnd.h (1.1),
	Object/objectsort.cpp (1.1), Observer/observer.cpp (1.1),
	Observer/observer.h (1.1), OsApi/monopub.h (1.1), OsApi/osapi.cpp
	(1.1), OsApi/osapi.h (1.1), OsApi/osregistry.cpp (1.1),
	OsApi/osregistry.h (1.1), OsApi/outwnd.cpp (1.1), OsApi/outwnd.h
	(1.1), Palman/palman.cpp (1.1), Palman/palman.h (1.1),
	Parse/encrypt.cpp (1.1), Parse/encrypt.h (1.1), Parse/parselo.cpp
	(1.1), Parse/parselo.h (1.1), Parse/sexp.cpp (1.1), Parse/sexp.h
	(1.1), Particle/particle.cpp (1.1), Particle/particle.h (1.1),
	PcxUtils/pcxutils.cpp (1.1), PcxUtils/pcxutils.h (1.1),
	Physics/physics.cpp (1.1), Physics/physics.h (1.1),
	Playerman/managepilot.cpp (1.1), Playerman/managepilot.h (1.1),
	Playerman/player.h (1.1), Playerman/playercontrol.cpp (1.1),
	Popup/popup.cpp (1.1), Popup/popup.h (1.1), Popup/popupdead.cpp
	(1.1), Popup/popupdead.h (1.1), Radar/radar.cpp (1.1),
	Radar/radar.h (1.1), Render/3d.h (1.1), Render/3dclipper.cpp (1.1),
	Render/3ddraw.cpp (1.1), Render/3dinternal.h (1.1),
	Render/3dlaser.cpp (1.1), Render/3dmath.cpp (1.1),
	Render/3dsetup.cpp (1.1), Scramble/scramble.cpp (1.1),
	Scramble/scramble.h (1.1), Ship/afterburner.cpp (1.1),
	Ship/afterburner.h (1.1), Ship/ai.cpp (1.1), Ship/ai.h (1.1),
	Ship/aibig.cpp (1.1), Ship/aibig.h (1.1), Ship/aicode.cpp (1.1),
	Ship/aigoals.cpp (1.1), Ship/aigoals.h (1.1), Ship/ailocal.h (1.1),
	Ship/awacs.cpp (1.1), Ship/awacs.h (1.1), Ship/shield.cpp (1.1),
	Ship/ship.cpp (1.1), Ship/ship.h (1.1), Ship/shipcontrails.cpp
	(1.1), Ship/shipcontrails.h (1.1), Ship/shipfx.cpp (1.1),
	Ship/shipfx.h (1.1), Ship/shiphit.cpp (1.1), Ship/shiphit.h (1.1),
	Ship/subsysdamage.h (1.1), Sound/acm.cpp (1.1), Sound/acm.h (1.1),
	Sound/audiostr.cpp (1.1), Sound/audiostr.h (1.1), Sound/channel.h
	(1.1), Sound/ds.cpp (1.1), Sound/ds.h (1.1), Sound/ds3d.cpp (1.1),
	Sound/ds3d.h (1.1), Sound/dscap.cpp (1.1), Sound/dscap.h (1.1),
	Sound/midifile.cpp (1.1), Sound/midifile.h (1.1), Sound/midiseq.h
	(1.1), Sound/rbaudio.cpp (1.1), Sound/rbaudio.h (1.1),
	Sound/rtvoice.cpp (1.1), Sound/rtvoice.h (1.1), Sound/sound.cpp
	(1.1), Sound/sound.h (1.1), Sound/winmidi.cpp (1.1),
	Sound/winmidi.h (1.1), Sound/winmidi_base.cpp (1.1),
	Starfield/nebula.cpp (1.1), Starfield/nebula.h (1.1),
	Starfield/starfield.cpp (1.1), Starfield/starfield.h (1.1),
	Starfield/supernova.cpp (1.1), Starfield/supernova.h (1.1),
	Stats/medals.cpp (1.1), Stats/medals.h (1.1), Stats/scoring.cpp
	(1.1), Stats/scoring.h (1.1), Stats/stats.cpp (1.1), Stats/stats.h
	(1.1), TgaUtils/tgautils.cpp (1.1), TgaUtils/tgautils.h (1.1),
	UI/button.cpp (1.1), UI/checkbox.cpp (1.1), UI/gadget.cpp (1.1),
	UI/icon.cpp (1.1), UI/inputbox.cpp (1.1), UI/keytrap.cpp (1.1),
	UI/listbox.cpp (1.1), UI/radio.cpp (1.1), UI/scroll.cpp (1.1),
	UI/slider.cpp (1.1), UI/slider2.cpp (1.1), UI/ui.h (1.1),
	UI/uidefs.h (1.1), UI/uidraw.cpp (1.1), UI/uimouse.cpp (1.1),
	UI/window.cpp (1.1), VCodec/codec1.cpp (1.1), VCodec/codec1.h
	(1.1), Weapon/beam.cpp (1.1), Weapon/beam.h (1.1),
	Weapon/corkscrew.cpp (1.1), Weapon/corkscrew.h (1.1),
	Weapon/emp.cpp (1.1), Weapon/emp.h (1.1), Weapon/flak.cpp (1.1),
	Weapon/flak.h (1.1), Weapon/muzzleflash.cpp (1.1),
	Weapon/muzzleflash.h (1.1), Weapon/shockwave.cpp (1.1),
	Weapon/shockwave.h (1.1), Weapon/swarm.cpp (1.1), Weapon/swarm.h
	(1.1), Weapon/trails.cpp (1.1), Weapon/trails.h (1.1),
	Weapon/weapon.h (1.1), Weapon/weapons.cpp (1.1) (utags:
	volition_import):
	
	Volition sources -- warpcore CVS import


# generated by cvs2cl.pl
# cvs2cl.pl -r -S --utc -f ChangeLog2
```
