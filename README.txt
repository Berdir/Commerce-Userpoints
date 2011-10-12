----------------
Summary
----------------
This module makes a bridge between Drupal Commerce module and Userpoints.

It allows users to create commerce line items with negative points value, behaving like a discount system.
The user can choose how many points he wants to use, they will be automatically taken from his userpoints account.

It is not directly used as a payment method for the following reason:
- Depending on how your site will manage points, user will or will not always have the exact amount nor enough points
to fully pay an order
- I don't want to overload the payment process by displaying an odd interface asking the user how he will pay
(userpoints and CB ? CB only ? Userpoints only etc...)
