function Component(){}

Component.prototype.createOperations = function()
{
    try {
        
        component.createOperations();
        if (installer.value("os") === "win") {
            
            component.addElevatedOperation("Execute", "{0,3010,1638,5100}", "@TargetDir@/vcredist_x86.exe", "/quiet", "/norestart");
            component.addElevatedOperation("Delete", "@TargetDir@/vcredist_x86.exe");
        }
    } catch (e) {
        
    }
}