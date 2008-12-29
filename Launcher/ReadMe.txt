Launcher 3.2 Readme

1. What is the Launcher?

The launcher is a small set-up utility much like the original launcher that came with the retail game. You need to run it once to set-up the game but it is also useful to tweak fs2_open settings to get the most out of our changes.

1.1. I want to play fs2_open now!

OK, simply select your fs2_open exe using the Browse button and press run.
However most fs2_open extra functionality is off by default so at some point it would be wise to checkout the flags and video tab.

2. The main window

2.1. Browse

This brings up a standard browse window that allows you to select the fs2_open exe you wish to set-up / run. After you select the path of the exe can be seen in the box to the left of the browse button.

2.2. Run

This will save your settings and run the exe selected.

2.3. OK

This will save your settings and close the Launcher

2.4. Cancel

This will close the Launcher without saving settings

2.5. Apply

This will save your settings without closing the Launcher

3. The Welcome Tab

Readme
Forums
Report Bug

4. The Flags Tab

This is probably the most important tab, it directly controls which extra features by adding command line flags.

4.1. Command line box

The command line box shows you exactly how all your flag choices have been combined. This could be useful for making shortcuts with the given flag list.

4.2. Custom flags 

Some of fs2_open flags need parameters, this isn't currently handled in the list below so they can be types in here. Also new parameters that have added after this Launcher was compiled can be added here as well. If you put in an incorrect parameter fs2_open will tell you but let your continue anyway.

4.3. Select MOD and No MOD

These button let you control the use of any Freespace MOD's you may have. Any MOD's that are set-up to work with fs2_open should be in a directory in your Freespace directory. Use 'Select MOD' to select that directory.

4.4. Standard flags

4.4.1. This list shows you many, but not all of the flags available for fs2_open. Go to the help website for details of them all. Clicking on the boxes sometimes gives you info well worth reading, some features require extra art to work. To turn help off using the tick box at bottom right.

4.4.2. Flag functionality

I don't want to get into all the different functionality of all the flags but I will mention a very important one:

-htl 

This enables Hardware Transform and Light if you are using D3D or OGL. At the time of writing this feature is not bug free but it is stable and provides a much needed frame rate boost to ANY system. 

For information on at other parameters try clicking on them and the launcher may give you some info. Otherwise you should be able to find help online if you press the 'Readme' button in the welcome tab. Remember visual improvements always mean a loss of frame rates.

4.5. Use these flags running fs2_open directly

If you tick this box when you run fs2_open it will use the flags set without having to run it through the launcher. If you use a shortcut flags lists will be combined, so you could use the launcher once to set-up htl and use shortcuts to activate different MOD's.

If you have this off the flag settings will be remembered by the Launcher but you will have to run fs2_open through the launcher to use them.

4.6. Help

This toggles info on the flags in the list, not all flags have help.

5.  The Video Tab

You must have all seen a graphics mode selection screen before so I'll only go into stuff specific to this project. There are three API's to run fs2_open under, Direct3D8, OpenGL, Glide.

Generally it's a good idea to try D3D and OGL and see which one is faster with your system. Not all features are implemented in both but we intend to implement all features in both of these API's.

Glide is dying out, don't use this unless you have to. And of course, don't use this if you don't have a 3DFX card. To honest at the time of writing I can't be sure it even works still. Glide has had no development, will support few of the new visual features and I expect it to be cut out of the project at some point.

5.1. Modes

The standard fs2_open resolutions are 640x480 and 1024x768. 

5.2. Direct3D8

D3D allows you to run at non-standard resolutions but the code is still in development so there are a fair few bugs running around. Antialiasing is available but also has some bugs.

5.3. OpenGL

OGL is nice and simple, pick your mode and go with it.

5.4. General Settings

5.4.1. Four options which work the same way as they used to in the original launcher, they control the very general settings of Freespace. Leave set at High unless you have a really crap system.

5.4.2. Hi res pack installed

This indicated if you have the high res pack installed. If you do not you will not be able to in modes higher than 640x480. To install this pack reinstall retail Freespace 2 and remember to install high quality art.

5.4.3. Use Large Textures

This allows Freespace to use large textures (bigger than 256x256) if your card supports it (which it will unless it's a voodoo). This will improve the quality of some textures but may slow things down a little.

5.4.4. Fix Geforce Font Problem

Some Geforce cards experience problems at the edges of letters. Checking this box fixes that bug if you don't have the problem in the first place it will cause the problem. So don't check it unless you actually have the problem in the first place.

6. The Audo/Joystick Tab

6.1. Sound

This is very similar to the original Launcher. Select your sound card. Don't select EAX or A3D unless your card supports it though.

6.2. Joystick

Select your joystick. If your joystick isn't listed use force 0.

7.1. The Speech Tab

Simulated speech doesn't work on all systems, but I have yet to hear of one it didn't. Activating this means any text in the game that does not have audio attached will be read out by a simulated voice. It doesn't sound human but it's a lot better than nothing. This is most useful if you are playing a MOD that doesn't have any voice acting. Depending on your system you may have to install some packs first though. 

7.2. Get working on 98 and ME 

Go to the Microsoft site and download SAPI 5.1 SDK

7.3. Get working on 2000 and XP 

XP already has SAPI installed and one voice, however there are two slightly higher quality voices that you can get by clicking on the 'Get Addition Voices' button. I believe 2000 is the same, if not follow the 98 / ME instructions.
 
7.4. Testing it works

To find out simply press 'Play', if you can hear it then it should work in the game.

7.5. It doesn't work in the game!

Due to technical reasons only main builds generally have speech compiled in. If you are using an in between build its quite possible it wont work. You might have to ask a coder on the boards to make an exe that has it.

8. The Network Tab

This isn't relevant for fs2_open because multiplayer doesn't work yet.
The code was stripped out before the source code went public, efforts are being made to recode it.

9. The Registry Tab

9.1. Look

Allows you to look at your Freespace registry options, if the window empty then something is not right. Looking at the registry tab can help you let coders know more about any problems you might be having. 

9.2. But don't touch

If you want to start editing registry entries without direct coder advice then you are entering a world of pain. All of the relevant entries are set-up by the Launcher so it will just set them all back anyway.


