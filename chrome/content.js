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

// Extension namespace.
var xh = xh || {};


////////////////////////////////////////////////////////////////////////////////
// Generic helper functions and constants

xh.SHIFT_KEYCODE = 16;
xh.X_KEYCODE = 88;

xh.bind = function(object, method) {
  return function() {
    return method.apply(object, arguments);
  };
};

xh.elementsShareFamily = function(primaryEl, siblingEl) {
  if (primaryEl.tagName === siblingEl.tagName &&
      (!primaryEl.className || primaryEl.className === siblingEl.className) &&
      (!primaryEl.id || primaryEl.id === siblingEl.id)) {
    return true;
  }
  return false;
};

xh.getElementIndex = function(el) {
  var className = el.className;
  var id = el.id;

  var index = 1;  // XPath is one-indexed
  var sib;
  for (sib = el.previousSibling; sib; sib = sib.previousSibling) {
    if (sib.nodeType === Node.ELEMENT_NODE && xh.elementsShareFamily(el, sib)) {
      index++;
    }
  }
  if (index > 1) {
    return index;
  }
  for (sib = el.nextSibling; sib; sib = sib.nextSibling) {
    if (sib.nodeType === Node.ELEMENT_NODE && xh.elementsShareFamily(el, sib)) {
      return 1;
    }
  }
  return 0;
};

xh.makeQueryForElement = function(el) {
  var query = '';
  for (; el && el.nodeType === Node.ELEMENT_NODE; el = el.parentNode) {
    var component = el.tagName.toLowerCase();
    var index = xh.getElementIndex(el);
    if (el.id) {
      component += '[@id=\'' + el.id + '\']';
    } else if (el.className) {
      component += '[@class=\'' + el.className + '\']';
    }
    if (index >= 1) {
      component += '[' + index + ']';
    }
    // If the last tag is an img, the user probably wants img/@src.
    if (query === '' && el.tagName.toLowerCase() === 'img') {
      component += '/@src';
    }
    query = '/' + component + query;
  }
  return query;
};

xh.highlightNodes = function(nodes) {
  for (var i = 0, l = nodes.length; i < l; i++) {
    nodes[i].className += ' xh-highlight';
  }
};

xh.clearHighlights = function() {
  var els = document.getElementsByClassName('xh-highlight');
  // Note: getElementsByClassName() returns a live NodeList.
  while (els.length) {
    els[0].className = els[0].className.replace(' xh-highlight', '');
  }
};

// Returns [values, nodeCount]. Highlights result nodes, if applicable. Assumes
// no nodes are currently highlighted.
xh.evaluateQuery = function(query) {
  var xpathResult = null;
  var str = '';
  var nodeCount = 0;
  var nodesToHighlight = [];

  try {
    xpathResult = document.evaluate(query, document, null,
                                    XPathResult.ANY_TYPE, null);
  } catch (e) {
    str = '[INVALID XPATH EXPRESSION]';
    nodeCount = 0;
  }

  if (!xpathResult) {
    return [str, nodeCount];
  }

  if (xpathResult.resultType === XPathResult.BOOLEAN_TYPE) {
    str = xpathResult.booleanValue ? '1' : '0';
    nodeCount = 1;
  } else if (xpathResult.resultType === XPathResult.NUMBER_TYPE) {
    str = xpathResult.numberValue.toString();
    nodeCount = 1;
  } else if (xpathResult.resultType === XPathResult.STRING_TYPE) {
    str = xpathResult.stringValue;
    nodeCount = 1;
  } else if (xpathResult.resultType ===
             XPathResult.UNORDERED_NODE_ITERATOR_TYPE) {
    for (var it = xpathResult.iterateNext(); it;
         it = xpathResult.iterateNext()) {
      nodesToHighlight.push(it);
      if (str) {
        str += '\n';
      }
      str += it.textContent;
      nodeCount++;
    }
    if (nodeCount === 0) {
      str = '[NULL]';
    }
  } else {
    // Since we pass XPathResult.ANY_TYPE to document.evaluate(), we should
    // never get back a result type not handled above.
    str = '[INTERNAL ERROR]';
    nodeCount = 0;
  }

  xh.highlightNodes(nodesToHighlight);
  return [str, nodeCount];
};


////////////////////////////////////////////////////////////////////////////////
// xh.Bar class definition

