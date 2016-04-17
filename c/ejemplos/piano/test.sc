./scsynth -t $PORT -D 0 -R 0 -U /opt/sonic-pi/app/server/native/raspberry/extra-ugens:/usr/lib/SuperCollider/plugins -l 1s.boot;
s.sendMsg("/d_load", "/opt/sonic-pi/etc/synthdefs/compiled/sonic-pi-piano.scsyndef");
s.sendMsg("/s_new", "sonic-pi-piano", x = s.nextNodeID, 1, 1, "freq", 900);
s.sendMsg("/n_free", x);