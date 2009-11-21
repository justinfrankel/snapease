#!/usr/bin/php

<?

$src_build_dir = "Release";

function copy_text_replace_line($infn, $outfn, $srctext, $desttext)
{
  $replcnt=0;
  $in=fopen($infn,"r");
  $out = fopen($outfn,"w");
  if (!$in || !$out) die("error opening file(s) for copy_text_replace\n");
  while (($x = fgets($in)))
  { 
    $x=rtrim($x);
    if ($x == $srctext) { $replcnt++; $x=$desttext; }
    fwrite($out,$x . "\n");
  }
  fclose($in);
  fclose($out);
  if ($replcnt != 1) echo "Warning: replaced $replcnt lines in $inffn -> $outfn\n";
}

function regsearch_file($fn, $pattern)
{
  $fp=fopen($fn,"r");
  if (!$fp) { echo "regsearch_file($fn) could not open file.\n";  return false; }

  while (($x = fgets($fp))) if (ereg($pattern,$x,$regs)) return $regs; 

  fclose($fp);
  return false;
}


$verstr1 = "";
$verstr1_full = "";

$regs = regsearch_file("../main_wnd.cpp",'#define VERSTRING "(.*)".*$');
if ($regs) $verstr1 = str_replace(".","",$verstr1_full=$regs[1]);

echo "APP version $verstr1\n";

if ($verstr1 =="") die("invalid version info found\n");

$ver=$verstr1;

$workdir = "build/BUILD";
system("rm -fr $workdir");
system("mkdir $workdir");
system("mkdir $workdir/SnapEase");

system("cp -R 'build/$src_build_dir/SnapEase.app' $workdir/SnapEase/");

copy_text_replace_line("build/$src_build_dir/SnapEase.app/Contents/Info.plist",
		       "$workdir/SnapEase/SnapEase.app/Contents/Info.plist",
			"\t<string>DEVELOPMENT BUILD -- DEVELOPMENT BUILD</string>",
			"\t<string>$verstr1_full</string>");

$fp = fopen("../license.txt","r");
if ($fp) 
{
  $fpo = fopen("./license-cleaned.txt","w");
  if ($fpo) { 
    while(($x=fgets($fp,4096)))
    {
      $x=rtrim($x);
      $o = "";
      for ($a=0;$a<strlen($x);$a++)
        if (ord($x[$a])<128) $o .= $x[$a];
      fwrite($fpo,$o ."\n");
    }
    fclose($fpo);
  }

  fclose($fp);
}

system("perl ./pkg-dmg --format UDBZ --target ./build/snapease$ver.dmg --source $workdir/SnapEase --license ./license-cleaned.txt --copy stage_DS_Store:/.DS_Store --symlink /Applications:/Applications --mkdir .background --copy background.png:.background --volname SNAPEASE_INSTALL --icon ../images/snapease.icns");



?>
