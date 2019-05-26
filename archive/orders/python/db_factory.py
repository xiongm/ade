import os
import psycopg2
from orders import Orders
from completed_no_shipping_by_date import CompletedNoShippingByDate


def _find_database_ini():
    return os.getenv('DATABASE_INI_PATH', 'database.ini')


def connect(db_ini):
    conn_parameters = dict()
    with open(db_ini) as cfg:
        for l in cfg:
            key, value = l.split('=')
            conn_parameters[key] = value.strip('\n')
    conn = psycopg2.connect(**conn_parameters)
    return conn



def create_db_connection():
    db_ini = _find_database_ini()
    db = connect(db_ini)
    return db


def create_orders(log, db):
    return Orders(log, db)

def create_completed_no_shipping_by_date(log, db):
    return CompletedNoShippingByDate(log, db)
