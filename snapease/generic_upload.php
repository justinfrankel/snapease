<?php

$username = "user";
$password = "pass";
$outpath = "/images"; // make sure this path has the correctpermissions set
$use_perm=0664; // default permissions to make new files (directories get 0111 added)


$output_root = dirname(__FILE__); 
if ($output_root == "") die("error on server\n");

if ($_SERVER['PHP_AUTH_USER'] != $username || $_SERVER['PHP_AUTH_PW'] != $pass)
{
   header('WWW-Authenticate: Basic realm="snapease_upload"');
   header('HTTP/1.0 401 Unauthorized');
   die('error bad password\n');
}

if (!$_FILES || !($file = $_FILES["snapease_file"])) die("error no file\n");

$tgt = str_replace("\\","/",$_REQUEST['snapease_target']);
for (;;) // remove any questionable .. from filenames!
{
  $x = str_replace("../","/",$tgt);
  $x = str_replace("/..","/",$x);
  $x = str_replace("//","/",$x);
  if ($x==$tgt) break;
  $tgt=$x;
}

if ($tgt == "") die("error invalid target specified\n");

// optional safety checks
if (strcasecmp(substr($tgt,-4),".jpg") && strcasecmp(substr($tgt,-4),".png")) die("error bad filename!\n");
//if (@file_exists("$output_root/$tgt")) die("error output file already exists!\n");

$a = explode("/",$tgt);
$tmp = $output_root;

for ($x=0;$x<count($a)-1;$x++)
{
  $tmp .= "/" . $a[$x];
  @mkdir($tmp);

  if ($use_perm) @chmod($tmp,$use_perm|0111);
}

if (!@move_uploaded_file($file["tmp_name"], "$output_root/$tgt")) die("error copying file to '$output_root/$tgt'\n");

if ($use_perm) @chmod("$output_root/$tgt",$use_perm);

echo "ok\n";

?>
