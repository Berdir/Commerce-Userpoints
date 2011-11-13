<?php
/**
 * @file
 * Implements the different hook required by Drupal Commerce module to display specific Checkout Panes
 */


/**
 * Form implementation set in commerce_userpoints_discount_commerce_checkout_pane_info().
 */
function commerce_userpoints_discount_pane_checkout_form($form, &$form_state, $checkout_pane, $order) {
  $currencies = commerce_userpoints_currencies();

  if (empty($currencies)) {
    drupal_set_message(t('No !points currencies have been set up.', userpoints_translation()), 'error');
    return array();
  }

  $total = field_get_items('commerce_order', $order, 'commerce_order_total');
  $total = $total[0];

  $default_currency = reset($currencies);
  if (count($currencies) > 1) {
    $options = array();
    foreach ($currencies as $currency) {
      $options[$currency['code']] = $currency['name'];
    }

    $pane_form['currency'] = array(
      '#type' => 'select',
      '#title' => t('!Points currency', userpoints_translation()),
      '#options' => $options,
      '#default_value' => $default_currency['code'],
      '#ajax' => array(
        'callback' => 'commerce_userpoints_discount_ajax_update_info',
        'wrapper' => 'commerce-userpoints-widget-info',
      ),
    );
  }
  else {
    $pane_form['currency'] = array(
      '#type' => 'value',
      '#value' => $default_currency['code'],
    );
  }

  if (isset($form_state['values']['commerce_userpoints_discount']['currency'])) {
    $current_code = $form_state['values']['commerce_userpoints_discount']['currency'];
    $current_tid = $currencies[$form_state['values']['commerce_userpoints_discount']['currency']]['tid'];
  }
  else {
    $current_code = $default_currency['code'];
    $current_tid = $default_currency['tid'];
  }

  $worth_of_a_point = commerce_currency_convert(1, $current_code, $total['currency_code']);
  $worth_of_order = commerce_currency_convert($total['amount'], $total['currency_code'], $current_code);

  $arguments = array(
    '%points' => $worth_of_a_point,
    '@order_currency' => commerce_currency_format($worth_of_a_point, $total['currency_code']),
    '@count_needed' => commerce_currency_format($worth_of_order, $current_code),
  ) + userpoints_translation();


  $pane_form['info'] = array(
    '#type' => 'container',
    '#id' => 'commerce-userpoints-widget-info',
    'exchange' => array(
      '#type' => 'item',
      '#prefix' => '<div class="commerce-userpoints-exchange-rate">',
      '#title' => t('Exchange rate'),
      '#markup' => t('A !point is worth @order_currency. @count_needed required to pay this order in full.', $arguments),
      '#suffix' => '</div>',
    ),
    'available' => array(
      '#type' => 'item',
      '#prefix' => '<div class="commerce-userpoints-available">',
      '#title' => t('Available'),
      '#markup' => t('You have @count !points available for use.', array('@count' => userpoints_get_current_points(NULL, $current_tid)) + userpoints_translation()),
      '#suffix' => '</div>',
    ),
  );

  $pane_form['amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount'),
    '#description' => t('The amount of !points you want to use.', userpoints_translation()),
    '#size' => 10,
  );

  $pane_form['cup_submit_all'] = array(
    '#type' => 'submit',
    '#title' => t('All credit'),
    '#value' => t('Use all my credit'),
    '#limit_validation_errors' => array(array('commerce_userpoints_discount')),
    '#submit' => array('commerce_userpoints_discount_use_credit_callback_all'),
  );

  $pane_form['cup_submit'] = array(
    '#type' => 'submit',
    //'#title' => 'input_credit',
    '#access' => TRUE,
    '#value' => t('Use credit'),
    '#limit_validation_errors' => array(array('commerce_userpoints_discount')),
    '#submit' => array('commerce_userpoints_discount_use_credit_callback_input'),
  );

  $pane_form['cup_remove'] = array(
    '#type' => 'submit',
    '#title' => t('Remove'),
    '#value' => t('Get back'),
    '#limit_validation_errors' => array(array('commerce_userpoints_discount')),
    '#submit' => array('commerce_userpoints_discount_remove_credit_callback'),
  );
  return $pane_form;
}

/**
 * Implements the callback for the checkout pane review form.
 */
function commerce_userpoints_discount_pane_review($form, $form_state, $checkout_pane, $order) {
  // Extract the View and display keys from the cart contents pane setting.
  list($view_id, $display_id) = explode('|', variable_get('commerce_userpoints_discount_cart_summary', 'commerce_userpoints_discount_cart_summary|default'));

  return commerce_embed_view($view_id, $display_id, array($order->order_id));
}

/**
 * Checkout pane callback: returns the cart contents pane's settings form.
 */
function commerce_userpoints_discount_pane_settings_form($checkout_pane) {
  $form = array();

  // Build an options array of Views available for the cart contents pane.
  $options = array();

  // Generate an option list from all user defined and module defined views.
  foreach (views_get_all_views() as $view_id => $view_value) {
  // Only include line item Views.
    if ($view_value->base_table == 'commerce_order') {
      foreach ($view_value->display as $display_id => $display_value) {
        $options[check_plain($view_id)][$view_id . '|' . $display_id] = check_plain($display_value->display_title);
      }
    }
  }

  $form['commerce_userpoints_discount_review_pane_view'] = array(
    '#type' => 'select',
    '#title' => t('Cart contents View'),
    '#description' => t('Specify the View to use in the review pane to display the coupons.'),
    '#options' => $options,
    '#default_value' => variable_get('commerce_userpoints_discount_cart_summary', 'commerce_userpoints_discount_cart_summary|default'),
  );

  return $form;
}
