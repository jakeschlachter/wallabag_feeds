<?php

/**
 * @file
 *
 * Provides the Wallabag api fetcher to feeds.
 */
define('PERPAGE', 10);

/**
 * Definition of the import batch object created on the fetching stage by
 * wallabag_feeds_fetcher.
 */
class WallabagFetcherResult extends FeedsFetcherResult {
  protected $wallabagApi;
  protected $sort;
  protected $perPage;
   
  /**
   * Constructor.
   */
  public function __construct($wallabagApi, $updated_content) {
    parent::__construct('');
    $this->sort = ($updated_content) ? 'updated' : 'created';
    $this->wallabagApi = $wallabagApi;
    $this->perPage = PERPAGE;
  }

  
 /**
   * Overrides parent::getRaw().
   */
  public function getRaw() {
    return $this->raw;
  }  
  
  /*
   * Grabs the next piece of content from the api and stores it in raw for a parcer to digest
   *   Returns the next piece if there is one
   */
  public function pullNextPage($page) {
    $this->raw = $this->wallabagApi->call('entries', array('page' => $page, 'perPage' => $this->perPage, '$sort' => $this->sort, 'order' => 'asc'));
  }
  
/*
   * Grabs the next piece of content from the api and stores it in raw for a parcer to digest
   *   Returns the next piece if there is one
   */
  public function pullNextSince($since) {
    $this->raw = $this->wallabagApi->call('entries', array('perPage' => $this->perPage, 'since' => $since, '$sort' => $this->sort, 'order' => 'asc'));
  }  
  
  public function getPage() {
    if (isset($this->raw->_embedded->items) && !empty($this->raw->_embedded->items)) {
      return (isset($this->raw->page)) ? $this->raw->page : false;
    }
    return false;
  }
  
  public function getPages() {
    if (isset($this->raw->_embedded->items) && !empty($this->raw->_embedded->items)) {
      return (isset($this->raw->pages)) ? $this->raw->pages : false;
    }
    return false;
  }
  
  public function getPerPage() {
    return (isset($this->perPage)) ? $this->perPage : false;
  }  
  
  public function getTimeStamp() {
    $time_field = $this->sort. '_at';
    $time_stamp = false;
    if (isset($this->raw->_embedded->items) && !empty($this->raw->_embedded->items)) {
      //grab the last one
      $lastEl = array_values(array_slice($this->raw->_embedded->items, -1))[0];
      $time_stamp = strtotime($lastEl->{$time_field});
    }
    return $time_stamp;
  }
  
}

/**
 * Fetches a file from a specified directory.
 *
 * This fetcher will keep track of the files that it has already fetched, and
 * will not re-fetch them, unless specifically told to.
 */
class WallabagFeedsFetcher extends FeedsFetcher {

 /**
   * Fetch content from a source and return it.
   *
   * Every class that extends FeedsFetcher must implement this method.
   *
   * @param $source
   *   Source value as entered by user through sourceForm().
   */
  public function fetch(FeedsSource $source) {
    $state = $source->state(FEEDS_FETCH);

    $source_config = $source->getConfigFor($this);
    $wallabagApi = new wallabagApi(
            $source_config['wallabag_url'], 
            $source_config['wallabag_key'],
            $source_config['wallabag_secret'],
            $source_config['wallabag_user'],
            $source_config['wallabag_user_password']);
      
    $wallabagFetcherResult = new WallabagFetcherResult($wallabagApi, $this->config['updated_content']);
    
    if ($state->next) {
      //we are batching so grab the next content
      $wallabagFetcherResult->pullNextPage($state->next);
    }    
    else {
      // If we're not batching, scan for new or updated content.
      $wallabagFetcherResult->pullNextSince($source_config['wallabag_last']);
    }
   
    $total = $wallabagFetcherResult->getPages();
    $page = $wallabagFetcherResult->getPage();
    
    if ($total - $page >= 0) {
      $state->total = $total;
      $state->progress($total, $page);
      $state->next = $page + 1;
    }
    
    // Save the modification time of the content in my config if a new content item was returned.
    if ($time_stamp = $wallabagFetcherResult->getTimeStamp()) {
      $source_config['wallabag_last'] = $time_stamp + 1;
      $blah = $source->getConfig();
      $blah[get_class($this)] = $source_config;
      $source->setConfig($blah);         
    }
      
    return $wallabagFetcherResult;    

    // Return an empty FeedsFetcherResult if we didn't get anything from the directory:
    //throw new Exception(t('No new files found in: %source to fetch.', array('%source' => $source_config['source'])));
  }

