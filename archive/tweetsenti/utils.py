from textblob import TextBlob
import re

def sentiment_teller(tweet):
   """
     returns an sentiment level between [-100, 100]
     sentiment level == 100 => positive
     sentiment level == -100 => negative
     sentiment level == 0 => neutral
   """
   cleaned_tweet = ' '.join(re.sub("(@[A-Za-z0-9]+)|([^0-9A-Za-z \t])|(\w+:\/\/\S+)", " ", tweet).split())
   analysis = TextBlob(cleaned_tweet)
   return int(analysis.sentiment.polarity * 100)

def encode(index, senti):
    """
     encode the index of the keyword and its sentiment
     so we only need to transmit a numeric value

     since sentiment value is from -100 to 100, we use
     [0, 100] to represent sentiment [0, 100]
     [101, 200] to represent [-100, -1]

     i.e. encoded senti only has positive value

    """
    senti = senti if senti >= 0 else (201 + senti)
    return index * 201 + senti


def decode(body):
    """
     decode to index of keywords and original value
     of senti
    """

    senti = body % 201
    senti = (senti - 201) if senti > 100 else senti

    return body // 201, senti

