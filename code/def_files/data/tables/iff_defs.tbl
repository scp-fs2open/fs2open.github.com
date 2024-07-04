																		
#IFFs																	
																		
;; Every iff_defs.tbl must contain a Traitor entry.  Traitors attack	
;; one another (required by the dogfighting code) but it is up to you	
;; to decide who attacks the traitor or whom else the traitor attacks.	
$Traitor IFF: Traitor													
																		
;------------------------												
; Friendly																
;------------------------												
$IFF Name: Friendly														
$Color: ( 0, 255, 0 )													
$Attacks: ( "Hostile" "Neutral" "Traitor" )						
$Flags: ( "support allowed" )											
$Default Ship Flags: ( "cargo-known" )								
																		
;------------------------												
; Hostile																
;------------------------												
$IFF Name: Hostile														
$Color: ( 255, 0, 0 )													
$Attacks: ( "Friendly" "Neutral" "Traitor" )						
+Sees Friendly As: ( 255, 0, 0 )										
+Sees Hostile As: ( 0, 255, 0 )											
																		
;------------------------												
; Neutral																
;------------------------												
$IFF Name: Neutral														
$Color: ( 255, 0, 0 )													
$Attacks: ( "Friendly" "Traitor" )									
+Sees Friendly As: ( 255, 0, 0 )										
+Sees Hostile As: ( 0, 255, 0 )											
+Sees Neutral As: ( 0, 255, 0 )											
																		
;------------------------												
; Unknown																
;------------------------												
$IFF Name: Unknown														
$Color: ( 255, 0, 255 )													
$Attacks: ( "Hostile" )												
+Sees Neutral As: ( 0, 255, 0 )											
+Sees Traitor As: ( 0, 255, 0 )											
$Flags: ( "exempt from all teams at war" )							
																		
;------------------------												
; Traitor																
;------------------------												
$IFF Name: Traitor														
$Color: ( 255, 0, 0 )													
$Attacks: ( "Friendly" "Hostile" "Neutral" "Traitor" )			
+Sees Friendly As: ( 255, 0, 0 )										
																		
#End																	
