#!/usr/bin/perl

while (<STDIN>) {
	$line = $_;
	$line =~ s/\Q"http:\/\/www.w3.org\/1999\/02\/22-rdf-syntax-ns#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/rdf:$1/g;
	$line =~ s/\Q"http:\/\/www.w3.org\/2000\/01\/rdf-schema#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/rdfs:$1/g;
	$line =~ s/\Q"http:\/\/www.w3.org\/2002\/07\/owl#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/owl:$1/g;
	$line =~ s/\Q"http:\/\/www.w3.org\/2007\/rif-builtin-predicate#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/pred:$1/g;
	$line =~ s/\Q"http:\/\/www.w3.org\/2007\/rif-builtin-function#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/func:$1/g;
	$line =~ s/\Q"http:\/\/www.w3.org\/2007\/rif#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/rif:$1/g;
	$line =~ s/\Q"\E(-?\d+)\Q"^^<http:\/\/www.w3.org\/2001\/XMLSchema#integer>\E/$1/g;
	print $line;
}
