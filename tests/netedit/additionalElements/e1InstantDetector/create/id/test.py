#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2019 German Aerospace Center (DLR) and others.
# This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v2.0
# which accompanies this distribution, and is available at
# http://www.eclipse.org/legal/epl-v20.html
# SPDX-License-Identifier: EPL-2.0

# @file    test.py
# @author  Pablo Alvarez Lopez
# @date    2016-11-25
# @version $Id$

# import common functions for netedit tests
import os
import sys

testRoot = os.path.join(os.environ.get('SUMO_HOME', '.'), 'tests')
neteditTestRoot = os.path.join(
    os.environ.get('TEXTTEST_HOME', testRoot), 'netedit')
sys.path.append(neteditTestRoot)
import neteditTestFunctions as netedit  # noqa

# Open netedit
neteditProcess, referencePosition = netedit.setupAndStart(neteditTestRoot)

# go to additional mode
netedit.additionalMode()

# select E1
netedit.changeElement("instantInductionLoop")

# Create E1
netedit.leftClick(referencePosition, 140, 250)

# enable ID
netedit.changeDefaultBoolValue(2)

# Create E1
netedit.leftClick(referencePosition, 160, 250)

# set a invalid  id (duplicated)
netedit.changeDefaultValue(3, "instantInductionLoop_gneE0_0_0")

# try to create E1 with invalid id
netedit.leftClick(referencePosition, 180, 250)

# set a invalid  id
netedit.changeDefaultValue(3, ";;;")

# try to create E1 with invalid id
netedit.leftClick(referencePosition, 200, 250)

# set a valid id
netedit.changeDefaultValue(3, "customID")

# create E1 with valid id
netedit.leftClick(referencePosition, 220, 250)

# Check undo redo
netedit.undo(referencePosition, 3)
netedit.redo(referencePosition, 3)

# save additionals
netedit.saveAdditionals(referencePosition)

# save network
netedit.saveNetwork(referencePosition)

# quit netedit
netedit.quit(neteditProcess)
