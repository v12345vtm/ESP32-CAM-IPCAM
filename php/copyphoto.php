<?php

$timestamp = date('Y-m-d-H-i-s').".jpeg" ;


$image = 'http://192.168.1.20/capture';
//$image = 'http://192.168.1.3/demoruimte-oud/ico/128/eetkamer.PNG';
//$image = 'http://192.168.1.42/fotozext';


echo copy($image,"cam/".$timestamp);  
echo "is copy gelukt ........ " .$timestamp . "       ";
echo '<hr>';


?>


 

 
