from config import (
    SUBSCRIPTIONS
)

from messaging import SentimentSubscriber
from plot import SentimentGraph


def main():
    graph = SentimentGraph()
    sub = SentimentSubscriber(graph.update_data)
    sub.start_consuming()

if __name__ == "__main__":
    main()

