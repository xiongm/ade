import tweepy

class SentimentListener(tweepy.StreamListener):

    def __init__(self, api, sentiment_teller, keywords, callback=None):
        self.api = api
        super().__init__()
        self.sentiment_teller = sentiment_teller
        self.keywords = keywords
        self.callback = callback

    def on_status(self, status):
        if status.retweeted:
            return
        senti = self.sentiment_teller(status.text)
        for i, w in enumerate(self.keywords):
            if (w in status.text):
               self.callback(i, w, senti)

    def on_error(self, error_code):
        if error_code == 420:
            print("rate limit reached. exiting")
            return False
