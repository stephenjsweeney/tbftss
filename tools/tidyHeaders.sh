#!/usr/bin/php

<?php

/*
Copyright (C) 2015-2016 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

$UPDATE_FILES = false;

function cleanHeader($headerFile)
{
	global $UPDATE_FILES;

	$func_pattern = "/(([A-Z0-9_]+)\\()/i";
	$struct_pattern = "/([A-Z]+);/i";

	$bodyFile = $headerFile;
	$bodyFile[strlen($bodyFile) - 1] = 'c';
	
	if (file_exists($bodyFile))
	{
		$header = file($headerFile);
		$body = file_get_contents($bodyFile);
	
		$i = 0;
		$hasChanges = false;
		
		foreach ($header as $line)
		{
			if (strstr($line, "extern") !== FALSE && strstr($line, "getTranslatedString") === FALSE)
			{
				preg_match($func_pattern, $line, $matches);

				if (count($matches) == 3)
				{
					$extern = $matches[2];
				
					if (strstr($body, $extern) === FALSE)
					{
						if (!$hasChanges)
						{
							echo "$headerFile\n";
							$hasChanges = true;
						}
						echo "\t- $line";
						unset($header[$i]);
					}
				}
				
				preg_match($struct_pattern, $line, $matches);

				if (count($matches) == 2)
				{
					$extern = $matches[1];
					
					if (strstr($body, "$extern") === FALSE)
					{
						if (!$hasChanges)
						{
							echo "$headerFile\n";
							$hasChanges = true;
						}
						echo "\t- $line";
						unset($header[$i]);
					}
				}
			}
			
			$i++;
		}
		
		if ($UPDATE_FILES && $hasChanges)
		{
			file_put_contents($headerFile, $header);
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
		else if (strstr($file, ".h") !== FALSE)
		{
			cleanHeader("$dir/$file");
		}
	}
}

if (isset($argv[1]))
{
	$UPDATE_FILES = ($argv[1] == "-commit");
}

recurseDir("../src");

if (!$UPDATE_FILES)
{
	echo "\nNo files updated. Use -commit to update headers\n";
}

?>
