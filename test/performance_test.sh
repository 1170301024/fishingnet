#! /bin/sh

filter="ip"
pkt_num=10

eval tcpdump -c ${pkt_num} ${filter}