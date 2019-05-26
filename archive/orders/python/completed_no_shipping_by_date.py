#!/usr/bin/env python3

# pylint: disable-msg=too-many-arguments, invalid-name

from collections import namedtuple

class CompletedNoShippingByDate(object):

    def __init__(self, log, db):
        self.log = log
        self.db = db

    def get_all(self):
        select_stmt = """SELECT * FROM completed_no_shipping_by_date"""

        with self.db.cursor() as cur:
            cur.execute(select_stmt)
            try:
                rows = cur.fetchall()
            except:
                rows = None
            return rows;
