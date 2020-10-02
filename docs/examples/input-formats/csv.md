---
layout: default
title: CSV
nav_order: 1
has_children: false
parent: Input formats
grand_parent: Examples
has_toc: false
---
# CSV

A CSV file should have the following columns:

* Author ID
* Author
* Paper ID
* Title
* Journal
* Year
* Authors
* Citations

See an example in [`resources/examples/input/ringo.csv`](https://github.com/alandefreitas/bibexplorer/blob/master/resources/examples/input/ringo.csv). 

Even though there are columns for "Author ID" and "Author", a CSV file used as input should have information for a single author. This redundant information is used to make it compatible with the CSV files later generated to the output directory.

The "Authors" should be separated by commas so that BibExplorer can properly identify the number of authors in each paper.





<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
