package Smf;

use strict;
use warnings;

use WWW::Mechanize;

my $SLEEPTIME = 10; # Set to a high enough value to keep the warning from occuring

# These must be defined before posting
my $homeurl;
my $board;
my $boardurl;
my $newtopicurl;
my $username;
my $password;
my $mech;

sub set_homeurl
{
	$homeurl = shift;
}

sub set_board
{
	$board = shift;
	$boardurl = $homeurl . "/index.php?board=" . $board;
	$newtopicurl = $homeurl . "/index.php?action=post;board=" . $board;
}

sub set_username
{
	$username = shift;
}

sub set_password
{
	$password = shift;
}

sub set_sleeptime
{
	$SLEEPTIME = shift;
}

sub init_mech
{
	$mech = WWW::Mechanize->new();
	$mech->agent_alias( 'Windows Mozilla' );
}

sub login
{
	# Call out to Mechanize to log in.
	if(!$mech)
	{
		init_mech();
	}
	
	$mech->get($homeurl);
	
	$mech->submit_form(
		form_number => 1,
		fields => {
			user => $username,
			passwrd => $password,
			cookielength => 60,
		}
	);
}

sub post
{
	my ($subject, $message) = @_;
	my $form;
	
	login();
	
	# Sleep so that you don't set off the SMF post delay warning
	sleep($SLEEPTIME);
	
	$mech->get($newtopicurl);
	$mech->agent_alias( 'Windows Mozilla' );
	$mech->form_with_fields(qw(message subject));
	$mech->field("message", $message);
	$mech->field("subject", $subject);

	# Debugging
#	$form = $mech->current_form();
#	print $form->dump;
	
	$mech->click("post");
	
	# More debugging
#	print $mech->content() . "\n";
}

1;