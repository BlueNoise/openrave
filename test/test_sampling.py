# -*- coding: utf-8 -*-
# Copyright (C) 2011 Rosen Diankov <rosen.diankov@gmail.com>
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
from openravepy import *
from numpy import *
from itertools import izip, combinations
import nose
from common_test_openrave import *

class TestKinematics(EnvironmentSetup):
    def _runsampler(self,samplername):
        sp=RaveCreateSpaceSampler(self.env,samplername)
        for type in SampleDataType.values.values():
            if sp.Supports(type):
                for dim in [1,5]:
                    sp.SetSpaceDOF(dim)
                    lower,upper = sp.GetLimits(type)
                    assert(len(lower)==dim and len(upper) == dim)
                    for N in [1,1000]:
                        lowerN = tile(lower,(N,1))
                        upperN = tile(upper,(N,1))
                        closedvalues = sp.SampleSequence(type,N,Interval.Closed)
                        openvalues = sp.SampleSequence(type,N,Interval.Open)
                        openendvalues = sp.SampleSequence(type,N,Interval.OpenEnd)
                        openstartvalues = sp.SampleSequence(type,N,Interval.OpenStart)
                        assert(len(closedvalues) == N and all(closedvalues>=lowerN) and all(closedvalues<=upperN))
                        assert(len(openvalues) == N and all(openvalues>lowerN) and all(openvalues<upperN))
                        assert(len(openendvalues) == N and all(openendvalues>=lowerN) and all(openendvalues<upperN))
                        assert(len(openstartvalues) == N and all(openstartvalues>lowerN) and all(openstartvalues<=upperN))
                        
    def test_default(self):
        sp=RaveCreateSpaceSampler(self.env,'MT19937')
        assert(sp.Supports(SampleDataType.Real) and sp.Supports(SampleDataType.Uint32))
        assert(sp.GetDOF()==1 and sp.GetNumberOfValues() == 1)
        for type in RaveGetLoadedInterfaces()[InterfaceType.spacesampler]:
            self._runsampler(type)
        
    def test_robot(self):
        self.env.Load('data/lab1.env.xml')
        robot = self.env.GetRobots()[0]
        sp=RaveCreateSpaceSampler(self.env,'RobotConfiguration %s'%robot.GetName())
        assert(sp.Supports(SampleDataType.Real))
        values = sp.SampleSequence(SampleDataType.Real,1)
        assert(len(values) == robot.GetActiveDOF())
        robot.SetActiveDOFs(range(robot.GetDOF()-4),Robot.DOFAffine.X|Robot.DOFAffine.Y|Robot.DOFAffine.RotationAxis,[0,0,1])
        values = sp.SampleSequence(SampleDataType.Real,1)
        assert(len(values) == robot.GetActiveDOF())
