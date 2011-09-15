<?php

/**
 * This function was meant to be called by fapi #ajax call to enhance user experience with commerce userpoints
 * This function is currently not used and probably not functionnal as it haven't been changed while
 * other function have been updated.
 * @param  $form
 * @param  $form_state
 * @return
 */
function commerce_userpoints_use_credit_ajax_callback($form, $form_state) {

  if($form_state['triggering_element']['#title'] == 'all_credit')
    $use_all_credit = TRUE;

  $order = $form_state['order'];
  $cur_user_points = userpoints_get_current_points(NULL, variable_get('commerce_userpoints_default_termid'));

  //Retrieve the order total for selected components
  $order_total = 0;
  $commerce_order_total_field = field_get_items('commerce_order', $order, 'commerce_order_total');
  foreach($commerce_order_total_field[0]['data']['components'] as $price_component) {
    if(array_key_exists($price_component['name'], variable_get('commerce_userpoints_total_price_components'))) {
      $order_total += floatval($price_component['price']['amount']) / 100; //TODO:make the 100 variable, based on the decimal settings
    }
    dpm($price_component, "price component");
  }

  if($use_all_credit) {
    $points_to_use = $cur_user_points >= $order_total ? $order_total : $cur_user_points;
  }
  else {
    $points_to_use = floatval($form['commerce_userpoints']['cup_credit_amount']['#value']);

    if(intval($form['commerce_userpoints']['cup_credit_amount']['#value']) > $cur_user_points) {
      $form['commerce_userpoints']['cup_information']['#description'] = t("You don't have enough credit");
      return $form['commerce_userpoints']['cup_information'];
    }

    if(intval($form['commerce_userpoints']['cup_credit_amount']['#value']) < 0) {
      $form['commerce_userpoints']['cup_information']['#description'] = t("The amount should be a positive value");
      return $form['commerce_userpoints']['cup_information'];
    }

    if(intval($form['commerce_userpoints']['cup_credit_amount']['#value']) > $order_total) {
      $form['commerce_userpoints']['cup_information']['#description'] = t("Too much !");
      return $form['commerce_userpoints']['cup_information'];
    }
  }

  $line_item_id = commerce_userpoints_order_has_userpoint_line_item($order->order_id);

  //If there is no existing userpoints line item, we create a new one
  if($line_item_id == NULL) {
    //Create the userpoints transaction
    $userpoints_params = array(
      'points' => intval($points_to_use) * -1,
      'moderate' => FALSE,
      'uid' => $order->uid,
      'time_stamp' => REQUEST_TIME,
      'operation' => t('Using credit for purchase'),
      'tid' => variable_get('commerce_userpoints_default_termid'),
      'expirydate' => NULL,
      'description' => t('User credit'),
      'display' => FALSE,
      'entity_id' => $order->order_id,
      'entity_type' => 'commerce_order',
    );

    $result = userpoints_userpointsapi($userpoints_params);

    if($result['status'] == FALSE) {
      $form['commerce_userpoints']['cup_information']['#description'] = t("Sorry, you cannot use your credit right now. Please contact us for more information");
      return $form['commerce_userpoints']['cup_information'];
    }

    //Create the line item
    $line_item = commerce_userpoints_line_item_factory_new($order->order_id);

    $label = "User credit";
    commerce_userpoints_line_item_populate(
      $line_item, $order->order_id,
      $result['transaction'], commerce_default_currency(), $label, TRUE);

    //TODO: Should test if the line item has been correctly created
    commerce_userpoints_line_item_save($line_item);
  }
  else { //If there is an existing userpoints line item for this order,
    //we load it to get its userpoints transaction id
    $line_item = commerce_line_item_load($line_item_id);

    //Get the existing transaction
    $txn_field = field_get_items('commerce_line_item', $line_item, 'commerce_userpoints_txn');

    //Clear out the old $line_item
    commerce_userpoints_clear_order($order->order_id);
    unset($line_item);

    //Update the userpoints transaction
    $userpoints_params = array(
      'points' => intval($points_to_use) * -1,
      'txn_id' => $txn_field[0]['value'],
    );
    $result = userpoints_userpointsapi($userpoints_params);

    if($result['status'] == FALSE) {
      $form['commerce_userpoints']['cup_information']['#description'] = t("Sorry, you cannot use your credit right now. Please contact us for more information");
      return $form['commerce_userpoints']['cup_information'];
    }

    //Create the new line item
    $line_item = commerce_userpoints_line_item_factory_new($order->order_id);

    $label = "User credit";
    commerce_userpoints_line_item_populate(
      $line_item, $order->order_id,
      $result['transaction'], commerce_default_currency(), $label, TRUE);

    //TODO: Should test if the line item has been correctly created
    commerce_userpoints_line_item_save($line_item);
  }
  //Refresh the view
  //$view = views_get_view('commerce_cart_summary');
  //$form['cart_contents']['cart_contents_view']['#markup'] = $view->execute_display('default', array($order->order_id));


  //Return the updated remaining points
  //$form['commerce_userpoints']['cup_credit_available']['#markup'] = check_plain(userpoints_get_current_points(NULL, variable_get('commerce_userpoints_default_termid')));
  //return array($form['commerce_userpoints']['cup_credit_available'], $form['cart_contents']['cart_contents_view']);

  //Reload the page
  dpm($form, "form");
  dpm($form_state, "form state");
}

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
    /*'#ajax' => array(
      'callback' => 'commerce_userpoints_use_credit_callback',
      'wrapper' => 'cup-available-credit-div',
      'method' => 'replace',
      'effect' => 'fade',
    )*/
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


function commerce_userpoints_pane_checkout_form_validate($form, &$form_state, $checkout_pane, $order) {
  //TODO: validation form
  return TRUE;
}

function commerce_userpoints_pane_checkout_form_submit($form, &$form_state, $checkout_pane, $order) {
  //TODO
  return;
}

/**
 * Implements the callback for the checkout pane review form
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
        $options[check_plain($view_id)][$view_id .'|'. $display_id] = check_plain($display_value->display_title);
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