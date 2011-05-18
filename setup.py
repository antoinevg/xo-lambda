#!/usr/bin/env python
try:
  from sugar.activity import bundlebuilder
  bundlebuilder.start()
except ImportError:
  print 'Cannot find a working sugar environment'

