<?php

/**
 * @file
 * Implements the different hook required by Drupal Commerce module to display specific Checkout Panes
 */

/**
 * Payment pane: form callback.
 */
function commerce_userpoints_pane_checkout_form($form, &$form_state, $checkout_pane, $order) {
  $pane_form = array();

  $pane_form['cup_information'] = array(
    '#type' => 'item',
    '#title' => t('Using your credit'),
    '#description' => 'Enter the amount of credit you wish to use.',
    '#prefix' => '<div id="cup-available-credit-div">',
    '#suffix' => '</div>',
  );

  $pane_form['cup_credit_amount'] = array(
    '#type' => 'textfield',
    '#title' => 'Amount',
    '#description' => 'Amount of credit to use.',
    '#size' => 10,
  );

  $pane_form['cup_credit_available'] = array(
    '#type' => 'item',
    '#title' => t('Available credit'),
    '#markup' => check_plain(userpoints_get_current_points(NULL, variable_get('commerce_userpoints_default_termid'))),
    '#prefix' => '<div id="cup-available-credit-div">',
    '#suffix' => '</div>',
  );

  $pane_form['cup_submit_all'] = array(
    '#type' => 'submit',
    '#title' => 'all_credit',
    '#value' => t('Use all my credit'),
    '#limit_validation_errors' => array(),       // No validation.
    '#submit' => array('commerce_userpoints_use_credit_callback_all'),
  );

  $pane_form['cup_submit'] = array(
    '#type' => 'submit',
    //'#title' => 'input_credit',
    '#access' => TRUE,
    '#value' => t('Use credit'),
    '#submit' => array('commerce_userpoints_use_credit_callback_input'),
  );

  $pane_form['cup_remove'] = array(
    '#type' => 'submit',
    '#title' => 'remove',
    '#value' => t('Get back'),
    '#limit_validation_errors' => array(),       // No validation.
    '#submit' => array('commerce_userpoints_remove_credit_callback'),
  );
  return $pane_form;
}

/**
 * Implement hook_form_validate().
 */
function commerce_userpoints_pane_checkout_form_validate($form, &$form_state, $checkout_pane, $order) {
  return TRUE;
}
/**
 * Implement hook_form_submit().
 */
function commerce_userpoints_pane_checkout_form_submit($form, &$form_state, $checkout_pane, $order) {
  return;
}

/**
 * Implements the callback for the checkout pane review form.
 */
function commerce_userpoints_pane_review($form, $form_state, $checkout_pane, $order) {

  // Extract the View and display keys from the cart contents pane setting.
  list($view_id, $display_id) = explode('|', variable_get('commerce_userpoints_cart_summary', 'commerce_userpoints_cart_summary|default'));

  return commerce_embed_view($view_id, $display_id, array($order->order_id));
}

/**
 * Checkout pane callback: returns the cart contents pane's settings form.
 */
function commerce_userpoints_pane_settings_form($checkout_pane) {
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

  $form['commerce_userpoints_review_pane_view'] = array(
    '#type' => 'select',
    '#title' => t('Cart contents View'),
    '#description' => t('Specify the View to use in the review pane to display the coupons.'),
    '#options' => $options,
    '#default_value' => variable_get('commerce_userpoints_cart_summary', 'commerce_userpoints_cart_summary|default'),
  );

  return $form;
}