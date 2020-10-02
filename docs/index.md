---
layout: default
title: BibExplorer 📚
nav_order: 1
has_children: false
has_toc: false
---
# BibExplorer 📚

> Process curricula, extract article meta-data, and calculate bibliometric indicators

[![BibExplorer](images/indicators_window.png)](https://alandefreitas.github.io/bibexplorer/)

<br/>

Bibliography indicators are a crucial complement to peer reviews. The careless use of indicators reduces the motivation for high-impact publications, creates incentives for unethical practices, leads to injustices, wastes money, and consumes the time of the community. Still, adequate evaluation methods demand elaborate algorithms and treating large volumes of data from multiple data sources. BibExplorer is a tool to process curriculums, extract article meta-data from various APIs, and calculate indicators. BibExplorer can reveal how the various indicators have fluctuated over time with custom time windows, estimate which indicators adequately predict future productivity, contrast the outcomes of indicators, measure their correlation, and provide numerous visualization tools to explore this information.

<br/>

[![Build Status](https://img.shields.io/github/workflow/status/alandefreitas/bibexplorer/Build%20BibExplorer?event=push&label=Build&logo=Github-Actions)](https://github.com/alandefreitas/bibexplorer/actions?query=workflow%3A%22Build+BibExplorer%22+event%3Apush)
[![Latest Release](https://img.shields.io/github/release/alandefreitas/bibexplorer.svg?label=Download)](https://GitHub.com/alandefreitas/bibexplorer/releases/)
[![Docs](https://img.shields.io/website-up-down-green-red/http/alandefreitas.github.io/bibexplorer.svg?label=Documentation)](https://alandefreitas.github.io/bibexplorer/)

<br/>

<!-- https://github.com/bradvin/social-share-urls -->
[![Facebook](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Facebook&logo=facebook)](https://www.facebook.com/sharer/sharer.php?t=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators&u=https://github.com/alandefreitas/bibexplorer/)
[![QZone](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+QZone&logo=qzone)](http://sns.qzone.qq.com/cgi-bin/qzshare/cgi_qzshare_onekey?url=https://github.com/alandefreitas/bibexplorer/&title=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators&summary=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators)
[![Weibo](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Weibo&logo=sina-weibo)](http://sns.qzone.qq.com/cgi-bin/qzshare/cgi_qzshare_onekey?url=https://github.com/alandefreitas/bibexplorer/&title=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators&summary=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators)
[![Reddit](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Reddit&logo=reddit)](http://www.reddit.com/submit?url=https://github.com/alandefreitas/bibexplorer/&title=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators)
[![Twitter](https://img.shields.io/twitter/url/http/shields.io.svg?label=Share+on+Twitter&style=social)](https://twitter.com/intent/tweet?text=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20Bibliometric%20indicators&url=https://github.com/alandefreitas/bibexplorer/&hashtags=Bibliometrics,BibliometricAnalysis,Bibliography,Indicators,ScientificPublications,Science,Research,ScientificVisualization)
[![LinkedIn](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+LinkedIn&logo=linkedin)](https://www.linkedin.com/shareArticle?mini=false&url=https://github.com/alandefreitas/bibexplorer/&title=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators)
[![WhatsApp](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+WhatsApp&logo=whatsapp)](https://api.whatsapp.com/send?text=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators:+https://github.com/alandefreitas/bibexplorer/)
[![Line.me](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Line.me&logo=line)](https://lineit.line.me/share/ui?url=https://github.com/alandefreitas/bibexplorer/&text=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators)
[![Telegram.me](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Telegram.me&logo=telegram)](https://telegram.me/share/url?url=https://github.com/alandefreitas/bibexplorer/&text=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators)
[![HackerNews](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+HackerNews&logo=y-combinator)](https://news.ycombinator.com/submitlink?u=https://github.com/alandefreitas/bibexplorer/&t=BibExplorer:%20a%20tool%20to%20process%20curricula,%20extract%20article%20meta-data%20from%20various%20APIs,%20and%20calculate%20bibliometric%20indicators)

<br/>



- [Examples](examples.md)
  - [Input formats](examples/input-formats.md)
    - [CSV](examples/input-formats/csv.md)
    - [JSON](examples/input-formats/json.md)
    - [Lattes XML](examples/input-formats/lattes-xml.md)
  - [Running BibExplorer](examples/running-bibexplorer.md)
  - [Indicators](examples/indicators.md)
  - [Timeline](examples/timeline.md)
  - [Stability](examples/stability.md)
  - [Compare](examples/compare.md)
  - [Correlations](examples/correlations.md)
  - [Citations](examples/citations.md)
- [Install](install.md)
  - [Packages](install/packages.md)
  - [Build from source](install/build-from-source.md)
    - [Dependencies](install/build-from-source/dependencies.md)
    - [Build Only](install/build-from-source/build-only.md)
    - [Install from Source](install/build-from-source/install-from-source.md)
    - [Building the packages](install/build-from-source/building-the-packages.md)
- [Limitations](limitations.md)
- [Contributing](contributing.md)
  - [Contributors](contributing/contributors.md)
- [References](references.md)


<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
