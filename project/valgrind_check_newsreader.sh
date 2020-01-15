#!/bin/bash

valgrind --tool=memcheck --vgdb=no --num-callers=18 --error-exitcode=0 --exit-on-first-error=no --leak-check=full --show-leak-kinds=definite,reachable --errors-for-leak-kinds=definite,reachable --track-origins=yes ../build/bin/newsreader
