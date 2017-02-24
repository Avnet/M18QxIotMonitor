mv -f /data/test/output.txt /data/test/output.last.txt 
/data/test/iot_monitor -f | grep "^[-*<>]" >/data/test/output.txt