xh.Bar = function() {
  this.boundShowBar_ = xh.bind(this, this.showBar_);
  this.boundHandleRequest_ = xh.bind(this, this.handleRequest_);
  this.boundMouseMove_ = xh.bind(this, this.mouseMove_);
  this.boundKeyDown_ = xh.bind(this, this.keyDown_);

  chrome.extension.onMessage.addListener(this.boundHandleRequest_);

  this.barFrame_ = document.createElement('iframe');
  this.barFrame_.src = chrome.extension.getURL('bar.html');
  this.barFrame_.id = 'xh-bar';
  this.barFrame_.className = 'top';
  this.barFrame_.style.height = '0';

  // Temporarily make bar 'hidden' and add it to the DOM. Once the bar's html
  // has loaded, it will send us a message with its height, at which point we'll
  // set this.barHeightInPx_, remove it from the DOM, and make it 'visible'.
  // We'll add it back to the DOM on the first bar request.
  this.barFrame_.style.visibility = 'hidden';
  document.body.appendChild(this.barFrame_);

  document.addEventListener('keydown', this.boundKeyDown_);
};

xh.Bar.prototype.active_ = false;
xh.Bar.prototype.barFrame_ = null;
xh.Bar.prototype.barHeightInPx_ = 0;
xh.Bar.prototype.currEl_ = null;
xh.Bar.prototype.boundHandleRequest_ = null;
xh.Bar.prototype.boundMouseMove_ = null;
xh.Bar.prototype.boundKeyDown_ = null;

xh.Bar.prototype.updateQueryAndBar_ = function(el) {
  xh.clearHighlights();
  this.query_ = el ? xh.makeQueryForElement(el) : '';
  this.updateBar_(true);
};

xh.Bar.prototype.updateBar_ = function(update_query) {
  var results = this.query_ ? xh.evaluateQuery(this.query_) : ['', 0];
  var request = {
    'type': 'update',
    'query': update_query ? this.query_ : null,
    'results': results
  };
  chrome.extension.sendMessage(request);
};

xh.Bar.prototype.showBar_ = function() {
  this.barFrame_.style.height = this.barHeightInPx_ + 'px';
  document.addEventListener('mousemove', this.boundMouseMove_);
  this.updateBar_(true);
};

xh.Bar.prototype.hideBar_ = function() {
  // Note: It's important to set this.active_ to false here rather than in
  // keyDown_() because hideBar_() could be called via handleRequest_().
  this.active_ = false;
  xh.clearHighlights();
  document.removeEventListener('mousemove', this.boundMouseMove_);
  this.barFrame_.style.height = '0';
};

xh.Bar.prototype.handleRequest_ = function(request, sender, callback) {
  if (request['type'] === 'height' && this.barHeightInPx_ === 0) {
    this.barHeightInPx_ = request['height'];
    // Now that we've saved the bar's height, remove it from the DOM and make it
    // 'visible'.
    document.body.removeChild(this.barFrame_);
    this.barFrame_.style.visibility = 'visible';
  } else if (request['type'] === 'evaluate') {
    xh.clearHighlights();
    this.query_ = request['query'];
    this.updateBar_(false);
  } else if (request['type'] === 'relocateBar') {
    // Move iframe to a different part of the screen.
    this.barFrame_.className = (
      this.barFrame_.className === 'top' ? 'middle' : 'top');
  } else if (request['type'] === 'hideBar') {
    this.hideBar_();
    window.focus();
  }
};

xh.Bar.prototype.mouseMove_ = function(e) {
  if (this.currEl_ === e.toElement) {
    return;
  }
  this.currEl_ = e.toElement;
  if (e.shiftKey) {
    this.updateQueryAndBar_(this.currEl_);
  }
};

xh.Bar.prototype.keyDown_ = function(e) {
  if (e.keyCode === xh.X_KEYCODE && e.ctrlKey && e.shiftKey) {
    if (!this.active_) {
      this.active_ = true;
      if (!this.barFrame_.parentNode) {
        // First bar request on this page. Add bar back to DOM.
        document.body.appendChild(this.barFrame_);
        // Use setTimeout so that the transition is visible.
        window.setTimeout(this.boundShowBar_, 0);
      } else {
        this.showBar_();
      }
    } else {
      this.hideBar_();
    }
  }

  // If the user just pressed Shift and they're not holding Ctrl, update query.
  // Note that we rely on the mousemove handler to have updated this.currEl_.
  // Also, note that checking e.shiftKey wouldn't work here, since Shift is the
  // key that triggered this event.
  if (this.active_ && e.keyCode === xh.SHIFT_KEYCODE && !e.ctrlKey) {
    this.updateQueryAndBar_(this.currEl_);
  }
};


////////////////////////////////////////////////////////////////////////////////
// Initialization code

if (window['xhBarInstance']) {
  window['xhBarInstance'].dispose();
}
if (location.href.indexOf('acid3.acidtests.org') === -1) {
  window['xhBarInstance'] = new xh.Bar();
}
