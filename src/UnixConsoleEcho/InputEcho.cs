using System.Runtime.InteropServices;

namespace UnixConsoleEcho
{
    /// <summary>
    /// Provides static methods for enabling and disabling console input echo.
    /// </summary>
    public static class InputEcho
    {
        #if CoreCLR
        private static readonly bool s_isWindows =
            System.Runtime.InteropServices.RuntimeInformation
                .IsOSPlatform(System.Runtime.InteropServices.OSPlatform.Windows);
        #else
        private static readonly bool s_isWindows = true;
        #endif

        /// <summary>
        /// Disable console input echo. Has no effect on Windows.
        /// </summary>
        public static void Disable()
        {
            if (s_isWindows)
            {
                return;
            }

            InitializeConsoleBeforeRead(0, 10);
        }

        /// <summary>
        /// Enable console input echo. Has no effect on Windows.
        /// </summary>
        public static void Enable()
        {
            if (s_isWindows)
            {
                return;
            }

            UninitializeConsoleAfterRead();
        }

        [DllImport("libdisablekeyecho")]
        private static extern void InitializeConsoleBeforeRead(byte minChars = 1, byte decisecondsTimeout = 0);

        [DllImport("libdisablekeyecho")]
        private static extern void UninitializeConsoleAfterRead();
    }
}
