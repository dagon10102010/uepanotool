Add-Content -Path .\example\stereo.txt -Value "width 3000"
Add-Content -Path .\example\stereo.txt -Value "height 1500"
Add-Content -Path .\example\stereo.txt -Value "fixedge 1"

for ($i = 0; $i -lt 80; $i++) {
    $f = '{0:d4}' -f $i
    $a = $i*4.5
    Add-Content -Path .\example\stereo.txt -Value "input example/stereo/stereo_left.$f.png $a 0 0 9"
}

for ($i = 80; $i -lt 160; $i++) {
    $f = '{0:d4}' -f $i
    $a = ($i-80)*4.5
    Add-Content -Path .\example\stereo.txt -Value "input example/stereo/stereo_left.$f.png $a 75 0 9"
}

for ($i = 160; $i -lt 240; $i++) {
    $f = '{0:d4}' -f $i
    $a = ($i-160)*4.5
    Add-Content -Path .\example\stereo.txt -Value "input example/stereo/stereo_left.$f.png $a -75 0 9"
}

Add-Content -Path .\example\stereo.txt -Value "output example/stereo_left.jpg"
Add-Content -Path .\example\stereo.txt -Value "clear"


for ($i = 0; $i -lt 80; $i++) {
    $f = '{0:d4}' -f $i
    $a = $i*4.5
    Add-Content -Path .\example\stereo.txt -Value "input example/stereo/stereo_right.$f.png $a 0 0 9"
}

for ($i = 80; $i -lt 160; $i++) {
    $f = '{0:d4}' -f $i
    $a = ($i-80)*4.5
    Add-Content -Path .\example\stereo.txt -Value "input example/stereo/stereo_right.$f.png $a 75 0 9"
}

for ($i = 160; $i -lt 240; $i++) {
    $f = '{0:d4}' -f $i
    $a = ($i-160)*4.5
    Add-Content -Path .\example\stereo.txt -Value "input example/stereo/stereo_right.$f.png $a -75 0 9"
}

Add-Content -Path .\example\stereo.txt -Value "output example/stereo_right.jpg"