
<%def name="build(file)">[url=${file.url}]${file.filename}[/url]</%def>

[b]Change log:[/b] (chronologically ordered)
PLACEHOLDER

[b]Deprecations:[/b]
PLACEHOLDER

Deprecations are a mechanism in FSO where a certain feature or aspect of the engine has changed or is no longer supported. Since this would normally break existing mods we have the mod table feature "[tt]$Target Version:[/tt]" with which a mod can specify what version of FSO it was developed with. The features listed above will be removed or changed when the target version of a mod is at least the version released in this post.

[size=5pt]Previous [url=https://www.hard-light.net/forums/index.php?topic=98360.0]22.2.0 Release Thread[/url][/size]

Launchers, if you don't have one already:
[b]All platforms: [/b] For every day use, we recommend [url=https://www.hard-light.net/forums/index.php?topic=94068.0]Knossos[/url], an integrated solution for downloading and launching mods.

[hidden=Alternative Launchers]
Cross-platform: [url=https://www.hard-light.net/forums/index.php?topic=89162]wxLauncher 0.12.x Test Build[/url] (ongoing project for a unified launcher, you should upgrade to the latest RC/test build if you have not yet)
[b]Important:[/b] For best compatibility with FSO 3.8 and later you should use at least wxLauncher 0.12.

Windows:  [url=https://scp.fsmods.net/files/Launcher55g.zip]Launcher 5.5g[/url] ([url=https://scp.indiegames.us/builds/Launcher55g.zip]Mirror[/url]) ([url=https://www.mediafire.com/?wdvzn7hhhzh418m]Mirror[/url]) Not compatible with Windows 8+, use wxLauncher above
OS X:  Soulstorm's [url=https://www.hard-light.net/forums/index.php/topic,51391.0.html]OS X Launcher 3.0[/url]
Linux:  [url=https://www.hard-light.net/forums/index.php/topic,53206.0.html]YAL[/url] or [url=https://www.hard-light.net/wiki/index.php/Fs2_open_on_Linux/Graphics_Settings]by hand[/url] or whatever you can figure out.[/hidden]

[img]https://scp.indiegames.us/img/windows-icon.png[/img] [color=green][size=12pt]Windows (32/64-bit)[/size][/color]
[size=8pt]Compiled using GitHub Actions on Windows Server 2019 (10.0.17763), Visual Studio Enterprise 2019[/size]

[b]64-bit:[/b] ${build(groups["Win64"].mainFile)}

[b]32-bit:[/b] ${build(groups["Win32"].mainFile)}
[size=8pt]This one is based on the SSE2 Optimizations from the MSVC Compiler.[/size]

[hidden=Alternative builds]

[b]64-bit AVX:[/b] ${build(groups["Win64"].subFiles["AVX"])}
[size=8pt]This one is based on the AVX Optimizations from the MSVC Compiler (fastest build if your CPU supports AVX instructions).[/size]


[b]32-bit AVX:[/b] ${build(groups["Win32"].subFiles["AVX"])}
[size=8pt]This one is based on the AVX Optimizations from the MSVC Compiler.[/size]

[b]What are those SSE, SSE2 and AVX builds I keep seeing everywhere?[/b]
[url=https://www.hard-light.net/forums/index.php?topic=65628.0]Your answer is in this topic.[/url]
Don't want to deal with that? Use [url=https://www.hard-light.net/forums/index.php?topic=94068.0]Knossos[/url] and it will download the best build specifically for your PC!
[/hidden]

[img]https://scp.indiegames.us/img/linux-icon.png[/img] [color=green][size=12pt]Linux 64-bit[/size][/color]
[size=8pt]Compiled with Ubuntu 16.04 LTS 64-bit, GCC 5[/size]
${build(groups["Linux"].mainFile)}

These builds use a mechanism called [url=https://appimage.org/]AppImage[/url] which should allow these builds to run on most Linux distributions. However, we recommend that you compile your own builds which will result in less issues.
Alternatively, if there is a package in your software repository then you should use that. If you are the maintainer of such a package for a distribution then let us know and we will include that here.


[img]https://scp.indiegames.us/img/mac-icon.png[/img] [color=green][size=12pt]OS X[/size][/color]
[b][color=red]Not available[/color][/b] We recently lost access to our Mac CI environment which we usually used for compiling these builds so for the time being, there will be no builds for this OS.

[hidden=TrackIR Users]
[size=12pt]Important!![/size]
An external DLL is required for FSO to use TrackIR functions.  The following DLL is simply unpacked in to your main FreeSpace2 root dir.
TrackIR is only supported on Windows.
[url=https://www.mediafire.com/download.php?4zw024zrh44etse]TrackIR SCP DLL[/url] ([url=https://scp.fsmods.net/builds/scptrackir.zip]Mirror[/url]) ([url=https://scp.indiegames.us/builds/scptrackir.zip]Mirror[/url])[/hidden]

Known issues:
[list]
[li][url=https://github.com/scp-fs2open/fs2open.github.com/issues]Github issues[/url] and [url=https://github.com/scp-fs2open/fs2open.github.com/pulls]pending pull requests[/url][/li]
[li]See the list of [url=https://github.com/scp-fs2open/fs2open.github.com/projects/8]bugs ranked by priority[/url][/li]
[/list]
