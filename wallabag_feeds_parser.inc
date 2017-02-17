<?php
/**
 * @file
 * WallabagFeedsParser class.
 */

/**
 * Parses an IMAP stream.
 */
class WallabagFeedsParser extends FeedsParser {



  public function sourceForm($source_config) {
    $form = array();
    
    return $form;
  }

  /**
   * Build configuration form.
   */
  public function configForm(&$form_state) {
    $form = array();
  
    return $form;
  }

  /**
   * Implementation of FeedsParser::parse().
   */
  public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {
    $fetched = $fetcher_result->getRaw();
    $result = new FeedsParserResult();
    if (isset($fetched->_embedded->items)) {
      foreach ($fetched->_embedded->items as  $raw_item)  {
        $tags = [];
        if (isset($raw_item->tags) && is_array($raw_item->tags)) {
          foreach ($raw_item->tags as $tag) {
            $tags[] = $tag->label;
          }
        }
        $raw_item->tags = $tags;
        $result->items[] = (array) $raw_item;       
      }
    }
    return $result;
  }

  /*
   * This defines sources which user's can select to map values to.
   */
  public function getMappingSources() {
    $sources = parent::getMappingSources();
    $sources['is_archive'] = array(
      'name' => t('Is Archive'),
      'description' => t(''),
    );
    $sources['is_starred'] = array(
      'name' => t('Is Starred'),
      'description' => t(''),
    );  
    $sources['user_name'] = array(
      'name' => t('User Name'),
      'description' => t(''),
    );    
    $sources['user_email'] = array(
      'name' => t('User Email'),
      'description' => t(''),
    );    
    $sources['user_id'] = array(
      'name' => t('User ID'),
      'description' => t(''),
    );
    $sources['id'] = array(
      'name' => t('ID'),
      'description' => t(''),
    );    
    $sources['title'] = array(
      'name' => t('Title'),
      'description' => t(''),
    );
    $sources['url'] = array(
      'name' => t('URL'),
      'description' => t(''),
    );    
    $sources['content'] = array(
      'name' => t('Content'),
      'description' => t(''),
    );  
    $sources['created_at'] = array(
      'name' => t('Created At'),
      'description' => t(''),
    );    
    $sources['updated_at'] = array(
      'name' => t('Updated At'),
      'description' => t(''),
    );
    $sources['annotations'] = array(
      'name' => t('Annotations'),
      'description' => t(''),
    );    
    $sources['domain_name'] = array(
      'name' => t('Domain Name'),
      'description' => t(''),
    );    
    $sources['preview_picture'] = array(
      'name' => t('Preview Picture'),
      'description' => t(''),
    ); 
    $sources['tags'] = array(
      'name' => t('Tags'),
      'description' => t('Multiple tags imported as a string concatinated with |.'),
    );     
    
    
    return $sources;
  }
  
}
