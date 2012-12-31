#!/usr/bin/perl

while (<STDIN>) {
	if ($_ =~ /^\s*([^"\s][^\s]*[^"\s])\s+([^\s]+)\s+([^\s].*[^\s])\s*\.\s*$/) {
		print "$1 $2 $3 .\n";
	}
}
