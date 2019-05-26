import logging
import csv
import argparse
import time
from functools import partial
from concurrent.futures import ThreadPoolExecutor

from db_factory import (
    create_db_connection,
    create_orders
)
from orders import Order


def timed_order_import(file_name, log, batch_size):
    log.info("timing processing of %s ", file_name)
    before = time.time()
    db = create_db_connection()
    orders = create_orders(log, db)
    batch = []
    # the reading assumes csv file has no header
    with open(file_name) as f:
        for l in f:
            seq = l.strip('\n').split(',')
            order = Order(*seq)
            batch.append(order)
            if len(batch) == batch_size:
                orders.bulk_add(batch)
                batch = []
    if len(batch):
        orders.bulk_add(batch)
    db.commit()
    after = time.time()
    return file_name, int(after - before)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
                        "--batch-size",
                        type=int,
                        default=100,
                        help="specify how many inserts in one batch")
    parser.add_argument(
                        "--num-workers",
                        type=int,
                        default=1,
                        help="specify how many parallel processing")
    parser.add_argument(
                        "files_list",
                        type=str,
                        help="a txt file with all csv files full path")
    args = parser.parse_args()

    FORMAT = '%(asctime)s %(message)s'
    logging.basicConfig(format=FORMAT, level=logging.INFO)
    logger = logging.getLogger(__name__)
    logger.info("Starting extracting")

    csvfile = open('./timings.csv', 'w+')
    csvwriter = csv.writer(csvfile)
    csvwriter.writerow(['filename', 'processing time (sec)'])
    with open(args.files_list) as file_list:
        order_files = (f.strip('\n') for f in file_list)
        worker_func = partial(timed_order_import, log=logger, batch_size=args.batch_size)
        if args.num_workers == 1:
            results = map(worker_func, order_files)
        else:
            results = ThreadPoolExecutor(
                max_workers=args.num_workers
            ).map(
                worker_func,
                order_files
            )
        for elapsed, file_name in results:
            csvwriter.writerow([elapsed, file_name])
    csvfile.close()


if __name__ == "__main__":
    main()
