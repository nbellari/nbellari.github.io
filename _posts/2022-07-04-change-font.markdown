---
layout: post
title: "On Changing Fonts in Jekyll Theme"
categories: blogging
date:  2022-07-04 12:27:57 +0530
---

I liked the Jekyll Theme - for its simplicity and also its mono font. I took a lot of pains to replice the Jekyll theme to my blogging space and I found I still dont get that font.

In `_sass/partials/_variables.scss` file there is a variable called `primary-font` which lists the set of fonts in an order - as to which one to use in case the previous one is not available. Then I realized I dont have `Ubuntu-Mono` font on my machine, I downloaded and installed it and voila!, I got what I wanted.
