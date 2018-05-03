#!/usr/bin/env python
# -*- coding: utf-8 -*-
# test.py

from cmd import Cmd

class MyPrompt(Cmd):
    def do_help(self, args):
        if len(args) == 0:
            name = "   |'exec <testname>' or 'exec !<testnum>'\n   |0 BQ1\n   |1 BS1\n   |2 BA1\n   |3 BP1'"
        else:
            name = args
        print ("%s" % name)

    def do_exec(self, args):
        if (args == "!0"):
            print ("   |||TEST BQ1_ACTIVE")
        elif (args == "!1"):
            print ("   |||TEST BS1_ACTIVE")
        elif (args == "!2"):
            print ("   |||TEST BA1_ACTIVE")
        elif (args == "!3"):
            print ("   |||TEST BP3_ACTIVE")
        else:
            print ("invalid input")

    def do_quit(self, args):
        print ("Quitting.")
        return True

if __name__ == '__main__':
    prompt = MyPrompt()
    prompt.use_rawinput = False
    prompt.prompt = '$ '
    prompt.cmdloop('Running test tool.')
