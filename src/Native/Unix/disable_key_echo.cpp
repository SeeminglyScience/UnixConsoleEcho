// The majority of this file is taken from the corefx repo. The original version is available here:
// https://github.com/dotnet/corefx/blob/0325187c9d2ef504a2adf984f94655408ec1315e/src/Native/Unix/System.Native/pal_console.cpp#L1

#include <assert.h>
#include <stdint.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "disable_key_echo.h"


static bool g_readInProgress = false;        // tracks whether a read is currently in progress, such that attributes have been changed
static bool g_signalForBreak = true;         // tracks whether the terminal should send signals for breaks, such that attributes have been changed
static struct termios g_preReadTermios = {}; // the original attributes captured before a read; valid if g_readInProgress is true
static struct termios g_currTermios = {};    // the current attributes set during a read; valid if g_readInProgress is true

static void IncorporateBreak(struct termios *termios, int32_t signalForBreak)
{
    assert(termios != nullptr);
    assert(signalForBreak == 0 || signalForBreak == 1);

    if (signalForBreak)
        termios->c_lflag |= static_cast<uint32_t>(ISIG);
    else
        termios->c_lflag &= static_cast<uint32_t>(~ISIG);
}

// In order to support Console.ReadKey(intecept: true), we need to disable echo and canonical mode.
// We have two main choices: do so for the entire app, or do so only while in the Console.ReadKey(true).
// The former has a huge downside: the terminal is in a non-echo state, so anything else that runs
// in the same terminal won't echo even if it expects to, e.g. using Process.Start to launch an interactive,
// program, or P/Invoking to a native library that reads from stdin rather than using Console.  The second
// also has a downside, in that any typing which occurs prior to invoking Console.ReadKey(true) will
// be visible even though it wasn't supposed to be.  The downsides of the former approach are so large
// and the cons of the latter minimal and constrained to the one API that we've chosen the second approach.
// Thus, InitializeConsoleBeforeRead is called to set up the state of the console, then a read is done,
// and then UninitializeConsoleAfterRead is called.
extern "C" void InitializeConsoleBeforeRead(uint8_t minChars, uint8_t decisecondsTimeout)
{
    struct termios newTermios;
    if (tcgetattr(STDIN_FILENO, &newTermios) >= 0)
    {
        if (!g_readInProgress)
        {
            // Store the original settings, but only if we didn't already.  This function
            // may be called when the process is resumed after being suspended, and if
            // that happens during a read, we'll call this function to reset the attrs.
            g_preReadTermios = newTermios;
        }

        newTermios.c_iflag &= static_cast<uint32_t>(~(IXON | IXOFF));
        newTermios.c_lflag &= static_cast<uint32_t>(~(ECHO | ICANON | IEXTEN));
        newTermios.c_cc[VMIN] = minChars;
        newTermios.c_cc[VTIME] = decisecondsTimeout;
        IncorporateBreak(&newTermios, g_signalForBreak);

        if (tcsetattr(STDIN_FILENO, TCSANOW, &newTermios) >= 0)
        {
            g_currTermios = newTermios;
            g_readInProgress = true;
        }
    }
}

extern "C" void UninitializeConsoleAfterRead()
{
    if (g_readInProgress)
    {
        g_readInProgress = false;

        int tmpErrno = errno; // preserve any errors from before uninitializing
        IncorporateBreak(&g_preReadTermios, g_signalForBreak);
        int ret = tcsetattr(STDIN_FILENO, TCSANOW, &g_preReadTermios);
        assert(ret >= 0); // shouldn't fail, but if it does we don't want to fail in release
        (void)ret;
        errno = tmpErrno;
    }
}
