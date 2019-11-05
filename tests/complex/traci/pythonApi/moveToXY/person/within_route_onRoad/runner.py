#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2008-2019 German Aerospace Center (DLR) and others.
# This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v2.0
# which accompanies this distribution, and is available at
# http://www.eclipse.org/legal/epl-v20.html
# SPDX-License-Identifier: EPL-2.0

# @file    runner.py
# @author  Jakob Erdmann
# @date    2015-02-06
# @version $Id$


from __future__ import print_function
from __future__ import absolute_import
import os
import sys
sys.path.append(os.path.join(
    os.path.dirname(sys.argv[0]), "..", "..", "..", "..", "..", "..", "..", "tools"))
import traci  # noqa
import sumolib  # noqa

sumoBinary = sumolib.checkBinary('sumo')
cmd = [
    sumoBinary,
    "-n", "input_net2.net.xml",
    "-r", "input_routes.rou.xml",
    "--fcd-output", "fcd.xml",
    "--no-step-log"]
traci.start(cmd)


def step():
    s = traci.simulation.getTime()
    traci.simulationStep()
    return s


p = "p0"
s = step()
x, y = traci.person.getPosition(p)
print("s=%s x=%s y=%s" % (s, x, y))
print("jumping onto the road on the same edge")
traci.person.moveToXY(p, "", x - 5, y)
for i in range(20):
    s = step()
    x, y = traci.person.getPosition(p)
    print("s=%s x=%s y=%s" % (s, x, y))
step()
traci.close()
