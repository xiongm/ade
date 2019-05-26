import tweepy
from tweepy import OAuthHandler
from config import (
    CONSUMER_KEY,
    CONSUMER_SECRET,
    ACCESS_TOKEN,
    ACCESS_TOKEN_SECRET,
    SUBSCRIPTIONS
)

from messaging import SentimentPublisher
from utils import sentiment_teller
from sentiment_listener import SentimentListener


class TwitterClient(object):
    def __init__(self):
        try:
            self.auth = OAuthHandler(CONSUMER_KEY, CONSUMER_SECRET)
            self.auth.set_access_token(ACCESS_TOKEN, ACCESS_TOKEN_SECRET)
            self.api = tweepy.API(self.auth)
        except:
            print("Authentication Failure")


    def stream(self, track, listener):
        stream = tweepy.Stream(auth=self.api.auth, listener=listener)
        stream.filter(track=track, languages=['en'])



def main():
    client = TwitterClient()

    sentiment_publisher = SentimentPublisher()

    client.stream(
        SUBSCRIPTIONS,
        SentimentListener(
            client.api,
            sentiment_teller,
            SUBSCRIPTIONS,
            callback=sentiment_publisher.publish_senti
        )
    )

if __name__ == "__main__":
    main()

