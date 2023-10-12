## UID: 405935837

## Pipe Up
This program implements the shell pipe ``|`` operator.

## Building

To build the program, run:

``make``

## Running
Running ``./pipe date rev wc`` should give the same result as the shell command ``date | rev | wc``.

Running ``./pipe ls cat wc rev cat`` should give the same result as  the shell command ``ls | cat | wc | rev | cat``.


## Cleaning up
To clean up all binary files, run:

``make clean``