using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Build.Utilities;
using Microsoft.Build.Framework;
using System.Diagnostics;
using System.IO;

namespace VersionTask
{
    public class VersionTask : Task
    {
        public override bool Execute()
        {
            FileVersionInfo verInfo = FileVersionInfo.GetVersionInfo(FilePath);
            Version = verInfo.FileMajorPart + "." + verInfo.FileMinorPart + "." + verInfo.FileBuildPart;
            return true;
        }

        private string filePath;
        private string version;

        [Required]
        public string FilePath
        {
            get { return filePath; }
            set { filePath = value; }
        }
        [Output]
        public string Version
        {
            get { return version; }
            set { version = value; }
        }
    }
}
