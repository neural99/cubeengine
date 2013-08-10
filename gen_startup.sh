#!/bin/sh

a=$(cat src/*.c | grep STARTUP_PROC | sed 's/STARTUP_PROC/ADD_PROC/' | tr '\n' ' ')

sed "s/__CALLS__/$a/" src/startup.c.template > src/startup.c

