ICON := "images/icon.svg"

icons:
	@cat images/icon.svg | sed "s!IndieFlower\.ttf!data:font/ttf;base64,$(shell base64 images/IndieFlower.ttf)!" > images/icon_base64.svg

	true || rsvg-convert -h 16 images/icon_base64.svg > static/icon16.png
	true || rsvg-convert -h 19 images/icon_base64.svg > static/icon19.png
	rsvg-convert -h 32 images/icon_base64.svg > static/icon32.png
	rsvg-convert -h 48 images/icon_base64.svg > static/icon48.png
	rsvg-convert -h 128 images/icon_base64.svg > static/icon128.png

.PHONY: icons
