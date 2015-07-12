SHELL := /bin/bash -euo pipefail
PATH := node_modules/.bin:$(PATH)

.DELETE_ON_ERROR:

crx.zip: clean
	zip -r $@ .

node_modules: package.json
	npm prune
	npm install
	touch $@

images/icon_base64.svg:
	@cat images/icon.svg | sed "s!IndieFlower\.ttf!data:font/ttf;base64,$(shell base64 images/IndieFlower.ttf)!" > images/icon_base64.svg

.PHONY: clean
clean:
	rm -rf crx.zip node_modules

.PHONY: lint
lint: node_modules
	jshint .
