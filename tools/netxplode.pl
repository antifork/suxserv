#!/usr/bin/perl

# #!/usr/bin/perl -w   # not for production use, slower

# netxplode 0.3 - The Network Daemon Exploder
# Copyright (C) 2001  Daniel Dent
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# I may be contacted at ddent@vc.bc.ca, and although I do read all email,
# I cannot guarantee a personal response to all questions due to time
# constraints.

# NOTE: I AM NOT RESPONSIBLE FOR YOUR USE OF THIS PROGRAM. PLEASE DO NOT ABUSE.


use constant clients => 100;
my @servers = ( "homes.vejnet.org:6667" );
		
my @onceactions = (
		   "PRIVMSG nickserv :register moomilk ircd\@localhost \nJOIN #netxplodeRAND\nPRIVMSG chanserv :register #netxplodeRAND moomilksux xyz\n",
	           "JOIN #clones\n",
		   );
my @actions = (
#	       "PRIVMSG netxplodeRAND :hehe I am a cow\n",
#	       "PRIVMSG netxplodeRAND :hehe I am a cow\n",
#	       "PRIVMSG chanserv :help\n",
#	       "WHOIS memoserv\n",  
#	       "WHOIS chanserv\n",
#	       "WHOIS netxplodeRAND\n",
#	       "PRIVMSG netxplodeRAND :hi\n",
#	       "PRIVMSG netxplodeRAND :mooloadbot\n",
#	       "PRIVMSG #netxplodeRAND :mark - TIME\n",
	       "JOIN #netxplodeRAND\n", 
	       "JOIN #netxplodeRAND\n",
	       "JOIN #netxplodeRAND\n",
	       "JOIN #netxplodeRAND\n",
	       "JOIN #netxplodeRAND\n", 
	       "JOIN #netxplodeRAND\n",
	       "JOIN #netxplodeRAND\n",
	       "JOIN #netxplodeRAND\n",
	       "JOIN #netxplodeRAND\n",
	       "NICK netxplodeRAND\n",
	       "NICK netxplodeRAND\n",
	       "NICK netxplodeRAND\n",
#	       "JOIN #netxplodeRAND\n",
#	       "PRIVMSG #clones :mark - TIME [RAND]\n",
#	       "PRIVMSG #sux :sux RAND TIME\n",
#	       "CS help\n",
#	       "CS INFO #netxplodeRAND\n",
#	       "NS HELP\n",
#	       "SS SEEN netxplodeRAND\n",
#	       "SILENCE *!*\@mooRAND\n",
#	       "PRIVMSG #netxplodeRAND :suuxxx RAND SUUUUXXXX\n",
#	       "WHO *RAND*\n",
               "INFO services.*\n",
               "INFO services.*\n",
               "INFO services.*\n",
               "INFO services.*\n",
#	       "LIST *RAND*\n",
#	       "ADMIN services.*\n",
	       "MOTD services.*\n",
	       "MOTD services.*\n",
	       "MOTD services.*\n",
	       "MOTD services.*\n",
#	       "USERHOST netxplodeRAND\n",
#	       "WHOWAS netxplodeRAND\n",
	       "AWAY awayRAND\n",
	       "AWAY\n",
	       "TOPIC #clones :sux RAND sux\n",
#	       "LINKS *RAND*\n",
	       "LUSERS x services.*\n"
);


use constant io_buf_size => 8192;

#use constant io_buf_size => 1024;
#use constant rand_after_replay => 0;

use constant stickyselections => 1;

my $testmode;		
if ($ARGV[0]) {
    $testmode = $ARGV[0];
} else {
    $testmode = "random";
}

my %replaybuffer;

#if ($testmode ne "replay") {
#    open(REPLAY, ">replay.log");
#print REPLAY "LOG STARTED AT " . time() . "\n";
#}
		
#open(XPLOSION, ">xplosion.log");
#print XPLOSION "LOG STARTED AT " . time() . "\n";
		
##############################################################################

use strict;
#use warnings;

use IO::Select;
use IO::Socket;

my $sel = IO::Select->new();

use constant version => "0.3";
my $dienow = 0;
my %connections;

