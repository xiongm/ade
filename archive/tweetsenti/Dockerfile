# TO BUILD & RUN:
# docker build . --tag tweetsenti:latest && docker run --rm -it tweetsenti:latest
FROM python:stretch

RUN apt-get update -y && apt-get upgrade -y

RUN apt-get install -y python3-tk
RUN apt-get install -y rabbitmq-server
RUN apt-get install -y python-pika
RUN apt-get install -y python3-matplotlib

WORKDIR /srv/
ADD requirements.txt /srv/requirements.txt
RUN pip3 install -r requirements.txt
ADD * /srv/

CMD ["python3", "pubsub.py"]
