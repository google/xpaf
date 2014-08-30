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

var prettify = function() {
  var text = $(this).text();
  var makeWrapFn = function(className) {
    var wrapFn = function(str) {
      return '<span class="' + className + '">' + str + '</span>';
    };
    return wrapFn;
  };
  var lines = text.split('\n');
  for (var i = 0; i < lines.length; i++) {
    var line = lines[i];
    line = line.replace(/</g, '&lt;');
    line = line.replace(/>/g, '&gt;');
    line = line.replace(/"[^\"]*"/g, makeWrapFn('literal'));
    line = line.replace(/\/\*.*\*\//g, makeWrapFn('comment'));
    // Note that we don't allow "//" comments on the same line as code.
    line = line.replace(/^ *\/\/.*/g, makeWrapFn('comment'));
    lines[i] = line;
  }
  text = lines.join('\n');
  return text;
};

$('.code-block').contents().replaceWith(prettify);
