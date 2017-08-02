#!/bin/bash
if uname -a | grep -q 'MINGW'; then
	#windows  
	export GYP_MSVS_VERSION='2017'
fi
