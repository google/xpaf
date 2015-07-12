// python -m SimpleHTTPServer 8001
// http://localhost:8001/index.html

'use strict';

var CLICK = true;

function addImage(src, width, height) {
  var image = new Image();
  image.src = src;
  image.onload = function() {
    var c = document.createElement('canvas');
    document.body.appendChild(c);

    c.width = width || image.width;
    c.height = height || image.height;
    c.getContext('2d').drawImage(image, 0, 0, c.width, c.height);

    var a = document.createElement('a');
    a.download = src.split('.')[0] + '_' + c.width + 'x' + c.height + '.png';
    a.href = c.toDataURL('image/png');
    if (CLICK) {
      a.dispatchEvent(new MouseEvent('click', {
        view: window,
        bubbles: true,
        cancelable: true
      }));
    }
  };
}

var src = 'icon_base64.svg';
addImage(src, 16, 16);
addImage(src, 19, 19);
addImage(src, 32, 32);
addImage(src, 38, 38);
addImage(src, 48, 48);
addImage(src, 128, 128);

addImage('promo.svg', 440, 280);
