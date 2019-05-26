import pika
from utils import encode, decode

class SimpleMessagingBase(object):
    def __init__(self):
        self.connection = pika.BlockingConnection(
            pika.ConnectionParameters(host='localhost')
        )
        self.exchange = ''
        self.queue_name = 'bitgo'
        self.routing_key = self.queue_name
        self.channel = self.connection.channel()
        self.channel.queue_declare(queue=self.queue_name)

class SimplePublisher(SimpleMessagingBase):
    def publish(self, body):
        self.channel.basic_publish(
            exchange=self.exchange,
            routing_key=self.routing_key,
            body=body
        )

class SentimentPublisher(SimplePublisher):
    def publish_senti(self, index, word, senti):
        body = encode(index, senti)
        self.publish(str(body))

class SimpleSubscriber(SimpleMessagingBase):
    def __init__(self, callback):
        super().__init__()
        self.callback = callback
        self.channel.basic_consume(
            queue=self.queue_name,
            on_message_callback=self._callback,
            auto_ack=True
        )

    def start_consuming(self):
        self.channel.start_consuming()

    def _callback(self, ch, method, properties, body):
        self.callback(body)


class SentimentSubscriber(SimpleSubscriber):
    def __init__(self, sentiment_callback):
        super().__init__(self._sentiment_callback)
        self.sentiment_callback = sentiment_callback

    def _sentiment_callback(self, body):
        body = int(body)
        index, senti = decode(body)
        self.sentiment_callback(index, senti)

