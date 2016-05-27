#!/bin/bash
#
# Set up path in torcs startup script.

shopt -s extglob

function updatepath () {
	fn=${SETUP_INSTALLPATH}/$1

	# rip off last "/"
	ip=${SETUP_INSTALLPATH/%\//}

	# replace all "/" with "\/" to use with regexp.
	fn2=${ip//\//\\\/}

ed -s $fn <<EOF
,s/@prefix@/$fn2/g
,wq
EOF
}

for f in accc nfs2ac nfsperf texmapper trackgen;
do updatepath $f ;
done

