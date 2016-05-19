#!/usr/bin/perl
use strict;
use warnings;

# extract extract all variable from the crosstable
#print "Loading crosstable variables...\n";

my $projectfilename = $ARGV[0];
my $srcdirname = $ARGV[1];

my $type = "";
my $line;
open(INFILE, $srcdirname."/template.pri") or die  "'".$srcdirname."/template.pri" . "': ". $!;
while ($line = <INFILE>)
{
	if ($line =~ /^TYPE/)
	{
		my $key;
		( $key, $type ) = $line =~ m/(\w+)=(\w+)/; 
		last;
	}
}
close INFILE;

if ($type eq "")
{
	printf "Cannot found TYPE\n";
	exit 1;
}
print $type . "...\n";

open(INFILE, $srcdirname."/config/system.ini") or die  "'".$srcdirname."/config/system.ini" . "': ". $!;
my $retval = 0;
my $section;
while ($line = <INFILE>)
{
	my $key = "";
	my $value = "";
	if ($line =~ /\[([A-z0-9]+)\]/)
	{
		$section = $1;
		# printf "section: '" . $section . "'\n";
	}
	elsif ($line =~ /=/)
	{
		my @values = split('=', $line);
		$key = $values[0];
		$value = $values[1];
		$key =~ s/^\s+|\s+$//g;
		$value =~ s/^\s+|\s+$//g;
		
		if ($type eq "TP1043_01_A" || $type eq "TP1043_01_B" || $type eq "TP1043_01_C")
		{
			if ($section eq "SERIAL_PORT_3" && $key eq "baudrate" && $value ne "225000")
			{
				print "Error: for product '" . $type . "' in the section '" . $section . "' the parameter '" . $key . "' must be '" . $value . "'\n";
				$retval = 1;
			}
		}
		elsif ($type eq "TP1057_01_A" || $type eq "TP1057_01_B")
		{
			if ($section eq "CANOPEN_0" && $key eq "baudrate" && $value ne "125000")
			{
				print "Error: for product '" . $type . "' in the section '" . $section . "' the parameter '" . $key . "' must be '" . $value . "'\n";
				$retval = 1;
			}
		}
		elsif ($type eq "TP1070_01_A" || $type eq "TP1070_01_B" || $type eq "TP1070_01_C" || $type eq "TP1070_01_D")
		{
			if ($section eq "CANOPEN_0" && $key eq "baudrate" && $value ne "125000")
			{
				print "Error: for product '" . $type . "' in the section '" . $section . "' the parameter '" . $key . "' must be '" . $value . "'\n";
				$retval = 1;
			}
		}
		elsif ($type eq "TPAC1007_03" || $type eq "TPAC1007_04_AA" || $type eq "TPAC1007_04_AB" || $type eq "TPAC1007_04_AC")
		{
			if ($section eq "SERIAL_PORT_3" && $key eq "baudrate" && $value ne "225000")
			{
				print "Error: for product '" . $type . "' in the section '" . $section . "' the parameter '" . $key . "' must be '" . $value . "'\n";
				$retval = 1;
			}
		}
		elsif ($type eq "TPAC1008_02_AA" || $type eq "TPAC1008_02_AB" || $type eq "TPAC1008_02_AC" || $type eq "TPAC1008_02_AD" || $type eq "TPAC1008_02_AE" || $type eq "TPAC1008_02_AF")
		{
			if ($section eq "CANOPEN_0" && $key eq "baudrate" && $value ne "125000")
			{
				print "Error: for product '" . $type . "' in the section '" . $section . "' the parameter '" . $key . "' must be '" . $value . "'\n";
				$retval = 1;
			}
		}
	}
}
close INFILE;

exit $retval;

