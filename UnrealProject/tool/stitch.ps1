[CmdletBinding()]
param (
    [Parameter()]
    [switch]
    $makevideo,
    [Parameter()]
    [int]
    $start=0,
    [Parameter()]
    [int]
    $end=0

)

$maxstitch = 10

# for editor
$renderarg = "-game -PIEVIACONSOLE -Multiprocess  GameUserSettingsINI=`"PIEGameUserSettings0`" -MultiprocessSaveConfig -forcepassthrough -messaging -SessionName=`"Play in Standalone Game`" -windowed -WinX=320 -WinY=194 -ResX=1280 -ResY=720"
# for shiping build
# $renderarg = "-windowed -WinX=320 -WinY=194 -ResX=1280 -ResY=720 -RenderOffscreen"
# $renderarg = "-windowed -WinX=320 -WinY=194 -ResX=1280 -ResY=720"
function Render {
    param (
        $id
    )
    $arg = "$($data.project) $renderarg -renderpano -renderindex=$id"
    Start-Process -FilePath $data.ued -ArgumentList $arg -Wait

}
function GenLoopScript {
    param (
        $prefix,
        $config,
        $ls,
        $vr180
    )

    if($ls[0] -ne 0){
        Add-Content .\tool\script.txt "background Saved/MovieRenders_output/CaptureSeq.$prefix%04d.tif"
    }
    foreach($id in $ls){
        Add-Content .\tool\script.txt "input s$id Saved/MovieRenders/CaptureSeq.$prefix$id.%04d.png"
    }
    Add-Content .\tool\script.txt "output Saved/MovieRenders_output/CaptureSeq.$prefix%04d.tif"
    # if($max -eq $config.total){
    Add-Content .\tool\script.txt "output Saved/MovieRenders_output/CaptureSeq.$prefix%04d.jpg"
    # }
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
    Add-Content .\tool\script.txt "height $($height*2)"
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
        $currid
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
    $projector = $data.projector
    $vr180 = $false
    if(($projector -eq 4) -or ($projector -eq 3)){
        $vr180 = $true
        $config.outwidth = $config.outheight
    }

    Set-Content .\tool\script.txt "width $($config.outwidth)"
    Add-Content .\tool\script.txt "height $($config.outheight)"
    Add-Content .\tool\script.txt "projector $projector"
    
    if($currid+$maxstitch -lt $config.total){
        Add-Content .\tool\script.txt "alpha 1"
    }
    
    $id = 0
    $tmp = 0
    $ran = $currid
    $ls = @()
    for ($i = 0; $i -lt $config.horizontalStep.Length; $i++) {
        $step = $config.horizontalStep[$i]
        $pitch = $config.horizontalPitch[$i]
        $a = 360.0/$step
        for ($j = 0; $j -lt $step; $j++) {
            $yaw = $a*$j
            $skip = $vr180 -and $yaw -gt 90 -and $yaw -lt 270
            if($tmp -lt $maxstitch -and (-not $skip) -and $id -ge $currid){
                $tmp++
                Add-Content .\tool\script.txt "plane s$id $yaw $pitch 0 $fov $width $height"
                Write-Host "Render slice $($id+1)/$($config.total) yaw $yaw pitch $pitch roll 0"
                Render $id
                $ls += $id
                if($stereo){
                    Render $($id+$config.total)
                }
            }
            $id++
        }
    }
    Add-Content .\tool\script.txt "loop 0 $frames"
    if($stereo){
        GenLoopScript "left" $config $ls $vr180
        GenLoopScript "right" $config $ls $vr180
    }else{
        GenLoopScript "center" $config $ls $vr180
    }
    Add-Content .\tool\script.txt "endloop"
    .\tool\uestitch -script .\tool\script.txt > .\tool\tool.log
    if($ls.Count){
        $ran = $ls[$ls.Count-1]
    }
    return $ran + 1
    
}
function MakeAll{
    if(!(Test-Path .\panoconfig.json)){
        return
        Write-Host "file panoconfig.json not found"
    }
    $data = Get-Content .\panoconfig.json | ConvertFrom-Json
    if($end -eq 0){
        $end = $data.data.total
    }
    
    # Remove-Item .\Saved\MovieRenders_output -Recurse -Force
    New-Item .\Saved\MovieRenders_output -ItemType Directory -Force
    $currid = $start
    while ($currid -lt $data.data.total) {
        $currid = Stitch $data $currid
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
