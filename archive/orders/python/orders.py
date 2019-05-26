#!/usr/bin/env python3

# pylint: disable-msg=too-many-arguments, invalid-name

from collections import namedtuple
from typing import List


Order = namedtuple(
    "Order",
    """
    order_id,
    customer_id,
    order_status_id,
    payment_status_id,
    shipping_status_id,
    order_subtotal_incl_tax,
    order_subtotal_discount_inc_tax,
    order_subtotal_discount_excl_tax,
    order_total,
    refunded_amount,
    order_discount,
    currency_rate,
    currency_code,
    order_date_time
    """
)

class Orders(object):

    def __init__(self, log, db):
        self.log = log
        self.db = db

    def bulk_add(self, orders: List[Order]):
        def build_values(o):
            return '(' + ','.join(o[:-2]) + ",'{}', '{}')".format(o[-2], o[-1])

        insert_values = ','.join([build_values(o) for o in orders])

        insert_stmt = """INSERT INTO orders
                         (OrderId, CustomerId, OrderStatusId, PaymentStatusId, ShippingStatusId, OrderSubTotalInclTax, OrderSubtotalDiscountInclTax, OrderSubtotalDiscountExclTax, OrderTotal, RefundedAmount, OrderDiscount, CurrencyRate, CurrencyCode, OrderDateTime)
                         VALUES
                         {}
                         RETURNING
                           OrderId
                         """.format(insert_values)

        with self.db.cursor() as cur:
            cur.execute(insert_stmt)
            row = cur.fetchone()
            return row[0] if row else None
