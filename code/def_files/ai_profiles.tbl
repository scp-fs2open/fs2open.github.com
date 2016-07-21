																		
;; AI Profiles table.  Incorporates stuff from the old difficulty.tbl	
;; plus additional flags previously covered under the blanket New AI	
;; flag.																
;;																		
;; This is what the retail table would look like, but you don't have to	
;; specify it as it's stored internally by the game.  This leaves you	
;; with four other slots to specify four other profiles.  Every setting	
;; is optional, so if you don't specify something it will inherit from	
;; the FS2 retail setting.  If you don't specify a default profile, it	
;; will set the default to retail as well.								
;;																		
																		
#AI Profiles															
																		
$Default Profile: FS2 RETAIL											
																		
																		
$Profile Name: FS2 RETAIL												
																		
																		
;; Difficulty-related values; much of this was originally in			
;; difficulty.tbl.  Each option specifies a list corresponding to the	
;; five skill values (Very Easy, Easy, Medium, Hard, Insane).			
																		
																		
;; speed of afterburner recharge										
$Player Afterburner Recharge Scale: 5, 3, 2, 1.5, 1						
																		
;; maximum damage inflicted by friendly beam fire						
$Max Beam Friendly Fire Damage: 0, 5, 10, 20, 30						
																		
;; factor applied to player countermeasure lifetime						
$Player Countermeasure Life Scale: 3, 2, 1.5, 1.25, 1					
																		
;; chance a countermeasure will be fired by an AI-controlled ship		
;; (this is scaled by ai_class)											
$AI Countermeasure Firing Chance: 0.2, 0.3, 0.5, 0.9, 1.1				
																		
;; seconds to add to the time it takes for an enemy to come in range of	
;; (i.e. target) a friendly ship										
$AI In Range Time: 2, 1.4, 0.75, 0, -1									
																		
;; AI ships will link ballistic primaries if ammo levels are greater	
;; than these percents													
$AI Always Links Ammo Weapons: 95, 80, 60, 40, 20						
$AI Maybe Links Ammo Weapons: 90, 60, 40, 20, 10						
																		
;; Multiplier that modifies the length and frequency of bursts used		
;; by the AI for ballistic primary weapons								
$Primary Ammo Burst Multiplier: 0, 0, 0, 0, 0							
																		
;; AI ships will link laser primaries if energy levels are greater		
;; than these percents													
$AI Always Links Energy Weapons: 100, 80, 60, 40, 20					
$AI Maybe Links Energy Weapons: 90, 60, 40, 20, 10						
																		
;; maximum number of missiles allowed to be homing in on a player at a	
;; given time (single-player only; no restriction in multiplayer)		
$Max Missiles Locked on Player: 2, 3, 4, 7, 99							
																		
;; maximum number of ships allowed to be attacking the player at a		
;; given time (single-player only; no restriction in multiplayer)		
$Max Player Attackers: 2, 3, 4, 5, 99									
																		
;; maximum number of active (i.e. 'thrown') asteroids that can be		
;; heading toward a friendly ship at any given time						
$Max Incoming Asteroids: 3, 4, 5, 7, 10									
																		
;; factor applied to damage suffered by the player						
$Player Damage Factor: 0.25, 0.5, 0.65, 0.85, 1							
																		
;; factor applied to subsystem damage suffered by the player			
;; (in addition to Player Damage Factor)								
$Player Subsys Damage Factor: 0.2, 0.4, 0.6, 0.8, 1						
																		
;; measure of time (in F1_0 units) after which the AI will recalculate	
;; the position of its target											
$Predict Position Delay: 2, 1.5, 1.333, 0.5, 0							
																		
;; seconds between each instance of an AI ship managing its shields		
$AI Shield Manage Delay: 5, 4, 2.5, 1.2, 0.1							
																		
;; factor applied to 'fire wait' for friendly ships						
$Friendly AI Fire Delay Scale: 2, 1.4, 1.25, 1.1, 1						
																		
;; factor applied to 'fire wait' for hostile ships						
$Hostile AI Fire Delay Scale: 4, 2.5, 1.75, 1.25, 1						
																		
;; factor applied to 'fire wait' for secondaries of friendly ships		
$Friendly AI Secondary Fire Delay Scale: 0.4, 0.6, 0.8, 1.0, 1.2		
																		
;; factor applied to 'fire wait' for secondaries of hostile ships		
$Hostile AI Secondary Fire Delay Scale: 1.4, 1.2, 1.0, 0.8, 0.6			
																		
;; factor applied to time it takes for enemy ships to turn				
$AI Turn Time Scale: 3, 2.2, 1.6, 1.3, 1								
																		
;; Percentage of the time where AI ships will use the glide attack		
;; when it is an option.												
$Glide Attack Percent: 0, 0, 0, 0, 0									
																		
;; Percentage of the time where AI ships will use circle strafe			
;; when it is an option.												
$Circle Strafe Percent: 0, 0, 0, 0, 0									
																		
