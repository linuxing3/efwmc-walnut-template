#!/usr/bin/env bash
xrepo add-repo private-repo https://github.com/linuxing3/xmake-repo.git

git clone https://github.com/linuxing3/efwmc-walnut-template.git test-walnut-app

cd test-walnut-app || exit
xmake && xmake r
