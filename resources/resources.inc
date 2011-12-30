<?php

/**
 * @file
 * .inc file that embed Resources Resource callbacks.
 *
 */

/**
 * Function that check accessibility of the listing.
 *
 * @param mixed $args
 *   Arguments ggathered by the server.
 *
 * @return bool
 *   Is the listing accessible under certain conditions.
 */
function resources_resource_access($args) {
  return TRUE;
}

/**
 * The actual function that construct the Resources' Listing.
 */
function resources_listing() {
  module_load_include('inc', 'services', 'services.resource_build');
  $server_infos = services_server_info_object();
  list($supposed_endpoint_path) = explode('/', $_GET['q']);
  if ($server_infos->endpoint_path === $supposed_endpoint_path) {
    $endpoint = services_endpoint_load($server_infos->endpoint);
    $user = services_get_server_info('original_user');
    $resources = _services_build_resources($server_infos->endpoint);
    $wsdl_array = ResourcesToWSDL::init($resources);
    $out = $wsdl_array->render();
  }
  else {
    drupal_not_found();
  }
  return $out;
}

/**
 * Translate resources array into WSDL-like.
 */
class ResourcesToWSDL {

  static private $instance, $types;
  protected $resources, $port = array(), $messages = array();

  /**
   * Constructor
   */
  private function __construct(array $resources) {
    $this->resources = $resources;
    $this->addTypes($resources);
  }

  /**
   * Main entry point for this singleton.
   *
   * @param array $resources
   *   the resources available for a endpoint.
   *
   * @return ResourcesToWSDL
   *   the object used to render the array.
   */
  static public function init(array $resources) {
    if (!is_array(self::$types)) {
      self::$types = array();
    }
    if (!is_a(self::$instance, __CLASS__)) {
      $my_name = __CLASS__;
      self::$instance = new $my_name($resources);
    }
    return self::$instance;
  }

  /**
   * Recursively run throught an array of arrays of… and so on… of resources to extract the different
   * types and send each to $this->addtype (notice the lack of "s" at the end).
   *
   * @param array $resources
   */
  protected function addTypes(array $resources) {
    if (isset($resources['type'])) {
      $this->addType($resources['type']);
    }
    else {
      foreach ($resources as $sr) {
        if (is_array($sr)) {
          $this->addTypes($sr);
        }
      }
    }
  }

  /**
   * Register a type of resource.
   *
   * @param type $type
   */
  protected function addType($type) {
    // Only if we didn't do it already !
    if (!is_numeric(array_search($type, self::$types))) {
      self::$types[] = $type;
    }
  }

  /**
   * Simple Getter for registered Resources' Types.
   *
   * @return array
   *   All the registered Resources' Types.
   */
  static public function getTypes() {
    return self::$types;
  }

  /**
   * /!\ Warning /!\ If you attend to debug this recursive function, you MUST use xDebug or any
   * debugger that level of efficiency ! Seriously !
   *
   * There's at least three different kind of Resources' describing Arrays.
   * I just can't describe the whole process here and I think ranting about services inner conceptions
   * points is useless, since I don't have time to contribute to it.
   *
   * @param array $resource
   *   The Resources containing Array.
   * @param type $base_name
   *   You MUSTN'T use this by yourself, it's only for the recursive parsing !
   * @param type $inside_endpoint
   *   You MUSTN'T use this by yourself, it's only for the recursive parsing !
   *
   * @return string
   */
  protected function parsePorts(array $resource, $base_name = '',
    $inside_endpoint = FALSE) {
    $out = array();
    // Normaly, the first Pass skip this directly because we aren't in a Resource yet, nor in a
    // endpoint.
    if (isset($resource['endpoint'])) {
      foreach ($resource['endpoint'] as $key => $value) {
        $sub_endpoint_level = $this->parsePorts($value, $key, TRUE);
        if (is_array($sub_endpoint_level)) {
          $ports = array();
          foreach ($sub_endpoint_level as $name) {
            $this->port[] = $base_name . ucfirst($name);
          }
        }
        else {
          $this->port[] = $base_name . $this->parsePorts($value, $key, TRUE);
        }
      }
    }
    elseif ($inside_endpoint) {
      $keys = array_keys($resource);
      $sub_endpoint_out = array();

      // Check if we are really "at the bottom" of the array, then return the base name.
      if ($resource[$keys[0]] == 1) {
        return ucfirst($base_name);
      }
      else {
        foreach ($resource as $key => $value) {
          $sub_endpoint_out[] = ucfirst($base_name) . $this->parsePorts($value,
              $key, TRUE);
        }
        return $sub_endpoint_out;
      }
    }
    else {
      foreach ($resource as $key => $value) {
        $tmpout = $this->parsePorts($value, $key);
        if (is_array($tmpout)) {
          foreach ($tmpout as $value) {
            $this->port[] = $base_name . $value;
          }
        }
      }
    }
  }

  /**
   * Same thing here, just like "parsePorts"
   *
   * @param type $resources
   * @param type $trail
   * @param type $in_args
   *
   * @return type
   */
  protected function parseMessages($resources, $trail = '', $in_args = FALSE) {
    $results = array();
    if ($in_args) {
      $methods = array(
        'data' => 'POST',
        'path' => 'GET|path',
        'param' => 'GET|param',
      );
      $results[$resources['name']]['type'] = $resources['type'];

      if (is_array($resources['source'])) {
        foreach ($resources['source'] as $name => $value) {
          $method_name = isset($methods[$name])
            ? $methods[$name]
            : $name;
          $results[$resources['name']]['method'] = $method_name . '|' . $value;
        }
      }
      else {
        $method_name = isset($methods[$resources['source']])
          ? $methods[$resources['source']]
          : $resources['source'];
        $results[$resources['name']]['method'] = $method_name . '|' . $resources['name'];
      }
    }
    elseif (isset($resources['args']) && is_array($resources['args'])) {
      foreach ($resources['args'] as $arg) {
        $results = array_merge($results,
          $this->parseMessages($arg, $trail, TRUE));
      }
    }
    else {
      if (is_array($resources)) {
        foreach ($resources as $name => $value) {
          $out = $this->parseMessages($value, $trail . ucfirst($name));
          if ($out) {
            if(in_array($name, array('retrieve', 'actions'))) {
              $results = $this->parseMessages($value,
                $trail . ucfirst($name));
            }
            else {
            $results[$name] = $this->parseMessages($value,
              $trail . ucfirst($name));
            }
          }
        }
      }
    }
    $results = array_filter($results);
    return $results;
  }

  /**
   * The only useful function.
   * @return array
   *   the WSDL-like one.
   */
  public function render() {
    $this->parsePorts($this->resources);
    $out = $this->parseMessages($this->resources);
//    $out['resources'] = $this->resources;

    return $out;
  }

}