;; Percentage of the time where AI ships will use glide to strafe		
;; capital ships when it is an option.									
$Glide Strafe Percent: 0, 0, 0, 0, 0									
																		
;; Percentage of the time where AI ships will randomly sidethrust in a	
;; dogfight.															
$Random Sidethrust Percent: 0, 0, 0, 0, 0								
																		
;; The amount of time required for the AI to detect 					
;; (and try to break) dogfight stalemate.								
$Stalemate Time Threshold: 0, 0, 0, 0, 0								
																		
;; The maximum distance the AI and target must be within				
;; for a dogfight stalemate												
$Stalemate Distance Threshold: 0, 0, 0, 0, 0							
																		
;; Sets the factor applied to the speed at which the player's shields	
;; recharge																
$Player Shield Recharge Scale: 4, 2, 1.5, 1.25, 1						
																		
;; factor applied to the speed at which the player's weapons recharge	
$Player Weapon Recharge Scale: 10, 4, 2.5, 2, 1.5						
																		
;; maximum number of turrets on one ship allowed to be attacking a		
;; target at a given time												
$Max Turret Target Ownage: 3, 4, 7, 12, 19								
																		
;; maximum number of turrets on one ship allowed to be attacking the	
;; player at a given time												
$Max Turret Player Ownage: 3, 4, 7, 12, 19								
																		
;; the minimum percentage of the total assessed damage a player		 	
;; must inflict in order to be awarded a kill							
$Percentage Required For Kill Scale: 0.30, 0.30, 0.30, 0.30, 0.30		
																		
;; the minimum percentage of the total assessed damage a player		 	
;; must inflict in order to be awarded an assist						
$Percentage Required For Assist Scale: 0.15, 0.15, 0.15, 0.15, 0.15		
																		
;; in TvT and Coop missions all teammates will be granted this 		 	
;; percentage of the capships score when someone scores a kill			
$Percentage Awarded For Capship Assist: 0.1, 0.2, 0.35, 0.5, 0.6		
																		
;; the amount to subtract from the player's score if they are			
;; repaired by a support ship											
$Repair Penalty: 10, 20, 35, 50, 60										
																		
;; time delay after bombs have been fired before they can collide		
;; with other weapons (ie. be shot down)								
$Delay Before Allowing Bombs to Be Shot Down: 1.5, 1.5, 1.5, 1.5, 1.5	
																		
;; Chance AI has to fire missiles at player is (value + 1) / 7 in every	
;; 10 second interval													
$Chance AI Has to Fire Missiles at Player:	0, 1, 2, 3, 4				
																		
;; The maximum amount of delay allowed before the AI will update its	
;; aim. Applies for small ships vs small ships							
$Max Aim Update Delay: 0, 0, 0, 0, 0									
																		
;; The maximum amount of delay allowed before turret AI will update its	
;; aim. Applies for turrets vs small ships								
$Turret Max Aim Update Delay: 0, 0, 0, 0, 0								
																		
;; Size of the player autoaim cone for each difficulty level  			
;; Only affects the player. If the ship has autoaim, the wider FOV value
;; will be used. Uses convergence.										
$Player Autoaim FOV: 0, 0, 0, 0, 0										
																		
;; The multiplier that affects at what range LOD switching will occur.	
;; NOTE THAT THIS IS NOT BY DIFFICULTY LEVEL (it's by model detail level
;; in the Options menu)                                                 
$Detail Distance Multiplier: 0.125, 0.25, 1.0, 4.0, 8.0					
																		
;; General AI-related flags.  These were previously all lumped together	
;; under the New AI mission flag.										
																		
																		
;; if set, big ships can attack a beam turret that's firing on them		
;; from a ship that they don't currently have targeted.					
$big ships can attack beam turrets on untargeted ships: NO				
																		
;; if set, enables the new primary weapon selection method				
$smart primary weapon selection: NO										
																		
;; if set, enables the new secondary weapon selection method (including	
;; proper use of bomber+ missiles)										
$smart secondary weapon selection: NO									
																		
;; if set, shields will devote all their charging energy to the weakest	
;; quadrant(s) and not waste energy on fully-charged quadrants			
;; (previously was -smart_shields on the command line)					
$smart shield management: NO											
																		
;; if set, the AI will properly use brief pulses of afterburner power	
;; instead of afterburning until fuel is exhausted						
$smart afterburner management: NO										
																		
;; if set, allows an AI ship to switch to rapid fire for dumbfire		
;; missiles																
$allow rapid secondary dumbfire: NO										
																		
;; if set, causes huge turret weapons (including anti-capship beams) to	
;; not target bombs														
$huge turret weapons ignore bombs: NO									
																		
;; if set, removes the random turret fire delay (from .1 to .9 seconds)	
;; inserted in addition to AI Fire Delay Scale							
$don't insert random turret fire delay: NO								
																		
;; if set, triggers a hack to improves the accuracy of non-homing swarm	
;; missiles by firing them along the turret's last fire direction		
;; rather than the direction it currently faces							
$hack improve non-homing swarm turret fire accuracy: NO					
																		