# hot pipes!
$| = 1;

print "The Network Daemon Exploder " . version . " by Daniel Dent.  (C) 2001.\n";

sub SelectServer {
    return split ( /:/, $servers[ int( rand scalar $#servers + 0.5 ) ] );
}

sub GetNick {

    # possibly some file magic later
    my $randnick = "netxplode";
    for ( 1 .. 5 ) { $randnick .= int( rand scalar 9 + 0.5 ); }
    return $randnick;
}

sub ReadLog {
    my $currentconn;
    my @currentconnparams;
    open( REPLAYLOG, $ARGV[1] );
    while (<REPLAYLOG>) {
	next if (/^LOG STARTED/);
	next if (/PONG/);
	next if (/^\n/);
        if (s/^--> //) {
	    @currentconnparams = split(/ /);
	    $currentconn = $currentconnparams[1];
#	    print "MOOO $currentconn\n";
	} else {
	    push(@{ $replaybuffer{$currentconn} }, $_);
	}
    }
}

sub NewConnection {
    my ( $num, $server, $port, $nick ) = @_;
    ( $server, $port ) = SelectServer() if ($server eq "" || $port eq "");
    $nick = GetNick() if ($nick eq "");
    
    my $sock = IO::Socket::INET->new(
      PeerAddr => $server,
      PeerPort => $port,
#      Proto    => "tcp" ) or NewConnection() and return;
          Proto    => "tcp" ) or return;
    #      Proto    => "tcp" ) or die "opening server socket: $!";
    $connections{$sock}{'server'} = $server;
    $connections{$sock}{'port'}   = $port;
    $connections{$sock}{'num'}    = $num;
    $connections{$sock}{'nick'}   = $nick;
    QueueWrite( $sock, "NICK $nick\n" );
    QueueWrite( $sock, "USER $nick $nick $nick :netxplode version " . version . " by Daniel Dent\n" );

    $sel->add($sock);
}

sub CloseConnection {
    my $sock = shift;

    $sel->remove($sock);
    $sock->close;
#    print STDERR "--> D $connections{$sock}{'num'} $connections{$sock}{'server'} $connections{$sock}{'port'} $connections{$sock}{'nick'}\n";
#    print XPLOSION "--> D " . time() . " $connections{$sock}{'num'} $connections{$sock}{'server'} $connections{$sock}{'port'} $connections{$sock}{'nick'}\n";
    my $num    = $connections{$sock}{'num'};
    my $server = $connections{$sock}{'server'};
    my $port   = $connections{$sock}{'port'};
    my $nick   = $connections{$sock}{'nick'};
    delete $connections{$sock};
    if (stickyselections) {
	NewConnection($num, $server, $port, $nick);
    } else {
	NewConnection($num, "", "", "");
    }
}

sub ProcessRead {
    my $sock = shift;
    my $data;
    my $read = sysread( $sock, $data, io_buf_size );
    unless ($read) { CloseConnection($sock); return; }
    $read-- if chomp $data;    # compensate for taking out the newline

#    print "--> R $connections{$sock}{'num'} $connections{$sock}{'server'} $connections{$sock}{'port'} $connections{$sock}{'nick'} $read \n" . $data . "\n";
#    print XPLOSION "--> R " . time() . " $connections{$sock}{'num'} $connections{$sock}{'server'} $connections{$sock}{'port'} $connections{$sock}{'nick'} $read \n" . $data . "\n";
    
    # things which need an auto-reply go here
    foreach my $lineread (split(/\n/, $data)) {
	
	if ( substr( $lineread, 0, 4 ) eq "PING" ) {
	    on_ping( $sock, $lineread );
	}

	if ( substr( $lineread, 0, 5 ) eq "ERROR" ) {
	    CloseConnection($sock);
	    return;
	}
	
    }
}

sub on_ping {
    my ( $sock, $data ) = @_;
    if ($testmode ne "replay") { 
	QueueWrite( $sock, "PONG :" . substr( $data, 6 ) );
    } else {
	push(@{ $replaybuffer{$connections{$sock}{'num'}} }, "PONG :" . substr( $data, 6 ) );
    }
}

sub QueueWrite {
    my ( $sock, $data ) = @_;
    push ( @{ $connections{$sock}{'bufout'} }, $data );
}

sub FiveRand {
    my $randnum = "";
    for ( 1 .. 5 ) {
        $randnum .= int( rand scalar 9 + 0.5 );
    }
    return $randnum;
}

sub RandOnceAction {
    my $sock     = shift;
    my $fiverand = FiveRand();
    my $time     = time();
    my $rand     = int( rand scalar $#onceactions + 0.5 );

    unless ( $connections{$sock}{onceactions}{$rand} ) {
        $connections{$sock}{onceactions}{$rand} = 1;
        $_ = $onceactions[$rand];
        s/RAND/$fiverand/g;
        s/TIME/$time/g;
        return $_;
    }
    else {
        return RandAction();
    }
}

sub RandAction {
    my $rand = FiveRand();
    my $time = time();
    $_ = $actions[ int( rand scalar $#actions + 0.5 ) ];
    s/RAND/$rand/g;
    s/TIME/$time/g;
    return $_;
}

sub GetAction {
    my $sock = shift;
    if ( int( rand scalar 3 + 0.5 ) eq 1 ) {
        return RandOnceAction($sock);
    }
    else {
        return RandAction();
    }
}

sub ProcessWrite {
    my $sock = shift;
    my $data;
    unless ( $sel->exists($sock) ) { return; }

    if ($testmode ne "replay") {
	$data = shift @{ $connections{$sock}{'bufout'} };
	unless ($data) {
	    select( undef, undef, undef, 0.25 );
#	    select( undef, undef, undef, rand(1) );
	    $data = GetAction($sock);   
	}
    }
    
    if ($testmode eq "replay") {
	select( undef, undef, undef, 0.25 );
#	select( undef, undef, undef, rand(1) );
	$data = shift @{ $replaybuffer{$connections{$sock}{'num'}} };
    }
	    
    if ($data) {
#	print "--> W $connections{$sock}{'num'} $connections{$sock}{'server'} $connections{$sock}{'port'} $connections{$sock}{'nick'} " . length($data) . "\n" . $data . "\n";
#	print XPLOSION "--> W " . time() . " $connections{$sock}{'num'} $connections{$sock}{'server'} $connections{$sock}{'port'} $connections{$sock}{'nick'} " . length($data) . "\n" . $data . "\n";
#	print REPLAY "--> " . time() . " $connections{$sock}{'num'} $connections{$sock}{'server'} $connections{$sock}{'port'} $connections{$sock}{'nick'} " . length($data) . "\n" . $data . "\n" if ( $testmode ne "replay" );
        syswrite( $sock, $data, length($data) );
        $connections{$sock}{'lastmsg'} = time();
    }
}

#### MAIN ############################################################

#$SIG{INT} = sub {
#    close(XPLOSION);
#    close(REPLAY);
#    die "Aieeeeeee. SIG TERM.";
#}

if ( $testmode eq "replay" ) {
    ReadLog();
}

for ( 1 .. clients ) { NewConnection($_, "", "", ""); }
print "Connections established.\n";

# now for the actual loop.
while ( my @handles = IO::Select::select( $sel, $sel, $sel, 1 ) ) {

    # remove any sockets that are in error
    my %removed;
    foreach my $sock ( @{ $handles[2] } ) {

        CloseConnection($sock);
        $removed{$sock} = 1;
    }

    select( undef, undef, undef, rand(1) );
    
    # get input from each active socket
    READABLE_SOCKET:
    foreach my $sock ( @{ $handles[0] } ) {

        # make sure that that socket hasn't gone *blammo* yet
        next READABLE_SOCKET if exists $removed{$sock};
        ProcessRead($sock);
    }

    # write to each active socket
    WRITABLE_SOCKET:
    foreach my $sock ( @{ $handles[1] } ) {

        # make sure that that socket hasn't gone *blammo* yet
        next WRITABLE_SOCKET if exists $removed{$sock};
        ProcessWrite($sock);
    }
}
