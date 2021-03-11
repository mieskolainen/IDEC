<?php
  // ---------------- CONFIGURABLE SECTION -----------------

  // Please modify the following or it will not work on
  // your website.

  // Where did you actually put your images?
  // Make sure that the path you put below ends with
  // a directory slash ("/"). The script below assumes it.
  $imagedir = "/disk/home/images/" ;

  // What are the websites (hostnames) that can use this
  // image?
  // If your site can be accessed with or without the
  // "www" prefix, make sure you put both here. Do not put
  // any trailing slashes ("/") nor any "http://" prefixes.
  // Follow the example below.
  $validprefixes = array (
    "thesitewizard.com",
    "www.thesitewizard.com"
  ) ;

  // What is the main page of your website? Visitors will
  // be directed here if they type
  //   "http://www.example.com/chimage.php"
  // in their browser.
  $homepage = "http://www.thesitewizard.com/" ;

  // What is your email address?
  // If you want to be informed when someone tries to use
  // this script to access an image illegitimately, you
  // must uncomment (remove the "//" prefix) the following
  // line and change it to point to your email address.
  //$email = "yourname@example.com" ;

  // ------------ END OF CONFIGURABLE SECTION ------------


  // --- YOU NEED NOT MODIFY ANYTHING AFTER THIS LINE ---

  function isreferrerokay ( $referrer, $validprefixes )
  {
    $validreferrer = 0 ;
    $authreferrer  = current( $validprefixes );
    while ($authreferrer) {
      if (eregi( "^https?://$authreferrer/", $referrer )) {
        $validreferrer = 1 ;
        break ;
      }
      $authreferrer = next( $validprefixes );
    }
    return $validreferrer ;
  }

  //----------------------- main program -----------------------

  $image = $_GET['image'] ;
  $referrer = getenv( "HTTP_REFERER" );

  if (isset($_GET['image'])) {

    if (empty($referrer) ||
      isreferrerokay( $referrer, $validprefixes )) {

      $imagepath = $imagedir . $image ;

      $imageinfo = getimagesize( $imagepath );
      if ($imageinfo[2] == 1) {
        $imagetype = "gif" ;
      }
      elseif ($imageinfo[2] == 2) {
        $imagetype = "jpeg" ;
      }
      elseif ($imageinfo[2] == 3) {
        $imagetype = "png" ;
      }
      else {
        header( "HTTP/1.0 404 Not Found" );
        exit ;
      }

      header( "Content-type: image/$imagetype" );
      @readfile( $imagepath );

    }
    else {

      if (isset($email)) {
        mail( $email, "Bandwidth Theft Alert",
           "WARNING:\n\n$referrer\ntried to access\n$image\n",
           "From: CHImageGuard <$email>" );
      }
      header( "HTTP/1.0 404 Not Found" );
    }
  }
  else {
    header( "Location: $homepage" );
  }

?>