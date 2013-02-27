#!/usr/bin/perl

# Copyright 2012 Jesse Weaver
# 
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
# 
#        http://www.apache.org/licenses/LICENSE-2.0
# 
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
#    implied. See the License for the specific language governing
#    permissions and limitations under the License.

while (<STDIN>) {
	$line = $_;
	$line =~ s/\Q"http:\/\/www.w3.org\/1999\/02\/22-rdf-syntax-ns#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/rdf:$1/g;
	$line =~ s/\Q"http:\/\/www.w3.org\/2000\/01\/rdf-schema#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/rdfs:$1/g;
	$line =~ s/\Q"http:\/\/www.w3.org\/2002\/07\/owl#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/owl:$1/g;
	$line =~ s/\Q"http:\/\/www.w3.org\/2007\/rif-builtin-predicate#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/pred:$1/g;
	$line =~ s/\Q"http:\/\/www.w3.org\/2007\/rif-builtin-function#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/func:$1/g;
	$line =~ s/\Q"http:\/\/www.w3.org\/2007\/rif#\E([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/rif:$1/g;
	$line =~ s/\Q"\E(\w+)\Q"^^<http:\/\/www.w3.org\/2007\/rif#local>\E/_$1/g;
	$line =~ s/"tag:\/\/weavej3\@rpi\.edu,2012:\/([^\s]*?)\Q"^^<http:\/\/www.w3.org\/2007\/rif#iri>\E/mine:$1/g;
	$line =~ s/\Q"\E(-?\d+)\Q"^^<http:\/\/www.w3.org\/2001\/XMLSchema#integer>\E/$1/g;
	print $line;
}
