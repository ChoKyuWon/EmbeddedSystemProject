#!/bin/bash

clang-format $1 > $1.bak || rm $1
mv $1.bak $1