using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Deployment.WindowsInstaller;
using System.IO;

namespace LaunchBrackets
{
    public class LaunchBracketsClass
    {

        [CustomAction]
        public static ActionResult LaunchBracketsError(Session session)
        {
            return LaunchBracketsHandler(session, "error");
        }

        [CustomAction]
        public static ActionResult LaunchBracketsSuspend(Session session)
        {
            return LaunchBracketsHandler(session, "suspend");
        }

        [CustomAction]
        public static ActionResult LaunchBracketsCancel(Session session)
        {
            return LaunchBracketsHandler(session, "cancel");
        }

        [CustomAction]
        public static ActionResult LaunchBrackets(Session session)
        {
            return LaunchBracketsHandler(session, "success");
        }

        public static ActionResult LaunchBracketsHandler(Session session, string status)
        {
            session.Log("Begin LaunchBrackets");
            string bracketsInstallLocation = session["INSTALLDIRREGISTRY"];
            if (!string.IsNullOrEmpty(bracketsInstallLocation))
            {
                session.Log("Received Brackets Install Location: " + bracketsInstallLocation);
                if (File.Exists(bracketsInstallLocation))
                {
                    session.Log("Running brackets");
                    System.Diagnostics.Process.Start(bracketsInstallLocation);
                }
            }
            session.Log("Begin LaunchBrackets");

            string appdata = Environment.GetEnvironmentVariable("APPDATA");
            string installerStateFile = appdata + "\\Brackets\\InstallerState.json";

            session.Log("Writing InstallerState to " + installerStateFile);
            using (System.IO.StreamWriter file =
                new System.IO.StreamWriter(installerStateFile))
            {
                file.WriteLine("{");
                file.WriteLine("'state':" + status);
                file.WriteLine("}");
            }

            return ActionResult.Success;
        }
    }
}
