#!/bin/sh

if test `uname -s` = "Linux"; then
	echo "root";
else
	echo "wheel";
fi;
