<?php
/**
 * @file
 * commerce_userpoints_buy.features.inc
 */

/**
 * Implementation of hook_commerce_product_default_types().
 */
function commerce_userpoints_buy_commerce_product_default_types() {
  $items = array(
    'points_product' => array(
      'type' => 'points_product',
      'name' => 'Points Product',
      'description' => '',
      'help' => '',
      'module' => 'commerce_product_ui',
    ),
  );
  return $items;
}
