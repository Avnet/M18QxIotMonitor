cp /data/user/mm_conf/malmanager.cfg /data/user/mm_conf/malmanager.cfg_old
cat /data/user/mm_conf/malmanager.cfg | sed 's/\<name\>\ =.*;/name\ =\ "m2m.com.attz";/' > /data/user/mm_conf/malmanager.cfg
