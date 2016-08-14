#!/bin/bash

cat <<'__EOF__'>/etc/apt/sources.list.d/pdns.list
deb [arch=amd64] http://repo.powerdns.com/ubuntu trusty-auth-40 main
__EOF__

cat <<'__EOF__'>/etc/apt/preferences.d/pdns
Package: pdns-*
Pin: origin repo.powerdns.com
Pin-Priority: 600
__EOF__

wget -O - https://repo.powerdns.com/FD380FBB-pub.asc | apt-key add - &&
apt-get install pdns-server
