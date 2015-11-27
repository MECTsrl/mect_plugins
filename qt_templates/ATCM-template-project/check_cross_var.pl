#!/usr/bin/perl
use strict;
use warnings;

sub strip_comments {
	my $string=shift;
	$string =~ s#/\*.*?\*/##sg; #strip multiline C comments
	return $string;
}
my @uifiles = glob( './*.ui' );
my @ui_keyword = (
"variable",
"visibilityVar",
"passwordVar",
"statusVar",
"x1Variable",
"x1Min",
"x1Max",
"x1Step",
"y1Variable",
"y1Min",
"y1Max",
"y1Step",
"Display1",
"x2Min",
"x2Max",
"x2Step",
"y2Variable",
"y2Min",
"y2Max",
"y2Step",
"Display2"
);

my @cppfiles = glob( './*.cpp' );
my @cpp_keyword = (
"isBlockActive",
"getHeadBlockName",
"disconnectDeviceByVarname",
"connectDeviceByVarname",
"isDeviceConnectedByVarname",
"setFormattedVar",
"getString",
"setString",
"checkTagWriting",
"deactivateVar",
"activateVar",
"prepareWriteVar",
"prepareWriteBlock",
"readVar",
"writeVar",
"writeBlock",
"getVarDecimalByName",
"getVarDivisor",
"getVarDecimal",
"getFormattedVar"
);

# extract extract all variable from the crosstable
#print "Loading crosstable variables...\n";
my $srcdirname = $ARGV[0];
open(INFILE, $srcdirname."/config/Crosstable.csv") or die  "'".$srcdirname."/config/Crosstable.csv" . "': ". $!;
my $line;
my $file;
my $keyword;
my $string;
my @crosstable;
while ($line = <INFILE>)
{
	if ($line =~ /^[1-3];/)
	{
		$line =~ m/^[1-3];\w+;(\w+)/g;
		#print "#$1#\n";
		push (@crosstable, $1);
	}
}
close INFILE;

# extract from *.cpp *.ui files all possible crosstable variable used

#print "Checking for crosstable variables mismatch...\n";

foreach $file (sort @uifiles)
{
	#print "$file\n";
	open(INFILE, $file) or die $!;
	while ($line = <INFILE>)
	{
		foreach $keyword (@ui_keyword)
		{
			if ($line =~ /<property name=\"$keyword\">/)
			{
				$line = <INFILE>;
				if($line =~ m/<string>(\w+)<\/string>/g)
				{
				if ( $1 ~~ @crosstable)
				{
					#print "#$1 DONE#\n";
				}
				elsif ($1 ne '' && !($1 =~ /^-?(0|([1-9][0-9]*))(\.[0-9]+)?([eE][-+]?[0-9]+)?$/))
				{
					print "Error3: Undeclared variable '$1' into the Crosstable\n";
					print join("', '", @crosstable);
					close INFILE;
					exit 1;
				}
				}
			}
		}
	}
	close INFILE;
}

foreach $file (sort @cppfiles)
{
	#print "$file\n";

	open(INFILE, $file) or die $!;
	$string = "";
	while ($line = <INFILE>)
	{
		my @values = split('//', $line);
		$line = $values[0];
		$string .= $line;
	}
	close INFILE;

	$line = strip_comments($string);
	
	open TMP, "+>", undef or die $!;
	print TMP "$line";
	seek(TMP,0,0);

	while ($line = <TMP>)
	{
		my @values = split('//', $line);
		$line = $values[0];
		foreach $keyword (@cpp_keyword)
		{
			if ($line =~ /$keyword\(\"/)
			{
				#trim space
				$line =~ s/^\s+//;
				$line =~ s/\s+$//;
				if (!($line =~ /^\/\//))
				{
					# trim double quote
					$line =~ m/$keyword\(\"(\w+)/g;
					# if there was a double quote, is a real name and not a variable that cotain the name
					# so it will be cheked
					if ($1 ne $line)
					{
						#print "#$1#\n";
						if ( $1 ~~ @crosstable)
						{
							#print "#$1 DONE#\n";
						}
						elsif ($1 ne '' && !($1 =~ /^-?(0|([1-9][0-9]*))(\.[0-9]+)?([eE][-+]?[0-9]+)?$/))
						{
							print "Error1: Undeclared variable '$1' into the Crosstable [" . $file . "]\n";
							close TMP;
							exit 1;
						}
					}
				}
			}
		}
		if ($line =~ "variableList <<")
		{
			my @values = split('<<', $line);

			foreach my $val (@values) {

				#trim space
				$val =~ s/^\s+//;
				$val =~ s/\s+$//;
				# trim double quotes
				$val =~ s/^"(.*)"$/$1/;
				$val =~ s/^"(.*)";$/$1/;

				if ($val ne "variableList")
				{
				if ( $val ~~ @crosstable )
				{
					#print "#$val DONE#\n";
				}
				elsif ($val ne '' && !($val =~ /^-?(0|([1-9][0-9]*))(\.[0-9]+)?([eE][-+]?[0-9]+)?$/))
				{
					print "Error2: Undeclared variable '$val' into the Crosstable [" . $file . "]\n";
					close TMP;
					exit 1;
				}
				}
			}
		}
	}
	close TMP;
}

