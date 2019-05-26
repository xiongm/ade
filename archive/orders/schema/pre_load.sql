-- Deploy schema loaded BEFORE orders csv are imported to db

BEGIN;


CREATE TABLE order_status(
  Id               SMALLINT                       PRIMARY KEY
, Name             VARCHAR(32)                    NOT NULL
);


CREATE TABLE payment_status(
  Id             SMALLINT                       PRIMARY KEY
, Name           VARCHAR(64)                    NOT NULL
);

CREATE TABLE shipping_status(
  Id             SMALLINT                       PRIMARY KEY
, Name           VARCHAR(64)                    NOT NULL
);

CREATE TABLE orders(
 OrderId                        BIGINT                         NOT NULL
, CustomerId                    INTEGER                        NOT NULL
, OrderStatusId                 SMALLINT                       NOT NULL
, PaymentStatusId               SMALLINT                       NOT NULL
, ShippingStatusId              SMALLINT                       NOT NULL
, OrderSubtotalInclTax          NUMERIC                        NOT NULL
, OrderSubtotalDiscountInclTax  NUMERIC                        NOT NULL
, OrderSubtotalDiscountExclTax  NUMERIC                        NOT NULL
, OrderTotal                    NUMERIC                        NOT NULL
, RefundedAmount                NUMERIC                        NOT NULL
, OrderDiscount                 NUMERIC                        NOT NULL
, CurrencyRate                  NUMERIC                        NOT NULL
, CurrencyCode                  CHAR(3)                        NOT NULL
, OrderDateTime                 TIMESTAMP                      NOT NULL
);

CREATE VIEW completed_no_shipping_by_date AS
  WITH aggregation AS (
    SELECT
      date(OrderDateTime) as Date,
      sum(OrderTotal) as OrderTotal,
      OrderStatusId,
      ShippingStatusId,
      PaymentStatusId
    FROM orders
    GROUP BY date(OrderDateTime), OrderStatusId, ShippingStatusId, PaymentStatusId HAVING OrderStatusId = 30 and ShippingStatusId = 10 and PaymentStatusId = 30
  ), names AS (
    SELECT
    os.Name as OrderStatusName,
    ss.Name as ShippingStatusName,
    ps.Name as PaymentStatusName
    FROM order_status as os, shipping_status as ss, payment_status as ps
    WHERE os.Id = 30 and ss.Id = 10 and ps.Id = 30
  )
  SELECT
    a.Date,
    a.OrderTotal,
    n.*
  FROM aggregation as a
  CROSS JOIN names as n
  ORDER BY a.Date DESC;

COMMIT;

