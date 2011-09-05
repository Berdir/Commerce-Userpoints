<?php
/**
 * @file
 * commerce_userpoints_buy.rules_defaults.inc
 */

/**
 * Implementation of hook_default_rules_configuration().
 */
function commerce_userpoints_buy_default_rules_configuration() {
  $items = array();
  $items['rules_add_points_from_product'] = entity_import('rules_config', '{ "rules_add_points_from_product" : {
      "LABEL" : "Add points from product",
      "PLUGIN" : "rule",
      "REQUIRES" : [ "rules", "userpoints_rules" ],
      "USES VARIABLES" : {
        "line_item" : { "label" : "Line Item", "type" : "commerce_line_item" },
        "order" : { "label" : "Oder", "type" : "commerce_order" }
      },
      "IF" : [
        { "entity_has_field" : { "entity" : [ "line-item" ], "field" : "commerce_product" } },
        { "entity_has_field" : {
            "entity" : [ "line-item:commerce-product" ],
            "field" : "field_userpoints"
          }
        }
      ],
      "DO" : [
        { "userpoints_action_grant_points" : {
            "user" : [ "order:owner" ],
            "points" : [ "line-item:commerce-product:field-userpoints" ],
            "tid" : "0",
            "entity" : [ "order" ],
            "description" : "Druplicons bought",
            "operation" : "commerce_userpoints_buy",
            "display" : 1,
            "moderate" : "default"
          }
        }
      ]
    }
  }');
  $items['rules_grant_userpoints'] = entity_import('rules_config', '{ "rules_grant_userpoints" : {
      "LABEL" : "Grant userpoints",
      "PLUGIN" : "reaction rule",
      "REQUIRES" : [ "commerce_cart", "rules", "commerce_payment" ],
      "ON" : [ "commerce_payment_order_paid_in_full" ],
      "IF" : [
        { "commerce_order_is_cart" : { "commerce_order" : [ "commerce_order" ] } },
        { "entity_has_field" : {
            "entity" : [ "commerce-order:commerce-line-items:0" ],
            "field" : "commerce_product"
          }
        }
      ],
      "DO" : [
        { "LOOP" : {
            "USING" : { "list" : [ "commerce-order:commerce-line-items" ] },
            "ITEM" : { "list_item" : "Current list item" },
            "DO" : [
              { "component_rules_add_points_from_product" : { "line_item" : [ "list-item" ], "order" : [ "commerce-order" ] } }
            ]
          }
        }
      ]
    }
  }');
  return $items;
}
