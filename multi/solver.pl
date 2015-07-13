#!/usr/bin/perl -w
use strict;

my($trgtfol) = &make_folder($ARGV[0]);

my($log) = "$trgtfol/log.txt";
open OUT, ">>$log" or die "open failed:$!";
my($time);

my($trgt_i) = $ARGV[0];
my($trgt_o) = $ARGV[1];

#fdls
&set_time();
(system "./solver $trgt_i $trgt_o") == 0
    or die "solver faild :$?";
print OUT "time " . &print_time() . "\n";

close OUT;

#################################################
#      makedir
#################################################
sub make_folder{
	my($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdat) = localtime(time);
	my($date) = ($mon + 1) . "_" . $mday . "_" . $hour . "_" . $min . "_" .$sec;
	my($folname) = $_[0] . $date;
	
	mkdir $folname, 0777 or die "mkdir failed:$!";

	return $folname;
}

#################################################
#     time
#################################################

sub set_time{
	$time = (times)[2];
}

sub print_time{
	my($end) = (times)[2];
	my($long) = sprintf "%8f",($end - $time);
	return $long;
}
