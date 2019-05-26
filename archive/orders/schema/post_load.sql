-- Deploy orders schema AFTER orders csv are loaded into db

BEGIN;

ALTER TABLE orders ADD PRIMARY KEY(OrderId);

ALTER TABLE orders ADD CONSTRAINT check_order_status_id CHECK (OrderStatusId IN (10, 20, 30, 40));
ALTER TABLE orders ADD CONSTRAINT check_payment_status_id CHECK (PaymentStatusId IN (10, 20, 30, 35, 40));
ALTER TABLE orders ADD CONSTRAINT check_shipping_status_id CHECK (ShippingStatusId IN (10, 20, 30, 40));
ALTER TABLE orders ADD CONSTRAINT check_currency_code CHECK (CurrencyCode IN ('USD'));

CREATE INDEX orders_time_stamp_date_idx ON orders(date(OrderDateTime));

COMMIT;

