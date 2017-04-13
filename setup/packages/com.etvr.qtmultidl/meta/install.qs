function Component()
{
	
}
Component.prototype.createOperations = function()
{

    component.createOperations();
       
      if (installer.value("os") === "win") {
        
        component.addOperation("CreateShortcut", "@TargetDir@/MultiDownload.exe", "@StartMenuDir@/MultiDownload.lnk",
            "workingDirectory=@TargetDir@", "@TargetDir@/MultiDownload.exe",
            "iconId=0");
        var windir = installer.environmentVariable("WINDIR");
  		if (windir == "")
    		print("Could not find windows installation directory.");
        
        component.addElevatedOperation("Copy", "@TargetDir@/ssleay32.dll", windir+"/SYSTEM32/");
        component.addElevatedOperation("Copy", "@TargetDir@/libeay32.dll", windir+"/SYSTEM32/");

    }
}
