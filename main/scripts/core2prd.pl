#!/usr/bin/perl

$rule = "";
while (<STDIN>) {
	chomp;
	$line = $_;
	if ($line =~ m/^\s*Prefix\s*\(\s*(\w+)\s+<([^\s]+)>\s*\)\s*$/) {
		$prefix{$1} = $2;
	} elsif ($line =~ m/^\s*$/) {
		if ($rule =~ m/^\s*Forall\s*(?:\?\w+\s*)*\(\s*(.*?)\s*\)\s*$/) {
			$impl = $1;
			if ($impl =~ m/^(.*?)\s*:-\s*(.*?)$/) {
				$newrule = "If " . $2 . " Then Do(Assert(" . $1 . "))";
				$newrule =~ s/\b(\d+)\b/"$1"^^xsd:integer/g;
				$newrule =~ s/\b(\w*):([\w-]+)\b/"$prefix{$1}$2"^^<http:\/\/www.w3.org\/2007\/rif#iri>/g;
				$newrule =~ s/\^\^"([^"]*)"\^\^<[^\s]+>/^^<$1>/g;
				print $newrule . "\n";
			} else {
				die "Unexpected implication: " . $impl . "\n";
			}
		} elsif ($rule eq "") {
			# ignore
		} else {
			$newrule = "If And() Then Do(Assert(" . $rule . "))";
			$newrule =~ s/\b(\d+)\b/"$1"^^xsd:integer/g;
			$newrule =~ s/\b(\w*):([\w-]+)\b/"$prefix{$1}$2"^^<http:\/\/www.w3.org\/2007\/rif#iri>/g;
			$newrule =~ s/\^\^"([^"]*)"\^\^<[^\s]+>/^^<$1>/g;
			print $newrule . "\n";
		}
		$rule = "";
	} elsif ($line =~ m/^\s*\(\*.*\*\)\s*$/) {
		# ignore
	} elsif ($line =~ m/^\s*(#PRAGMA\s+.*)$/) {
		$p = $1;
		$p =~ s/\b(\d+)\b/"$1"^^xsd:integer/g;
		$p =~ s/\b(\w*):([\w-]+)\b/"$prefix{$1}$2"^^<http:\/\/www.w3.org\/2007\/rif#iri>/g;
		$p =~ s/\^\^"([^"]*)"\^\^<[^\s]+>/^^<$1>/g;
		print $p . "\n"; # pass on expanded pragma
	} elsif ($line =~ m/^\s*(#.*)$/) {
		print $1 . "\n"; # pass comment through
	} else {
		$line =~ s/\s*(.*)\s*/$1/;
		if ($rule ne "") {
			$rule = $rule . " ";
		}
		$rule = $rule . $line;
	}
}
