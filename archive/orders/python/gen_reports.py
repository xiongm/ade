import logging
import csv
from db_factory import (
    create_db_connection,
    create_completed_no_shipping_by_date
)
from orders import Order

def main():
    FORMAT = '%(asctime)s %(message)s'
    logging.basicConfig(format=FORMAT, level=logging.INFO)
    logger = logging.getLogger(__name__)

    db = create_db_connection()

    report = create_completed_no_shipping_by_date(logger, db)
    rows = report.get_all()
    if rows:
        with open("../results.csv", "w+") as csvfile:
            csvwriter = csv.writer(csvfile)
            csvwriter.writerow( ['Date', 'OrderTotal', 'OrderStatusName', 'ShippingStatusName', 'PaymentStatusName'])
            for row in rows:
                csvwriter.writerow(row)
    logger.info("Done")


if __name__ == "__main__":
    main()
