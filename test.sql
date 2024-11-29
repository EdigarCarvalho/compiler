SELECT customer_id AS id, COUNT(order_id) AS total_orders, SUM(quantity) AS total_quantity, MAX(quantity) AS max_order_quantity FROM  orders JOIN 
    order_items ON orders.order_id = order_items.order_id
WHERE 
    order_date BETWEEN '2023-01-01' AND '2023-12-31'
    AND status = 'Shipped'
GROUP BY 
    customer_id
HAVING 
    SUM(quantity) > 100
ORDER BY 
    total_orders DESC;