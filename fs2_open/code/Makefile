TARGET = libcode.a

CXXFLAGS =  -DWIN32 -D_DEBUG \
 -Iwindows_stub \
 -IAnim -IAsteroid -IBmpman -ICFile -ICmdline -ICMeasure -IControlConfig \
 -ICutscene -IDebris -IDebugConsole -IDirectX -IFireball -IGameHelp -IFREESPACE2 \
 -IGameSequence -IGamesnd -IGlide -IGlobalIncs -IGraphics -IHud -IIo -IJumpNode \
 -ILighting -IMath -IMenuUI -IMission -IMissionUI -IModel -INetwork -IObject \
 -IObserver -IOsApi -IPalman -IParse -IParticle -IPcxUtils -IPhysics -IPlayerman \
 -IPopup -IRadar -IRender -IShip -ISound -IStarfield -IStats -ITgaUtils -IUI -IVCodec \
 -IWeapon -ILocalization -INebula -IDemo -IInetfile -IExceptionHandler \
 -D_DEBUG -DNO_CD_CHECK -UNDEBUG

# removed from OBJFILES
# CFile/cfile.o \
# CFile/CfileArchive.o \
# CFile/CfileList.o \
# CFile/CfileSystem.o \

OBJFILES = \
 Anim/animplay.o \
 Anim/packunpack.o \
 Asteroid/asteroid.o \
 Bmpman/bmpman.o \
 Cmdline/cmdline.o \
 CMeasure/cmeasure.o \
 ControlConfig/controlsconfig.o \
 ControlConfig/controlsconfigcommon.o \
 Cutscene/cutscenes.o \
 Debris/debris.o \
 DebugConsole/console.o \
 Fireball/fireballs.o \
 Fireball/warpineffect.o \
 GameHelp/contexthelp.o \
 GameHelp/gameplayhelp.o \
 GameSequence/gamesequence.o \
 Gamesnd/eventmusic.o \
 Gamesnd/gamesnd.o \
 Glide/glide.o \
 GlobalIncs/alphacolors.o \
 GlobalIncs/crypt.o \
 GlobalIncs/systemvars.o \
 GlobalIncs/version.o \
 GlobalIncs/windebug.o \
 Graphics/2d.o \
 Graphics/aaline.o \
 Graphics/bitblt.o \
 Graphics/circle.o \
 Graphics/colors.o \
 Graphics/font.o \
 Graphics/gradient.o \
 Graphics/grd3d.o \
 Graphics/grd3drender.o \
 Graphics/grd3dtexture.o \
 Graphics/grdirectdraw.o \
 Graphics/grglide.o \
 Graphics/grglidetexture.o \
 Graphics/gropengl.o \
 Graphics/grsoft.o \
 Graphics/grzbuffer.o \
 Graphics/line.o \
 Graphics/pixel.o \
 Graphics/rect.o \
 Graphics/scaler.o \
 Graphics/shade.o \
 Graphics/tmapgenericscans.o \
 Graphics/tmapper.o \
 Graphics/tmapscanline.o \
 Graphics/tmapscantiled128x128.o \
 Graphics/tmapscantiled16x16.o \
 Graphics/tmapscantiled256x256.o \
 Graphics/tmapscantiled32x32.o \
 Graphics/tmapscantiled64x64.o \
 Hud/hud.o \
 Hud/hudartillery.o \
 Hud/hudbrackets.o \
 Hud/hudconfig.o \
 Hud/hudescort.o \
 Hud/hudets.o \
 Hud/hudlock.o \
 Hud/hudmessage.o \
 Hud/hudobserver.o \
 Hud/hudreticle.o \
 Hud/hudshield.o \
 Hud/hudsquadmsg.o \
 Hud/hudtarget.o \
 Hud/hudtargetbox.o \
 Hud/hudwingmanstatus.o \
 Io/joy.o \
 Io/joy_ff.o \
 Io/key.o \
 Io/keycontrol.o \
 Io/mouse.o \
 Io/swff_lib.o \
 Io/timer.o \
 JumpNode/jumpnode.o \
 Lighting/lighting.o \
 Math/fix.o \
 Math/floating.o \
 Math/fvi.o \
 Math/spline.o \
 Math/staticrand.o \
 Math/vecmat.o \
 MenuUI/barracks.o \
 MenuUI/credits.o \
 MenuUI/fishtank.o \
 MenuUI/mainhallmenu.o \
 MenuUI/mainhalltemp.o \
 MenuUI/optionsmenu.o \
 MenuUI/optionsmenumulti.o \
 MenuUI/playermenu.o \
 MenuUI/readyroom.o \
 MenuUI/snazzyui.o \
 MenuUI/techmenu.o \
 MenuUI/trainingmenu.o \
 Mission/missionbriefcommon.o \
 Mission/missioncampaign.o \
 Mission/missiongoals.o \
 Mission/missiongrid.o \
 Mission/missionhotkey.o \
 Mission/missionload.o \
 Mission/missionlog.o \
 Mission/missionmessage.o \
 Mission/missionparse.o \
 Mission/missiontraining.o \
 MissionUI/chatbox.o \
 MissionUI/missionbrief.o \
 MissionUI/missioncmdbrief.o \
 MissionUI/missiondebrief.o \
 MissionUI/missionloopbrief.o \
 MissionUI/missionpause.o \
 MissionUI/missionrecommend.o \
 MissionUI/missionscreencommon.o \
 MissionUI/missionshipchoice.o \
 MissionUI/missionstats.o \
 MissionUI/missionweaponchoice.o \
 MissionUI/redalert.o \
 Model/modelcollide.o \
 Model/modelinterp.o \
 Model/modeloctant.o \
 Model/modelread.o \
 Object/collidedebrisship.o \
 Object/collidedebrisweapon.o \
 Object/collideshipship.o \
 Object/collideshipweapon.o \
 Object/collideweaponweapon.o \
 Object/objcollide.o \
 Object/object.o \
 Object/objectsnd.o \
 Object/objectsort.o \
 Observer/observer.o \
 OsApi/osapi.o \
 OsApi/osregistry.o \
 OsApi/outwnd.o \
 Palman/palman.o \
 Parse/encrypt.o \
 Parse/parselo.o \
 Parse/sexp.o \
 Particle/particle.o \
 PcxUtils/pcxutils.o \
 Physics/physics.o \
 Playerman/managepilot.o \
 Playerman/playercontrol.o \
 Popup/popup.o \
 Popup/popupdead.o \
 Radar/radar.o \
 Render/3dclipper.o \
 Render/3ddraw.o \
 Render/3dlaser.o \
 Render/3dmath.o \
 Render/3dsetup.o \
 Ship/afterburner.o \
 Ship/ai.o \
 Ship/aibig.o \
 Ship/aicode.o \
 Ship/aigoals.o \
 Ship/awacs.o \
 Ship/shield.o \
 Ship/ship.o \
 Ship/shipcontrails.o \
 Ship/shipfx.o \
 Ship/shiphit.o \
 Sound/acm.o \
 Sound/audiostr.o \
 Sound/ds.o \
 Sound/ds3d.o \
 Sound/dscap.o \
 Sound/midifile.o \
 Sound/rbaudio.o \
 Sound/rtvoice.o \
 Sound/sound.o \
 Sound/winmidi.o \
 Sound/winmidi_base.o \
 Starfield/nebula.o \
 Starfield/starfield.o \
 Starfield/supernova.o \
 Stats/medals.o \
 Stats/scoring.o \
 Stats/stats.o \
 UI/button.o \
 UI/checkbox.o \
 UI/gadget.o \
 UI/icon.o \
 UI/inputbox.o \
 UI/keytrap.o \
 UI/listbox.o \
 UI/radio.o \
 UI/scroll.o \
 UI/slider.o \
 UI/slider2.o \
 UI/uidraw.o \
 UI/uimouse.o \
 UI/window.o \
 VCodec/codec1.o \
 Weapon/beam.o \
 Weapon/corkscrew.o \
 Weapon/emp.o \
 Weapon/flak.o \
 Weapon/muzzleflash.o \
 Weapon/shockwave.o \
 Weapon/swarm.o \
 Weapon/trails.o \
 Weapon/weapons.o \
 Nebula/neb.o \
 Nebula/neblightning.o \
 Localization/fhash.o \
 Localization/localize.o \
 TgaUtils/tgautils.o \
 Demo/demo.o \
 Inetfile/cftp.o \
 Inetfile/chttpget.o \
 Inetfile/inetgetfile.o \
 ExceptionHandler/exceptionhandler.o \
 Network/multi.o \
 Network/multi_campaign.o \
 Network/multi_data.o \
 Network/multi_dogfight.o \
 Network/multi_endgame.o \
 Network/multi_ingame.o \
 Network/multi_kick.o \
 Network/multi_log.o \
 Network/multi_obj.o \
 Network/multi_observer.o \
 Network/multi_oo.o \
 Network/multi_options.o \
 Network/multi_pause.o \
 Network/multi_pinfo.o \
 Network/multi_ping.o \
 Network/multi_pmsg.o \
 Network/multi_rate.o \
 Network/multi_respawn.o \
 Network/multi_team.o \
 Network/multi_update.o \
 Network/multi_voice.o \
 Network/multi_xfer.o \
 Network/multilag.o \
 Network/multimsgs.o \
 Network/multiteamselect.o \
 Network/multiui.o \
 Network/multiutil.o \
 Network/psnet.o \
 Network/psnet2.o \
 Network/stand_gui.o


# ==============================
.PHONY: all clean code

.cpp.o:	
	$(CXX) $(CXXFLAGS) -E $< > $@
#	$(CXX) $(CXXFLAGS) -c $<


all:	$(TARGET)

clean:
	rm -f $(TARGET) core *.o *~
	find -name *.o -exec rm {} \;
	find -name *~ -exec rm {} \;

code:	$(TARGET)


# ==============================
libcode.a:	$(OBJFILES)
