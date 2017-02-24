mv -f /data/test/output.txt /data/test/output.last.txt 
ping 8.8.8.8 & 
/data/test/iot_monitor -f | grep "^[-*<>]" >/data/test/output.txt
