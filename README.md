# csci350-nachos

This repo contains our team's CSCI 350 project for Fall 2015. Our team members are:

    Nicholas Roubal     nroubal@usc.edu
    Yetsebaot Sisay     sisay@usc.edu
    John Timms          timms@usc.edu

Tags are available for seeing each finished assignment.

# General Usage

To build all of Nachos run `gmake` from this directory. You can also run `gmake` from any sub-folder to build just that code / portion of the project.

To demonstrate Assignment 1 (tag v1.0) you must be in the `threads` directory. From `threads`, run `nachos -T` (for the Part 2 test suite) or `nachos -P2` (for a demonstration of the Passport Office with a menu).

To build the Nachos user programs that are found in the `test` directory (part of Assignment 2) you will need to temporarily add the MIPS cross-compiler from the `gnu` directory to your `PATH`. Execute either `export PATH=../gnu/:$PATH` (for Bash shells) or `setenv PATH ../gnu/:$PATH` (for other shells) once each time you log in. Then, from the `test` directory, run `gmake`.

To demonstrate Assignment 2 (tag v2.0) user programs, go to the `userprog` directory and run `nachos -x ../test/*`, replacing `*` with the user program you wish to run (`halt`, `testfiles`, etc.).

**(Note: This general test was removed after tag v2.0)** To run a general networking test (see assignment document for Assignment 3) open two terminals and run `nachos -m 0 -o 1` in one and `nachos -m 1 -o 0` in the other (do this from the `networking` directory). The `-m` identifies the machine you're starting, the `-o` identifies the other machine you intend to communicate with. **(Note: right now, one terminal will display a Condition error from `synch.cc`. This warning has not yet been solved.)**
