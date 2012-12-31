#!/usr/bin/perl

while (<STDIN>) {
	$line = $_;
	$line =~ s/\b_:([^\s]+)\b/<tag:weavej3\@rpi.edu,2012:\/blank#$1>/g;
	print "$line";
}
