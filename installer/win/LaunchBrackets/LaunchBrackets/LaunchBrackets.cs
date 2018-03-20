using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Deployment.WindowsInstaller;
using System.IO;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace LaunchBrackets
{
    public class LaunchBracketsClass
    {

        [CustomAction]
        public static ActionResult LaunchBracketsSuccess(Session session)
        {
            return LaunchBracketsHandler(session, "success");
        }

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

        public static JObject LoadJson(string fileName)
        {
            if (File.Exists(fileName))
            {
                using (StreamReader r = new StreamReader(fileName))
                {
                    string json = r.ReadToEnd();
                    object parsed = JsonConvert.DeserializeObject(json);
                    JObject jsonObject = JObject.Parse(parsed.ToString());
                    return jsonObject;
                }
            }
            return new JObject();
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
            string fileDirectory = appdata + "\\Brackets\\updateTemp";
            string installerStateFile = fileDirectory + "\\updateHelper.json";
            JObject parsedJSON = LoadJson(installerStateFile);
            parsedJSON["state"] = "cancel";

            session.Log("Writing InstallerState to " + installerStateFile);

            //It will create if dir does not exist
            System.IO.Directory.CreateDirectory(fileDirectory);
            using (System.IO.StreamWriter file =
                new System.IO.StreamWriter(installerStateFile))
            {
                session.Log("Writing: " + parsedJSON.ToString());

            }

            using (StreamWriter file = File.CreateText(installerStateFile))
            using (JsonTextWriter writer = new JsonTextWriter(file))
            {
                parsedJSON.WriteTo(writer);
            }

            return ActionResult.Success;
        }
    }
}
