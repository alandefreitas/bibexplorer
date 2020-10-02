---
layout: default
title: JSON
nav_order: 2
has_children: false
parent: Input formats
grand_parent: Examples
has_toc: false
---
# JSON

A JSON file should have the following fields:

* Author ID
* Author
* Papers

See an example in [`resources/examples/input/satoshi.json`](https://github.com/alandefreitas/bibexplorer/blob/master/resources/examples/input/satoshi.json). 

The "Author ID" field might have a number or a string. And "Papers" is a list of papers where each paper has the following fields:

* Paper ID
* Title
* Journal
* Year
* Authors
* Citations

The "Paper ID" can also be a string or a number. The "Authors" field should be a list of strings.





<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
