/**
 * Copyright 2011 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @author opensource@google.com
 * @license Apache License, Version 2.0.
 */

'use strict';

var kIconHide = 'toggle_minus.png';
var kIconHideBg = 'url(' + kIconHide + ')';
var kIconShow = 'toggle_plus.png';
var kIconShowBg = 'url(' + kIconShow + ')';

var kTextHide = 'Hide parser output';
var kTextShow = 'Show parser output';

// Preload images to avoid flickering.
// http://chipsandtv.com/articles/jquery-image-preload
// http://api.jquery.com/jQuery/
var loadImage = function(url) {
  $('<img>').attr('src', url);
};
loadImage(kIconHide);
loadImage(kIconShow);

$('.toggle-icon').css('background-image', kIconShowBg);
$('.toggle-text').text(kTextShow);

var onToggle = function(e) {
  var rowEl = $(e.target).closest('.toggle-row');
  var valueEl = rowEl.siblings('.toggle-value').first();
  var wasHidden = (valueEl.css('display') == 'none');
  valueEl.slideToggle(100 /* milliseconds */, null /* callback */);
  rowEl.find('.toggle-icon').css(
    'background-image', wasHidden ? kIconHideBg : kIconShowBg);
  rowEl.find('.toggle-text').text(wasHidden ? kTextHide : kTextShow);
  return false;  // prevent jump-to-top for href="#"
};

$('.toggle-clickable').click(onToggle);
