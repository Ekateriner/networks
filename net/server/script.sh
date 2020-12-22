#!/bin/bash
DEV=$1
#IN_DEV=$2
FIRST_ZONE=$2
SECOND_ZONE=$3
THIRD_ZONE=$4
FOURTH_ZONE=$5

#second & fourth rule:
tc qdisc add dev $DEV handle ffff: ingress

tc filter add dev $DEV protocol all parent ffff: prio 1 u32 match ip protocol 1 0xff action drop #ignore icmp
tc filter add dev $DEV protocol all parent ffff: prio 1 u32 \
       match ip src $THIRD_ZONE \
       match ip sport 22 0xffff action drop #ignore ssh from third zone


#first rule:
tc qdisc add dev $DEV root handle 1:0 htb default 50
tc class add dev $DEV parent 1:0 classid 1:1 htb rate 100mbit ceil 101mbit

tc class add dev $DEV parent 1:1 classid 1:10 htb rate 40mbit ceil 101mbit # first zone
tc class add dev $DEV parent 1:1 classid 1:20 htb rate 20mbit ceil 101mbit # second zone
tc class add dev $DEV parent 1:1 classid 1:30 htb rate 20mbit ceil 101mbit # third zone
tc class add dev $DEV parent 1:1 classid 1:40 htb rate 20mbit ceil 101mbit # fourth zone
tc class add dev $DEV parent 1:1 classid 1:50 htb rate 1mbit ceil 101mbit # other trafic

tc filter add dev $DEV protocol ip parent 1:1 prio 2 u32 match ip src $FIRST_ZONE flowid 1:10
tc filter add dev $DEV protocol ip parent 1:1 prio 3 u32 match ip src $SECOND_ZONE flowid 1:20
tc filter add dev $DEV protocol ip parent 1:1 prio 4 u32 match ip src $THIRD_ZONE flowid 1:30
tc filter add dev $DEV protocol ip parent 1:1 prio 5 u32 match ip src $FOURTH_ZONE flowid 1:40
#tc filter add dev $DEV protocol ip parent 1:1 prio 5 flowid 1:50

#third rule:
tc qdisc add dev $DEV parent 1:20 handle 20: prio
tc filter add dev $DEV protocol all parent 1:20 prio 1 u32 match ip protocol 17 0xff flowid 20:1 # udp high priority

#handles:
tc qdisc add dev $DEV parent 1:10 handle 10: sfq perturb 10

tc qdisc add dev $DEV parent 20:1 handle 21: sfq perturb 10
tc qdisc add dev $DEV parent 20:2 handle 22: sfq perturb 10
tc qdisc add dev $DEV parent 20:3 handle 23: sfq perturb 10

tc qdisc add dev $DEV parent 1:30 handle 30: sfq perturb 10
tc qdisc add dev $DEV parent 1:40 handle 40: sfq perturb 10
tc qdisc add dev $DEV parent 1:50 handle 50: sfq perturb 10

