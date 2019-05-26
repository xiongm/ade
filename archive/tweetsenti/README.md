PREREQUISITE
====================================
sudo apt-get install -y python3-tk, rabbitmq-server
pip3 install tweepy
pip3 install textblob


RUNNING WITH DOCKER ON MAC
====================================
1. make sure conifg.py has correct twitter api keys/secrets
2. Enable docker to run GUI interface by following instructions [here](https://cntnr.io/running-guis-with-docker-on-mac-os-x-a14df6a76efc)
   * brew install socat 
   * socat TCP-LISTEN:6000,reuseaddr,fork UNIX-CLIENT:\"$DISPLAY\"
   * brew install xquartz
   * open -a xquartz
   * check security->“allow connections from network clients”
3. get ip of the host mac os, usually ifconfig en0, let's say 192.168.1.14
4. docker run -ti --rm -e DISPLAY=192.168.1.14:0 -v /tmp/.X11-unix:/tmp/.X11-unix tweetsenti


RUN WITH DOCKER
====================================
docker build --no-cache . --tag tweetsenti
docker-compose up

RUN
====================================
1. update config.py file with appropriate info

2. run sub from one window

export PYTHONPATH=.
python3 ./sub.py


3. run pub from another

export PYTHONPATH=.
python3 ./pub.py


NOTES
====================================
a pub/sub model

pub.py is the scraping script collecting the stats and publishing to rabbit mq

sub.py is the the subscribing script consuming the stats and output to graph