;; if set, shockwaves will cause damage to small ship subsystems		
;; (like in FS1)														
$shockwaves damage small ship subsystems: NO							
																		
;; if set, ships will not be able to engage their jump drive if their	
;; navigation subsystem is damaged or destroyed							
$navigation subsystem governs warpout capability: NO					
																		
;; if set, will not use a minimum speed limit for docked ships			
;; (like in FS1)														
$ignore lower bound for minimum speed of docked ship: NO				
																		
;; if set, will remove the increased delay when weapons are linked		
$disable linked fire penalty: NO										
																		
;; if set, will not scale weapon damage according to capital/supercap	
;; (like in FS1)														
$disable weapon damage scaling: NO										
																		
;; if set, will add the weapon velocity to the firing ship's velocity	
$use additive weapon velocity: NO										
																		
;; if set, will dampening closer to real newtonian physics				
$use newtonian dampening: NO											
																		
;; if set, beam damage is counted when calculating kills and assists 	
$include beams for kills and assists: NO								
																		
;; if set, kills gain score based on the percentage damage the killer	
;; inflicted on the dead ship											
$score kills based on damage caused: NO									
																		
;; if set, kills gain score based on the percentage damage the player	
;; gaining the assist inflicted on the dead ship						
$score assists based on damage caused: NO								
																		
;; if set, players (rather than just their team) can gain score from 	
;; events in multiplayer												
$allow event and goal scoring in multiplayer: NO						
																		
;; if set, the AI will properly link primaries according to				
;; specified percentages of energy levels, instead of retail behavior	
;; where it mistakenly linked according to absolute energy levels		
$fix linked primary weapon decision bug: NO								
																		
;; if set, prevents turrets from targeting bombs beyond maximum			
;; range of the weapons of the turret									
$prevent turrets targeting too distant bombs: NO						
																		
;; if set, prevents turrets from trying to target subsystems beyond		
;; their fov limits, also keeps the turret subsystem targeting			
;; preference order intact regardless of the angle to the target		
$smart subsystem targeting for turrets: NO								
																		
;; if set, heat-seeking missiles will not home in on stealth ships		
;; (this mirrors the established behavior where heat-seeking missiles	
;; do not home in on ships that are hidden from sensors)				
$fix heat seekers homing on stealth ships bug: NO						
																		
;; allow a player to commit into a multiplayer game without primaries	
$multi allow empty primaries:		NO									
																		
;; allow a player to commit into a multiplayer game without secondaries	
$multi allow empty secondaries:		NO									
																		
;; if set, allows turrets target other weapons than bombs assuming		
;; it is within allowed target priorities								
$allow turrets target weapons freely: NO								
																		
;; if set forces turrets to use only the set turret fov limits and		
;; ignore hard coded limits (with 'fire_down_normals' flag)				
$use only single fov for turrets:		NO								
																		
;; allow AI ships to dodge weapons vertically as well as horizontally	
$allow vertical dodge:	NO												
																		
;; If set makes beam turrets use same FOV rules as other weapons do.	
;; Prevents beam from a turret from following the target beyond the		
;; turret's FOV.														
$force beam turrets to use normal fov: NO								
																		
;; Fixes a bug where AI class is not properly set if set in the mission	
;; This should be YES if you want anything in AI.tbl to mean anything	
$fix AI class bug:	NO													
																		
;; TBD																	
$turrets ignore targets radius in range checks: NO						
																		
;; If set, the AI will NOT make extra effort to avoid ramming the player
;; during dogfights. This results in more aggressive AI.				
$no extra collision avoidance vs player:	NO							
																		
;; If set, the game will not check if the dying ship is a player's		
;; wingman, or if the maximum number of screams have been played, or 	
;; even if the dying ship is on the player's team before making it give	
;; a death scream										 				
$perform fewer checks for death screams:	NO							
																		
;; TBD																	
$advanced turret fov edge checks: NO									
																		
;; TBD																	
$require turrets to have target in fov: NO								
																		
;; If set, allows shield management for all ships						
;; (including capships).												
$all ships manage shields:				NO								
																		
;; If set, ai aims using ship center instead of first gunpoint			
$ai aims from ship center:			NO									
																		
;; If set, allows AI fighters to link their weapons at the beginning of	
;; a mission instead of imposing a delay of 30s to 120s					
$allow primary link at mission start:	NO								
																		
;; If set, prevents beams from instantly killing all weapons from first	
;; hit, instead allows weapon hitpoints to be used instead				
$allow beams to damage bombs:			NO								
																		
;; TBD																	
$disable weapon damage scaling for player:	NO							
																		
;; TBD																	
$countermeasures affect aspect seekers:	NO								
																		
;; TBD																	
$ai path mode:	normal													
																		
;; TBD																	
$no warp camera:	NO													
																		
;; If set, this flag overrides the retail behavior whereby a ship		
;; assigned to guard a ship in a wing will instead guard the entire wing
$ai guards specific ship in wing:	NO									
																		
#End																	
