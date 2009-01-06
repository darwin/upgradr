using System;
using System.Diagnostics;
using System.IO;

namespace BeauSkinner.VerCopy
{
    /// <summary>
    /// Summary description for Class1.
    /// </summary>
    class VerCopy
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static int Main(string[] args)
        {
            string PathFile = args[0] + "\\" + args[1];
            string PathFileCopy = args[0] + "\\" + args[2];

            FileInfo fileInfo = new FileInfo(PathFile);
            if (!fileInfo.Exists)
            {
                Console.WriteLine("File does not exist.");
                return 1;
            }

            FileVersionInfo verInfo = FileVersionInfo.GetVersionInfo(PathFile);
            string destPath = null;
            destPath = args[3].Trim("\\".ToCharArray());
            string destDir = verInfo.FileMajorPart + "-" + verInfo.FileMinorPart + "-" + verInfo.FileBuildPart + "-" + verInfo.FilePrivatePart;
            destPath += "\\" + destDir;
            if (!Directory.Exists(destPath))
                Directory.CreateDirectory(destPath);

            string destFile;
            destFile = destPath + "\\" + args[2];
            File.Copy(PathFileCopy, destFile, true);

            return 0;
        }
    }
}
