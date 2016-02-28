#!/usr/bin/php

<?php

$strings = [];
$lineNumbers = [];

function addString($in)
{
	global $strings;

	if ($in != "")
	{
		$strings[] = $in;	
		$strings = array_unique($strings);
	}
}

function extractC($filename)
{
	global $lineNumbers;
	
	$i = 0;

	$lines = file($filename);
	
	$reg = "(_\\(\"([^\\\"]*)\")";
	
	foreach ($lines as $line)
	{
		$i++;
	
		if (preg_match($reg, $line, $matches) > 0)
		{
			addString($matches[1]);
			
			$lineNumbers[] = "$filename:$i";
		}
	}
}

function extractJSON($filename)
{
	global $lineNumbers;

	$data = file_get_contents($filename);
	$json = json_decode($data);
	
	if (strpos($filename, "widget") !== false)
	{
		foreach ($json as $widget)
		{
			addString($widget->{"text"});
			
			$lineNumbers[] = "$filename";
		}
	}
	else if (strpos($filename, "missions") !== false)
	{
		addString($json->{"description"});
		
		if (array_key_exists("objectives", $json))
		{
			foreach ($json->{"objectives"} as $objective)
			{
				addString($json->{"description"});
				
				$lineNumbers[] = "$filename";
			}
		}
		
		if (array_key_exists("script", $json))
		{
			foreach ($json->{"script"} as $scripts)
			{
				foreach ($scripts->{"lines"} as $line)
				{
					if (strpos($line, "MSG_BOX") === 0)
					{
						$i = strpos($line, ";") + 1;
						
						$line = substr($line, $i);
					
						addString($line);
						
						$lineNumbers[] = "$filename";
					}
				}
			}
		}
	}
}

function recurseDir($dir)
{
	$files = array_diff(scandir($dir), array('..', '.'));
	
	foreach ($files as $file)
	{
		if (is_dir("$dir/$file"))
		{
			recurseDir("$dir/$file");
		}
		else if (strstr($file, ".c") !== FALSE)
		{
			extractC("$dir/$file");
		}
		else if (strstr($file, ".json") !== FALSE)
		{
			extractJSON("$dir/$file");
		}
	}
}

recurseDir("../src");

recurseDir("../data/widgets");

recurseDir("../data/missions");

recurseDir("../data/challenges");

$potHeader = file_get_contents("../tools/potHeader.txt");

$handle = fopen("../locale/tbftss.pot", "w");

$dateTime = date("Y-m-d H:i:sO");

$potHeader = str_replace("{POT_CREATION_DATE}", $dateTime, $potHeader);

fwrite($handle, "$potHeader\n");

$i = 0;

foreach ($strings as $string)
{
	fwrite($handle, "#: $lineNumbers[$i]\n");
	fwrite($handle, "msgid \"$string\"\n");
	fwrite($handle, "msgstr \"\"\n");
	fwrite($handle, "\n");
	
	$i++;
}

fclose($handle);

?>
