[CmdletBinding()]
param (
    [Parameter()]
    [switch]
    $makevideo
)

$maxstitch = 10
$continueface = 0
$renderarg = "-game -PIEVIACONSOLE -Multiprocess  GameUserSettingsINI=`"PIEGameUserSettings0`" -MultiprocessSaveConfig -forcepassthrough -messaging -SessionName=`"Play in Standalone Game`" -windowed -WinX=320 -WinY=194 -ResX=1280 -ResY=720"
function Render {
    param (
        $id
    )
    $arg = "$($data.project) $renderarg -renderpano -renderindex=$id"
    Start-Process -FilePath $data.ued -ArgumentList $arg -PassThru -Wait
}
function GenLoopScript {
    param (
        $prefix,
        $config,
        $min,
        $max
    )

    if($min -ne 0){
        Add-Content .\tool\script.txt "background Saved/MovieRenders_output/CaptureSeq.$prefix%04d.tif"
    }
    $id = 0
    for ($i = 0; $i -lt $config.horizontalStep.Length; $i++) {
        $step = $config.horizontalStep[$i]
        for ($j = 0; $j -lt $step; $j++) {
            if(($id -ge $min) -and ($id -lt $max)){
                Add-Content .\tool\script.txt "input s$id Saved/MovieRenders/CaptureSeq.$prefix$id.%04d.png"
            }
            $id++
        }
    }

    Add-Content .\tool\script.txt "output Saved/MovieRenders_output/CaptureSeq.$prefix%04d.tif"
    if($max -eq $config.total){
        Add-Content .\tool\script.txt "output Saved/MovieRenders_output/CaptureSeq.$prefix%04d.jpg"
    }
    Add-Content .\tool\script.txt "clear"
}
function JoinStereo {
    param (
        $data
        
    )
    $config = $data.data
    $width = $config.outwidth
    $height = $config.outheight
    Set-Content .\tool\script.txt "width $width"
    Add-Content .\tool\script.txt "height $width"
    Add-Content .\tool\script.txt "loop 0 $($data.frames)"
    Add-Content .\tool\script.txt "background Saved/MovieRenders_output/CaptureSeq.left%04d.tif"
    Add-Content .\tool\script.txt "background Saved/MovieRenders_output/CaptureSeq.right%04d.tif $height"
    Add-Content .\tool\script.txt "output Saved/MovieRenders_output/CaptureSeq.center%04d.jpg"
    Add-Content .\tool\script.txt "clear"
    Add-Content .\tool\script.txt "endloop"
    .\tool\uestitch -script .\tool\script.txt
}
function Stitch {
    param (
        $data,
        $min,
        $max
        
    )
    if(Test-Path .\Saved\MovieRenders){
        Remove-Item .\Saved\MovieRenders -Recurse -Force
    }
    
    $config = $data.data
    $fov = $config.fov
    $width = $config.width
    $height = $config.height
    $yaw = 0
    $pitch = 0
    $frames = $data.frames
    $stereo = $config.stereo

    Set-Content .\tool\script.txt "width $($config.outwidth)"
    Add-Content .\tool\script.txt "height $($config.outheight)"
    if($max -ne $config.total){
        Add-Content .\tool\script.txt "alpha 1"
    }
    
    $id = 0

    for ($i = 0; $i -lt $config.horizontalStep.Length; $i++) {
        $step = $config.horizontalStep[$i]
        $pitch = $config.horizontalPitch[$i]
        $a = 360.0/$step
        for ($j = 0; $j -lt $step; $j++) {
            $yaw = $a*$j
            if(($id -ge $min) -and ($id -lt $max)){
                Add-Content .\tool\script.txt "plane s$id $yaw $pitch 0 $fov $width $height"
                Write-Host "Render slice $id/$($config.total)"
                Render $id
                if($stereo){
                    Render $($id+$config.total)
                }
            }
            $id++
        }
    }
    Add-Content .\tool\script.txt "loop 0 $frames"

    
    if($stereo){
        GenLoopScript "left" $config $min $max
        GenLoopScript "right" $config $min $max
    }else{
        GenLoopScript "center" $config $min $max
    }
    
    Add-Content .\tool\script.txt "endloop"
    
    .\tool\uestitch -script .\tool\script.txt
    
}
function MakeAll{
    if(!(Test-Path .\panoconfig.json)){
        return
        Write-Host "file panoconfig.json not found"
    }
    $data = Get-Content .\panoconfig.json | ConvertFrom-Json
    
    # Remove-Item .\Saved\MovieRenders_output -Recurse -Force
    New-Item .\Saved\MovieRenders_output -ItemType Directory -Force
    for ($i = $continueface; $i -lt $data.data.total; $i+=$maxstitch) {
        $j = $i+$maxstitch
        if($j -ge $data.data.total){
            Stitch $data $i $data.data.total
    
        }else{
            Stitch $data $i $j
        }
    }
    if($data.data.stereo){
        JoinStereo $data
    }
    if($makevideo){
        ffmpeg.exe -i .\Saved\MovieRenders_output\CaptureSeq.center%04d.jpg -c:v libx264 -pix_fmt yuv420p video.mp4 -y
    }
    explorer .\Saved\MovieRenders_output
}
MakeAll
