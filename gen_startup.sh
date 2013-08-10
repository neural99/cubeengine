#!/bin/sh

a=$(cat *.c | grep STARTUP_PROC | sed 's/STARTUP_PROC/ADD_PROC/' | tr '\n' ' ')

sed "s/__CALLS__/$a/" startup.c.template > startup.c

