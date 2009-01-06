using System;
using System.Diagnostics;
using System.IO;
using System.Collections;

namespace BeauSkinner.VerHeader
{
    /// <summary>
    /// Summary description for Class1.
    /// </summary>
    class VerHeader
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static int Main(string[] args)
        {
            if (args.Length == 0 || args.Length > 2)
            {
                Console.WriteLine("Usage: VerHeader <resource-file> [output-file=version.h]");
                return 1;
            }
            string RCFileStr = args[0];

            string outFileStr = "version.h";
            if (args.Length == 2)
                outFileStr = args[1];

            StreamReader RCFile = null;
            try
            {
                RCFile = new StreamReader(RCFileStr);
            }
            catch
            {
                Console.WriteLine("Error opening resource file {0}.", RCFileStr);
                return 1;
            }

            string major = null, minor = null, build = null, revision = null;
            string line;
            while ((line = RCFile.ReadLine()) != null)
            {
                int pos;
                if ((pos = line.IndexOf("FILEVERSION")) < 0)
                    continue;
                pos += "FILEVERSION".Length + 1;
                string[] tempInfo = line.Substring(pos).Split(" .,".ToCharArray());
                ArrayList verInfo = new ArrayList(4);
                foreach (string s in tempInfo)
                    if (s.Length > 0)
                        verInfo.Add(s);
                if (verInfo.Count != 4)
                    break;
                major = verInfo[0].ToString();
                minor = verInfo[1].ToString();
                revision = verInfo[2].ToString();
                build = verInfo[3].ToString();
            }

            if (major == null)
            {
                Console.WriteLine("Version information not found in resource file {0}.", RCFileStr);
                return 1;
            }

            StreamWriter outFile = null;
            try
            {
                outFile = new StreamWriter(outFileStr, false);
                outFile.AutoFlush = true;
            }
            catch
            {
                Console.WriteLine("Error opening output file {0} for overwriting.", outFileStr);
                return 1;
            }

            outFile.WriteLine("#define MAJORVERSION {0}", major);
            outFile.WriteLine("#define MINORVERSION {0}", minor);
            outFile.WriteLine("#define REVISIONNUMBER {0}", revision);
            outFile.WriteLine("#define BUILDNUMBER {0}", build);

            RCFile.Close();
            outFile.Close();

            return 0;
        }
    }
}
