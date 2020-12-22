FROM alpine:latest
RUN apk add --no-cache iperf3
RUN apk add --no-cache iproute2
RUN apk add --no-cache --upgrade bash


ADD ./script.sh /
RUN chmod +x script.sh
#RUN tc qdisc
#RUN ./script.sh eth0 127.0.0.1:8081 127.0.0.1:8082 127.0.0.1:8083 127.0.0.1:8084 