 /**
   * Source form.
   */
  public function sourceForm($source_config) {
    $form = array();
    // When renaming, do not forget feeds_vews_handler_field_source class.
    $form['wallabag_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Wallabag URL'),
      '#description' => t('Specify a URL to pull feed from, e.g. app.wallabag.it or localhost:8000.'),
      '#default_value' => (isset($source_config['wallabag_url']) && !empty($source_config['wallabag_url'])) ? $source_config['wallabag_url'] : 'app.wallabag.it',
      '#required' => TRUE,
    );
    $wallabag_text = t('You can create a new client !new_client or view existing one !existing_client if you have an account with wallabag. Otherwise they can be created on your local client.', 
            array('!new_client' => '<a href="https://app.wallabag.it/developer/client/create">here</a>', '!existing_client' => '<a href="https://app.wallabag.it/developer">here</a>'));
    $form['wallabag_key'] = array(
      '#type' => 'textfield',
      '#title' => t('Wallabag Client Key'),
      '#description' => t('Client Key'). ' : '. $wallabag_text,
      '#default_value' => (isset($source_config['wallabag_key']) && !empty($source_config['wallabag_key'])) ? $source_config['wallabag_key'] : '',
      '#required' => TRUE,
    );
    $form['wallabag_secret'] = array(
      '#type' => 'textfield',
      '#title' => t('Wallabag Client Secret'),
      '#description' => t('Wallabag Client Secret'). ' : '. $wallabag_text,
      '#default_value' => (isset($source_config['wallabag_secret']) && !empty($source_config['wallabag_secret'])) ? $source_config['wallabag_secret'] : '',
      '#required' => TRUE,
    );
    $form['wallabag_user'] = array(
      '#type' => 'textfield',
      '#title' => t('Wallabag User'),
      '#description' => t('Specify Wallabag user account to pull content from.'),
      '#default_value' => (isset($source_config['wallabag_user']) && !empty($source_config['wallabag_user'])) ? $source_config['wallabag_user'] : '',
      '#required' => TRUE,
    );   
    $form['wallabag_user_password'] = array(
      '#type' => 'textfield',
      '#title' => t('Wallabag User Password'),
      '#description' => t("Specify the Wallabag user's password."),
      '#default_value' => (isset($source_config['wallabag_user_password']) && !empty($source_config['wallabag_user_password'])) ? $source_config['wallabag_user_password'] : '',
      '#required' => TRUE,
    ); 
    
    $form['reset'] = array(
      '#type' => 'checkbox',
      '#title' => t('Re-fetch entire Wallabag account'),
      '#description' => t('When checked will re-fetch previously imported data.'),
      '#default_value' => FALSE,
    );
    
    //track the timestamp of the last imported content 
    $form['wallabag_last'] = array(
      '#type' => 'value',
      '#value' => isset($source_config['wallabag_last']) ? $source_config['wallabag_last'] : 0,
    );    
    return $form;
  }

 /**
   * Override parent::sourceFormValidate().
   */
  public function sourceFormValidate(&$values) {
    // Check that the system can access the wallabag api specified
    $wallabagApi = new wallabagApi(
            $values['wallabag_url'], 
            $values['wallabag_key'],
            $values['wallabag_secret'],
            $values['wallabag_user'],
            $values['wallabag_user_password']);    
    
    if (!$wallabagApi->get_access_token()) {
      form_set_error('', t('Unable to log into Wallabag account.'));
    }
    if (!empty($values['reset'])) {
      $values['wallabag_last'] = 0;
      $values['reset'] = 0;
    }
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    $defaults = parent::configDefaults();
    $defaults['updated_content'] = TRUE;
    return $defaults;
  }  
  
  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = array();
    
    $form['updated_content'] = array(
      '#type' =>'checkbox',
      '#title' => t('Re-fetch content that is modified'),
      '#default_value' => $this->config['updated_content'],
    );
   
    return $form;
  }

}
