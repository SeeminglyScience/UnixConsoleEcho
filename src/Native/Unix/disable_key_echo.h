// The majority of this file is taken from the corefx repo. The original version is available here:
// https://github.com/dotnet/corefx/blob/0325187c9d2ef504a2adf984f94655408ec1315e/src/Native/Unix/System.Native/pal_console.cpp#L1

/**
 * Initializes the terminal in preparation for a read operation.
 */
extern "C" void InitializeConsoleBeforeRead(uint8_t minChars, uint8_t decisecondsTimeout);

/**
 * Restores the terminal's attributes to what they were before InitializeConsoleBeforeRead was called.
 */
extern "C" void UninitializeConsoleAfterRead();
