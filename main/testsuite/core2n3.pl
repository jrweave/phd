#!/usr/bin/perl

sub replace_externals {
	my $body = shift;
	while ($body =~ /^((?:.*\s+)?)Not\(External\(pred:list-contains\(List\(([^\)]*)\)\s+([^\s]+)\)\)\)(.*)$/) {
		my $before = $1;
		my $args = $2;
		my $var = $3;
		my $after = $4;
		my $n3 = "";
		@a = split(/\s+/, $args);
		for $arg (@a) {
			$n3 .= " $var log:notEqualTo $arg ."
		}
		$body = "$before $n3 $after";
	}
	$body =~ s/External\(pred:list-contains\(List(\([^\)]*\))\s+([^\s]+)\)\)/$2 list:in $1 ./g;
	return $body;
}

sub handle_list {
	my $arglist = shift;
	if ($arglist =~ /^\s*$/) {
		return ":nil";
	}
	$arglist =~ /^\s*([^\s]+)(.*)$/;
	my $first = $1;
	my $rest = $2;
	return "[ :first $first ; :rest " . &handle_list($rest) . " ]";
}

sub handle_atom {
	my $subject = shift;
	my $arglist = shift;
	return "$subject :tuple " . &handle_list($arglist) . " .";
	#return "$subject :tuple ($arglist) .";
}

$rule = "";
print '@prefix : <tag:weavej3@rpi.edu,2012:/n3/> .' . "\n";
print '@prefix list: <http://www.w3.org/2000/10/swap/list#> .' . "\n";
print '@prefix log: <http://www.w3.org/2000/10/swap/log#> .' . "\n";
while (<STDIN>) {
	chomp;
	$line = $_;
	if ($line =~ m/^\s*Prefix\s*\(\s*(\w+)\s+<([^\s]+)>\s*\)\s*$/) {
		print "\@prefix $1: <$2> .\n";
	} elsif ($line =~ m/^\s*$/) {
		if ($rule =~ m/^\s*Forall\s*(?:\?\w+\s*)*\(\s*(.*?)\s*\)\s*$/) {
			$impl = $1;
			if ($impl =~ m/^(.*?)\s*:-\s*(.*?)$/) {
				$head = $1;
				$body = $2;
				$body =~ s/^\s*And\s*\((.*)\)\s*$/$1/g;
				$body = &replace_externals($body);
				$rule = "{ $head } <= { $body }";
				$rule =~ s/Not\(\s*([^\s]+)\s+=\s+([^\s]+)\s*\)/$1\[log:notEqualTo->$2\]/g;
				$rule =~ s/([^\s]+)\s+=\s+([^\s]+)/$1\[log:equalTo->$2\]/g;
				$rule =~ s/([^\s]+)\[([^\s]+)\s*->\s*([^\s]+)\]/$1 $2 $3 ./g;
				$rule =~ s/\b_(\w+)\b/:$1/g;
				while ($rule =~ /^(.*)\b([^\s]+)\(\s*([^\)]*)\s*\)(.*)$/) {
					$front = $1;
					$pred = $2;
					$args = $3;
					$back = $4;
					$rule = $front . &handle_atom($pred, $args) . $back;
				}
				print "$rule .\n";
			} else {
				die "[ERROR] Unexpected implication: " . $impl . "\n";
			}
		} elsif ($rule eq "") {
			# ignore
		} elsif ($rule =~ m/^\s*Forall/) {
			die "[ERROR] Cannot parse: " . $rule . "\n";
		} else {
			$rule =~ s/([^\s]+)\[([^\s]+)\s*->\s*([^\s]+)\]/$1 $2 $3 ./g;
			print "$rule\n";
		}
		$rule = "";
	} elsif ($line =~ m/^\s*\(\*.*\*\)\s*$/) {
		# ignore
	} elsif ($line =~ m/^\s*(#PRAGMA\s+.*)$/) {
		# ignore
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
