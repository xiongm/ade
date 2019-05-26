from threading import Thread
import time
import pub
import sub

if __name__ == '__main__':
    time.sleep(5)
    Thread(target = pub.main).start()
    time.sleep(5)
    Thread(target = sub.main).start()
