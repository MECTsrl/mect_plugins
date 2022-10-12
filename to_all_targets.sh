#!/bin/bash
#X0_ADDR="192.168.5.25"
X1_ADDR="192.168.5.66"
#X2_ADDR="192.168.5.76"
#X3_ADDR="192.168.5.114"
X4_ADDR="192.168.5.213"
#X5_ADDR="192.168.5.243"
#X6_ADDR="192.168.5.57"
#X7_ADDR="192.168.5.63"
#X8_ADDR="192.168.5.68"
#X9_ADDR="192.168.5.69"

date
for XX_ADDR in  $X0_ADDR $X1_ADDR $X2_ADDR $X3_ADDR $X4_ADDR $X5_ADDR $X6_ADDR $X7_ADDR $X8_ADDR $X9_ADDR; do
	echo "------------------ Stopping Panel: $XX_ADDR ------------------"
	ssh root@$XX_ADDR "stop"                    || { echo "cannot stop:$XX_ADDR"; exit; } 
	ssh root@$XX_ADDR "mount -o remount,rw /"   || { echo "cannot remount in read/write / fs on:$XX_ADDR"; exit; } 
	
	echo "------------------ Deploy su target (da cartella mect_plugins): $XX_ADDR ------------------ "
	rsync -avxc ./qt_library/*/libATCM*.so.1.0.0 ./qt_plugins/libATCMplugin.so.1.0.0 root@$XX_ADDR:/usr/lib/ || { echo "cannot copy to  /usr/lib/ @$XX_ADDR"; exit; } 
	
	echo "------------------ Configuring Target: $XX_ADDR ------------------ "
	ssh root@$XX_ADDR "/sbin/ldconfig"              || { echo "cannot reconfigure libs on:$XX_ADDR"; exit; } 
	ssh root@$XX_ADDR sync                          || { echo "cannot sync the filesystem in:$XX_ADDR"; exit; }  
	ssh root@$XX_ADDR "mount -o remount,ro /"       || { echo "cannot remount in read/only / fs on:$XX_ADDR"; exit; } 
	echo "------------------ Restarting Target: $XX_ADDR ------------------ "
	ssh root@$XX_ADDR "/sbin/reboot"                || { echo "cannot restart:$XX_ADDR"; exit; } 
	echo "------------------ All Done with Target: $XX_ADDR ------------------ "
	echo ""	
	echo ""	
done	

	echo "------------------ ok, done ------------------"
date
