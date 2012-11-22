#!/usr/bin/perl

sub pickbest {
	$last = shift;
	$cur = shift;
	$lastnum =()= $last =~ m/REPLICATE/gi;
	$curnum =()= $cur =~ m/REPLICATE/gi;
	if ($curnum != $lastnum) {
		return $curnum < $lastnum ? $cur : $last;
	}
	$lastnum =()= $last =~ m/PROBLEM/gi;
	$curnum =()= $cur =~ m/PROBLEM/gi;
	if ($curnum != $lastnum) {
		return $curnum < $lastnum ? $cur : $last;
	}
	$lastnum =()= $last =~ m/SPLIT/gi;
	$curnum =()= $cur =~ m/SPLIT/gi;
	if ($curnum != $lastnum) {
		return $curnum < $lastnum ? $cur : $last;
	}
	return $cur;
}

$last_asgn = '';
$cur_asgn = '';
while (<STDIN>) {
	if ($_ =~ m/^===== Solution \d+ =====$/) {
		if ($last_asgn eq '') {
			$last_asgn = $cur_asgn;
		} else {
			$last_asgn = &pickbest($last_asgn, $cur_asgn);
		}
		$cur_asgn = '';
	} else {
		$cur_asgn = $cur_asgn . $_;
	}
}
$last_asgn = &pickbest($last_asgn, $cur_asgn);
print $last_asgn;
